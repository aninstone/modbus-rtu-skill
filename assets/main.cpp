#include "mainwindow.h"

#include <QApplication>
#include <QStyleFactory>
#include <QPalette>
#include <QFont>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 应用信息
    QApplication::setApplicationName("ModbusRTUMaster");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("ModbusMaster");

    // Fusion 风格
    a.setStyle(QStyleFactory::create("Fusion"));

    // 深色主题
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(53, 53, 53));
    palette.setColor(QPalette::WindowText, Qt::white);
    palette.setColor(QPalette::Base, QColor(25, 25, 25));
    palette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    palette.setColor(QPalette::ToolTipBase, Qt::white);
    palette.setColor(QPalette::ToolTipText, Qt::white);
    palette.setColor(QPalette::Text, Qt::white);
    palette.setColor(QPalette::Button, QColor(53, 53, 53));
    palette.setColor(QPalette::ButtonText, Qt::white);
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Link, QColor(42, 130, 218));
    palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    palette.setColor(QPalette::HighlightedText, Qt::black);
    a.setPalette(palette);

    // 字体
    QFont font;
    font.setFamily("Microsoft YaHei");
    font.setPointSize(10);
    a.setFont(font);

    MainWindow w;
    w.setWindowTitle("Modbus RTU Master - 串口通信调试工具");
    w.show();

    return a.exec();
}
