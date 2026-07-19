#include "mainwindow.h"

#include "editortab.h"

#include <QAction>
#include <QCloseEvent>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSet>
#include <QSaveFile>
#include <QSettings>
#include <QShowEvent>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTabBar>
#include <QTabWidget>
#include <QTextDocument>
#include <QTimer>

namespace
{
constexpr int SessionFormatVersion = 1;
constexpr int SessionSaveDelayMs = 350;

QString normalizedPath(const QString &path)
{
    const QFileInfo info(path);
    const QString canonical = info.canonicalFilePath();
    return canonical.isEmpty() ? info.absoluteFilePath() : canonical;
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      tabs_(new QTabWidget(this)),
      sessionSaveTimer_(new QTimer(this))
{
    setWindowTitle(QStringLiteral("ProperNotepad"));
    resize(1000, 700);

    tabs_->setTabsClosable(true);
    tabs_->setMovable(true);
    tabs_->setDocumentMode(true);
    setCentralWidget(tabs_);

    sessionSaveTimer_->setSingleShot(true);
    sessionSaveTimer_->setInterval(SessionSaveDelayMs);

    connect(sessionSaveTimer_, &QTimer::timeout, this, [this] { saveSession(); });
    connect(tabs_, &QTabWidget::tabCloseRequested, this, [this](int index) { closeTab(index); });
    connect(tabs_, &QTabWidget::currentChanged, this, [this] {
        updateWindowTitle();
        scheduleSessionSave();
    });
    connect(tabs_->tabBar(), &QTabBar::tabMoved, this, [this] { scheduleSessionSave(); });

    createMenus();

    QSettings settings;
    restoreGeometry(settings.value(QStringLiteral("window/geometry")).toByteArray());
    restoreSession();
}

void MainWindow::openFiles(const QStringList &paths)
{
    for (const QString &path : paths) {
        openFileAtPath(path);
    }
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);

    QTimer::singleShot(0, this, [this] {
        activateWindow();
        if (EditorTab *editor = currentEditor()) {
            editor->setFocus(Qt::OtherFocusReason);
        }
    });
}

void MainWindow::createMenus()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    QAction *newAction = fileMenu->addAction(tr("&New"));
    newAction->setShortcuts({QKeySequence::New, QKeySequence(Qt::CTRL | Qt::Key_T)});
    connect(newAction, &QAction::triggered, this, [this] { createNewTab(); });

    QAction *openAction = fileMenu->addAction(tr("&Open..."));
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, [this] { openFile(); });

    fileMenu->addSeparator();

    QAction *saveAction = fileMenu->addAction(tr("&Save"));
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, [this] {
        if (EditorTab *editor = currentEditor()) {
            saveEditor(editor);
        }
    });

    QAction *saveAsAction = fileMenu->addAction(tr("Save &As..."));
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAction, &QAction::triggered, this, [this] {
        if (EditorTab *editor = currentEditor()) {
            saveEditorAs(editor);
        }
    });

    fileMenu->addSeparator();

    QAction *closeAction = fileMenu->addAction(tr("&Close Tab"));
    closeAction->setShortcut(QKeySequence::Close);
    connect(closeAction, &QAction::triggered, this, [this] { closeTab(tabs_->currentIndex()); });

    QAction *quitAction = fileMenu->addAction(tr("&Quit"));
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &QWidget::close);

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));

    QAction *undoAction = editMenu->addAction(tr("&Undo"));
    undoAction->setShortcut(QKeySequence::Undo);
    connect(undoAction, &QAction::triggered, this, [this] {
        if (EditorTab *editor = currentEditor()) editor->undo();
    });

    QAction *redoAction = editMenu->addAction(tr("&Redo"));
    redoAction->setShortcut(QKeySequence::Redo);
    connect(redoAction, &QAction::triggered, this, [this] {
        if (EditorTab *editor = currentEditor()) editor->redo();
    });

    editMenu->addSeparator();

    QAction *cutAction = editMenu->addAction(tr("Cu&t"));
    cutAction->setShortcut(QKeySequence::Cut);
    connect(cutAction, &QAction::triggered, this, [this] {
        if (EditorTab *editor = currentEditor()) editor->cut();
    });

    QAction *copyAction = editMenu->addAction(tr("&Copy"));
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction, &QAction::triggered, this, [this] {
        if (EditorTab *editor = currentEditor()) editor->copy();
    });

    QAction *pasteAction = editMenu->addAction(tr("&Paste"));
    pasteAction->setShortcut(QKeySequence::Paste);
    connect(pasteAction, &QAction::triggered, this, [this] {
        if (EditorTab *editor = currentEditor()) editor->paste();
    });

    QAction *selectAllAction = editMenu->addAction(tr("Select &All"));
    selectAllAction->setShortcut(QKeySequence::SelectAll);
    connect(selectAllAction, &QAction::triggered, this, [this] {
        if (EditorTab *editor = currentEditor()) editor->selectAll();
    });
}

