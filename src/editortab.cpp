#include "editortab.h"

#include <QFileInfo>
#include <QFontDatabase>
#include <QTextCursor>

EditorTab::EditorTab(QWidget *parent)
    : QPlainTextEdit(parent)
{
    setLineWrapMode(QPlainTextEdit::WidgetWidth);
    setTabStopDistance(fontMetrics().horizontalAdvance(QLatin1Char(' ')) * 4.0);
    setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
}

const QString &EditorTab::filePath() const
{
    return filePath_;
}

void EditorTab::setFilePath(const QString &path)
{
    filePath_ = path;
}

const QString &EditorTab::untitledName() const
{
    return untitledName_;
}

void EditorTab::setUntitledName(const QString &name)
{
    untitledName_ = name;
}

QString EditorTab::displayName() const
{
    return filePath_.isEmpty() ? untitledName_ : QFileInfo(filePath_).fileName();
}

void EditorTab::restoreContent(const QString &content, bool modified, int cursorPosition)
{
    setPlainText(content);
    document()->setModified(modified);

    QTextCursor restoredCursor = textCursor();
    const int safePosition = qBound(0, cursorPosition, document()->characterCount() - 1);
    restoredCursor.setPosition(safePosition);
    setTextCursor(restoredCursor);
}
