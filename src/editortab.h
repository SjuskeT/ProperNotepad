#pragma once

#include <QPlainTextEdit>
#include <QString>

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

private:
    QString filePath_;
    QString untitledName_;
};