EditorTab *MainWindow::addEditor(const QString &content,
                                 const QString &filePath,
                                 const QString &untitledName,
                                 bool modified,
                                 int cursorPosition)
{
    auto *editor = new EditorTab;
    editor->setFilePath(filePath);
    editor->setUntitledName(untitledName);
    editor->restoreContent(content, modified, cursorPosition);

    const int index = tabs_->addTab(editor, editor->displayName());
    tabs_->setCurrentIndex(index);
    updateTabTitle(editor);

    connect(editor, &QPlainTextEdit::textChanged, this, [this, editor] {
        updateTabTitle(editor);
        scheduleSessionSave();
    });
    connect(editor, &QPlainTextEdit::cursorPositionChanged, this, [this] {
        scheduleSessionSave();
    });

    return editor;
}

void MainWindow::createNewTab()
{
    EditorTab *editor = addEditor(QString(), QString(), nextUntitledName(), false, 0);
    editor->setFocus();
    scheduleSessionSave();
}

void MainWindow::openFile()
{
    const QString path = QFileDialog::getOpenFileName(
        this,
        tr("Open File"),
        QString(),
        tr("Text files (*.txt *.md *.log *.csv *.json *.xml *.cpp *.c *.h *.hpp);;All files (*)"));

    if (!path.isEmpty()) {
        openFileAtPath(path);
    }
}

bool MainWindow::openFileAtPath(const QString &path)
{
    const QString absolutePath = normalizedPath(path);

    for (int index = 0; index < tabs_->count(); ++index) {
        auto *editor = qobject_cast<EditorTab *>(tabs_->widget(index));
        if (editor && !editor->filePath().isEmpty()
            && normalizedPath(editor->filePath()) == absolutePath) {
            tabs_->setCurrentIndex(index);
            return true;
        }
    }

    QFile file(absolutePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Could Not Open File"), file.errorString());
        return false;
    }

    const QString content = QString::fromUtf8(file.readAll());
    EditorTab *editor = addEditor(content, absolutePath, QString(), false, 0);
    editor->setFocus();
    scheduleSessionSave();
    return true;
}

bool MainWindow::saveEditor(EditorTab *editor)
{
    if (!editor) {
        return false;
    }

    return editor->filePath().isEmpty()
        ? saveEditorAs(editor)
        : writeEditorToPath(editor, editor->filePath());
}

bool MainWindow::saveEditorAs(EditorTab *editor)
{
    if (!editor) {
        return false;
    }

    const QString suggestedPath = editor->filePath().isEmpty()
        ? editor->displayName() + QStringLiteral(".txt")
        : editor->filePath();
    const QString path = QFileDialog::getSaveFileName(
        this,
        tr("Save File"),
        suggestedPath,
        tr("Text files (*.txt);;All files (*)"));

    return !path.isEmpty() && writeEditorToPath(editor, path);
}

bool MainWindow::writeEditorToPath(EditorTab *editor, const QString &path)
{
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Could Not Save File"), file.errorString());
        return false;
    }

    const QByteArray contents = editor->toPlainText().toUtf8();
    if (file.write(contents) != contents.size() || !file.commit()) {
        QMessageBox::critical(this, tr("Could Not Save File"), file.errorString());
        return false;
    }

    editor->setFilePath(normalizedPath(path));
    editor->document()->setModified(false);
    updateTabTitle(editor);
    scheduleSessionSave();
    statusBar()->showMessage(tr("Saved %1").arg(editor->displayName()), 2500);
    return true;
}

void MainWindow::closeTab(int index)
{
    if (index < 0 || index >= tabs_->count()) {
        return;
    }

    auto *editor = qobject_cast<EditorTab *>(tabs_->widget(index));
    if (!editor) {
        return;
    }

    if (editor->document()->isModified()) {
        const QMessageBox::StandardButton result = QMessageBox::warning(
            this,
            tr("Unsaved Changes"),
            tr("Save changes to %1 before closing this tab?").arg(editor->displayName()),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Save);

        if (result == QMessageBox::Cancel
            || (result == QMessageBox::Save && !saveEditor(editor))) {
            return;
        }
    }

    tabs_->removeTab(index);
    editor->deleteLater();

    if (tabs_->count() == 0) {
        createNewTab();
    } else {
        scheduleSessionSave();
    }
}

EditorTab *MainWindow::currentEditor() const
{
    return qobject_cast<EditorTab *>(tabs_->currentWidget());
}

QString MainWindow::nextUntitledName()
{
    QSet<QString> usedNames;
    for (int index = 0; index < tabs_->count(); ++index) {
        const auto *editor = qobject_cast<EditorTab *>(tabs_->widget(index));
        if (editor && editor->filePath().isEmpty()) {
            usedNames.insert(editor->untitledName());
        }
    }

    for (int number = 1; ; ++number) {
        const QString candidate = tr("Untitled %1").arg(number);
        if (!usedNames.contains(candidate)) {
            return candidate;
        }
    }
}

