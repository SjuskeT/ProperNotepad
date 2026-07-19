#pragma once

#include <QPlainTextEdit>
#include <QString>

class LineNumberArea;
class QPaintEvent;
class QResizeEvent;

class EditorTab final : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit EditorTab(QWidget *parent = nullptr);

    [[nodiscard]] const QString &filePath() const;
    void setFilePath(const QString &path);

    [[nodiscard]] const QString &untitledName() const;
    void setUntitledName(const QString &name);

    [[nodiscard]] QString displayName() const;
    void restoreContent(const QString &content, bool modified, int cursorPosition);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    friend class LineNumberArea;

    [[nodiscard]] int lineNumberAreaWidth() const;
    void updateLineNumberAreaWidth(int blockCount);
    void updateLineNumberArea(const QRect &rect, int verticalScrollDistance);
    void paintLineNumberArea(QPaintEvent *event);

    LineNumberArea *lineNumberArea_;
    QString filePath_;
    QString untitledName_;
};
