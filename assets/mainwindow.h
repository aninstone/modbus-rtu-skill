#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QMutex>
#include "modbusrtumaster.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 串口操作
    void on_refreshPorts();
    void on_openClosePort();

    // Modbus操作
    void on_readRegisters();
    void on_writeSingleRegister();

    // 数据处理
    void on_dataReceived(const QByteArray &data);
    void on_errorOccurred(const QString &error);
    void on_connectionStateChanged(bool connected);

    // 显示控制
    void on_clearLog();
    void on_saveLog();

private:
    Ui::MainWindow *ui;
    ModbusRTUMaster *m_modbusMaster;

    // 统计
    quint64 m_bytesSent;
    quint64 m_bytesReceived;
    int m_frameCount;

    // 显示锁
    QMutex m_displayMutex;

    // 辅助方法
    void setupUi();
    void setupConnections();
    void parseResponseData(const ModbusFrame &frame);
    void updatePortList();
    void updateUIState(bool connected);
    void appendLog(const QString &text, const QString &color = "#00ff00");
    void updateStatistics(quint64 txBytes, quint64 rxBytes);
    QString getCurrentSettings() const;
};

#endif // MAINWINDOW_H