QString MainWindow::sessionFilePath() const
{
    const QString directory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return QDir(directory).filePath(QStringLiteral("session.json"));
}

void MainWindow::updateTabTitle(EditorTab *editor)
{
    const int index = tabs_->indexOf(editor);
    if (index < 0) {
        return;
    }

    const QString marker = editor->document()->isModified() ? QStringLiteral("*") : QString();
    tabs_->setTabText(index, editor->displayName() + marker);
    tabs_->setTabToolTip(index, editor->filePath());
    updateWindowTitle();
}

void MainWindow::updateWindowTitle()
{
    const EditorTab *editor = currentEditor();
    const QString documentName = editor ? editor->displayName() : QString();
    setWindowTitle(documentName.isEmpty()
        ? QStringLiteral("ProperNotepad")
        : tr("%1 - ProperNotepad").arg(documentName));
}

void MainWindow::scheduleSessionSave()
{
    if (!restoringSession_) {
        sessionSaveTimer_->start();
    }
}

void MainWindow::saveSession()
{
    if (restoringSession_) {
        return;
    }

    const QFileInfo sessionInfo(sessionFilePath());
    if (!QDir().mkpath(sessionInfo.absolutePath())) {
        reportSessionError(tr("Could not create the recovery-data directory."));
        return;
    }

    QJsonArray tabArray;
    for (int index = 0; index < tabs_->count(); ++index) {
        const auto *editor = qobject_cast<EditorTab *>(tabs_->widget(index));
        if (!editor) {
            continue;
        }

        QJsonObject tab;
        tab.insert(QStringLiteral("filePath"), editor->filePath());
        tab.insert(QStringLiteral("untitledName"), editor->untitledName());
        tab.insert(QStringLiteral("content"), editor->toPlainText());
        tab.insert(QStringLiteral("modified"), editor->document()->isModified());
        tab.insert(QStringLiteral("cursorPosition"), editor->textCursor().position());
        tabArray.append(tab);
    }

    QJsonObject root;
    root.insert(QStringLiteral("version"), SessionFormatVersion);
    root.insert(QStringLiteral("currentIndex"), tabs_->currentIndex());
    root.insert(QStringLiteral("tabs"), tabArray);

    QSaveFile file(sessionFilePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        reportSessionError(tr("Could not write the recovery session: %1").arg(file.errorString()));
        return;
    }

    const QByteArray contents = QJsonDocument(root).toJson(QJsonDocument::Compact);
    if (file.write(contents) != contents.size() || !file.commit()) {
        reportSessionError(tr("Could not commit the recovery session: %1").arg(file.errorString()));
    }
}

void MainWindow::restoreSession()
{
    restoringSession_ = true;

    QFile file(sessionFilePath());
    if (!file.exists()) {
        restoringSession_ = false;
        createNewTab();
        return;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        restoringSession_ = false;
        createNewTab();
        reportSessionError(tr("Could not read the previous recovery session."));
        return;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    const QJsonObject root = document.object();
    const QJsonArray tabArray = root.value(QStringLiteral("tabs")).toArray();

    if (parseError.error != QJsonParseError::NoError
        || !document.isObject()
        || root.value(QStringLiteral("version")).toInt() != SessionFormatVersion) {
        restoringSession_ = false;
        createNewTab();
        reportSessionError(tr("The previous recovery session was invalid, so a new tab was opened."));
        return;
    }

    for (const QJsonValue &value : tabArray) {
        const QJsonObject tab = value.toObject();
        const QString filePath = tab.value(QStringLiteral("filePath")).toString();
        QString untitledName = tab.value(QStringLiteral("untitledName")).toString();
        if (filePath.isEmpty() && untitledName.isEmpty()) {
            untitledName = nextUntitledName();
        }

        addEditor(tab.value(QStringLiteral("content")).toString(),
                  filePath,
                  untitledName,
                  tab.value(QStringLiteral("modified")).toBool(),
                  tab.value(QStringLiteral("cursorPosition")).toInt());
    }

    if (tabs_->count() == 0) {
        createNewTab();
    } else {
        tabs_->setCurrentIndex(qBound(0,
                                      root.value(QStringLiteral("currentIndex")).toInt(),
                                      tabs_->count() - 1));
    }

    restoringSession_ = false;
    updateWindowTitle();
}

void MainWindow::reportSessionError(const QString &message)
{
    statusBar()->showMessage(message, 8000);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    sessionSaveTimer_->stop();
    saveSession();

    QSettings settings;
    settings.setValue(QStringLiteral("window/geometry"), saveGeometry());

    event->accept();
}
