#include "editortab.h"

#include <QColor>
#include <QFileInfo>
#include <QFontDatabase>
#include <QPaintEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QTextBlock>
#include <QTextCursor>
#include <QWidget>

class LineNumberArea final : public QWidget
{
public:
    explicit LineNumberArea(EditorTab *editor)
        : QWidget(editor), editor_(editor)
    {
    }

    [[nodiscard]] QSize sizeHint() const override
    {
        return {editor_->lineNumberAreaWidth(), 0};
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        editor_->paintLineNumberArea(event);
    }

private:
    EditorTab *editor_;
};

EditorTab::EditorTab(QWidget *parent)
    : QPlainTextEdit(parent),
      lineNumberArea_(new LineNumberArea(this))
{
    setLineWrapMode(QPlainTextEdit::WidgetWidth);
    setTabStopDistance(fontMetrics().horizontalAdvance(QLatin1Char(' ')) * 4.0);
    setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    connect(this, &QPlainTextEdit::blockCountChanged,
            this, &EditorTab::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest,
            this, &EditorTab::updateLineNumberArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged,
            lineNumberArea_, qOverload<>(&QWidget::update));

    updateLineNumberAreaWidth(0);
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

void EditorTab::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);

    const QRect area = contentsRect();
    lineNumberArea_->setGeometry(area.left(),
                                 area.top(),
                                 lineNumberAreaWidth(),
                                 area.height());
}

int EditorTab::lineNumberAreaWidth() const
{
    const int digits = QString::number(qMax(1, blockCount())).size();
    return 12 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
}

void EditorTab::updateLineNumberAreaWidth(int)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void EditorTab::updateLineNumberArea(const QRect &rect, int verticalScrollDistance)
{
    if (verticalScrollDistance != 0) {
        lineNumberArea_->scroll(0, verticalScrollDistance);
    } else {
        lineNumberArea_->update(0,
                                rect.y(),
                                lineNumberArea_->width(),
                                rect.height());
    }

    if (rect.contains(viewport()->rect())) {
        updateLineNumberAreaWidth(0);
    }
}

void EditorTab::paintLineNumberArea(QPaintEvent *event)
{
    QPainter painter(lineNumberArea_);
    painter.fillRect(event->rect(), QColor(30, 32, 36));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());
    const int currentBlockNumber = textCursor().blockNumber();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            if (blockNumber == currentBlockNumber) {
                painter.fillRect(0,
                                 top,
                                 lineNumberArea_->width(),
                                 fontMetrics().height(),
                                 QColor(42, 45, 50));
            }

            painter.setPen(blockNumber == currentBlockNumber
                               ? QColor(220, 220, 220)
                               : QColor(130, 135, 145));
            painter.drawText(0,
                             top,
                             lineNumberArea_->width() - 6,
                             fontMetrics().height(),
                             Qt::AlignRight,
                             QString::number(blockNumber + 1));
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}
