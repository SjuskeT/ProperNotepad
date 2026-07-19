#pragma once

#include <QMainWindow>
#include <QStringList>

class QAction;
class EditorTab;
class QCloseEvent;
class QShowEvent;
class QTabWidget;
class QTimer;

class MainWindow final : public QMainWindow
{
public:
    explicit MainWindow(QWidget *parent = nullptr);
    void openFiles(const QStringList &paths);

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void createMenus();
    EditorTab *addEditor(const QString &content,
                         const QString &filePath,
                         const QString &untitledName,
                         bool modified,
                         int cursorPosition);
    void createNewTab();
    void openFile();
    bool openFileAtPath(const QString &path);
    bool saveEditor(EditorTab *editor);
    bool saveEditorAs(EditorTab *editor);
    bool writeEditorToPath(EditorTab *editor, const QString &path);
    void closeTab(int index);

    [[nodiscard]] EditorTab *currentEditor() const;
    [[nodiscard]] QString nextUntitledName();
    [[nodiscard]] QString sessionFilePath() const;

    void updateTabTitle(EditorTab *editor);
    void updateWindowTitle();
    void scheduleSessionSave();
    void saveSession();
    void restoreSession();
    void reportSessionError(const QString &message);

    QTabWidget *tabs_;
    QTimer *sessionSaveTimer_;
    bool restoringSession_ = false;
};
