#include "mainwindow.h"

#include <QApplication>
#include <QColor>
#include <QIcon>
#include <QPalette>
#include <QStyleFactory>

namespace
{
void applyDarkTheme(QApplication &application)
{
    application.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));

    QPalette palette;
    palette.setColor(QPalette::Window, QColor(30, 32, 36));
    palette.setColor(QPalette::WindowText, QColor(230, 230, 230));
    palette.setColor(QPalette::Base, QColor(22, 24, 27));
    palette.setColor(QPalette::AlternateBase, QColor(35, 38, 42));
    palette.setColor(QPalette::ToolTipBase, QColor(230, 230, 230));
    palette.setColor(QPalette::ToolTipText, QColor(25, 25, 25));
    palette.setColor(QPalette::Text, QColor(230, 230, 230));
    palette.setColor(QPalette::Button, QColor(42, 45, 50));
    palette.setColor(QPalette::ButtonText, QColor(230, 230, 230));
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Link, QColor(86, 156, 214));
    palette.setColor(QPalette::Highlight, QColor(38, 79, 120));
    palette.setColor(QPalette::HighlightedText, Qt::white);
    palette.setColor(QPalette::Disabled, QPalette::Text, QColor(120, 120, 120));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(120, 120, 120));
    application.setPalette(palette);

    application.setStyleSheet(QStringLiteral(R"(
        QMainWindow, QMenuBar, QMenu, QStatusBar {
            background-color: #1e2024;
        }
        QPlainTextEdit {
            border: none;
            padding: 8px;
            selection-background-color: #264f78;
        }
        QTabWidget::pane {
            border: none;
        }
        QTabBar::tab {
            background: #2a2d32;
            color: #c7c7c7;
            padding: 8px 14px;
            border-right: 1px solid #1e2024;
        }
        QTabBar::tab:selected {
            background: #16181b;
            color: #ffffff;
            border-top: 2px solid #569cd6;
        }
        QTabBar::tab:hover:!selected {
            background: #34383e;
        }
        QMenu::item:selected {
            background-color: #264f78;
        }
    )"));
}
}

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    QApplication::setOrganizationName(QStringLiteral("SjuskeT"));
    QApplication::setApplicationName(QStringLiteral("ProperNotepad"));
    QApplication::setApplicationVersion(QStringLiteral("0.1.0"));
    QApplication::setDesktopFileName(QStringLiteral("propernotepad.desktop"));
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/propernotepad.png")));

    applyDarkTheme(application);

    MainWindow window;
    window.show();
    window.openFiles(application.arguments().mid(1));
    return application.exec();
}
