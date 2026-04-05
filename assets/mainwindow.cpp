#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "modbusrtumaster.h"

#include <QMessageBox>
#include <QDateTime>
#include <QFileDialog>
#include <QSerialPortInfo>
#include <QVector>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_modbusMaster(new ModbusRTUMaster(this))
    , m_bytesSent(0)
    , m_bytesReceived(0)
    , m_frameCount(0)
{
    ui->setupUi(this);

    // 设置窗口属性
    resize(1100, 750);
    setMinimumSize(900, 600);

    // 初始化UI
    setupUi();

    // 连接信号槽
    setupConnections();

    // 刷新串口列表
    on_refreshPorts();
}

MainWindow::~MainWindow()
{
}

/**
 * @brief 初始化UI
 */
void MainWindow::setupUi()
{
    // 波特率
    ui->comboBoxBaud->addItems({
        "9600", "19200", "38400", "57600", "115200", "230400", "460800", "921600"
    });
    ui->comboBoxBaud->setCurrentIndex(4); // 默认115200

    // 数据位
    ui->comboBoxDataBits->addItems({"5", "6", "7", "8"});
    ui->comboBoxDataBits->setCurrentIndex(3); // 默认8位

    // 校验位
    ui->comboBoxParity->addItems({"None", "Odd", "Even", "Mark", "Space"});
    ui->comboBoxParity->setCurrentIndex(0); // 默认无校验

    // 停止位
    ui->comboBoxStopBits->addItems({"1", "1.5", "2"});
    ui->comboBoxStopBits->setCurrentIndex(0); // 默认1位

    // 功能码
    ui->comboBoxFunctionCode->addItems({
        "0x01 - 读线圈 (Read Coils)",
        "0x02 - 读离散输入 (Read Discrete Inputs)",
        "0x03 - 读保持寄存器 (Read Holding Registers)",
        "0x04 - 读输入寄存器 (Read Input Registers)",
        "0x05 - 写单个线圈 (Write Single Coil)",
        "0x06 - 写单个寄存器 (Write Single Register)",
        "0x0F - 写多个线圈 (Write Multiple Coils)",
        "0x10 - 写多个寄存器 (Write Multiple Registers)"
    });
    ui->comboBoxFunctionCode->setCurrentIndex(2); // 默认读保持寄存器

    // 样式
    ui->textEditLog->setStyleSheet(
        "QTextEdit { "
        "font-family: 'Consolas', 'Courier New', monospace; "
        "font-size: 13px; "
        "background-color: #1e1e1e; "
        "color: #00ff00; "
        "border: 2px solid #2c3e50; "
        "border-radius: 5px; "
        "padding: 5px; "
        "}"
    );
}

/**
 * @brief 连接信号槽
 */
void MainWindow::setupConnections()
{
    // 串口操作
    connect(ui->btnRefresh, &QPushButton::clicked, this, &MainWindow::on_refreshPorts);
    connect(ui->btnOpenClose, &QPushButton::clicked, this, &MainWindow::on_openClosePort);

    // Modbus操作
    connect(ui->btnRead, &QPushButton::clicked, this, &MainWindow::on_readRegisters);
    connect(ui->btnWrite, &QPushButton::clicked, this, &MainWindow::on_writeSingleRegister);

    // 显示控制
    connect(ui->btnClearLog, &QPushButton::clicked, this, &MainWindow::on_clearLog);
    connect(ui->btnSaveLog, &QPushButton::clicked, this, &MainWindow::on_saveLog);

    // Modbus Master信号
    connect(m_modbusMaster, &ModbusRTUMaster::dataReceived, this, &MainWindow::on_dataReceived);
    connect(m_modbusMaster, &ModbusRTUMaster::errorOccurred, this, &MainWindow::on_errorOccurred);
    connect(m_modbusMaster, &ModbusRTUMaster::connectionStateChanged, this, &MainWindow::on_connectionStateChanged);
}

/**
 * @brief 刷新串口列表
 */
void MainWindow::on_refreshPorts()
{
    ui->comboBoxPort->clear();

    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

    if (ports.isEmpty()) {
        ui->comboBoxPort->addItem("无可用串口");
        ui->comboBoxPort->setEnabled(false);
    } else {
        ui->comboBoxPort->setEnabled(true);
        for (const QSerialPortInfo &info : ports) {
            QString portDesc = info.portName();
            if (!info.description().isEmpty()) {
                portDesc += " - " + info.description();
            }
            ui->comboBoxPort->addItem(portDesc, info.portName());
        }

        // 优先选择COM7
        int com7Index = ui->comboBoxPort->findData("COM7");
        if (com7Index >= 0) {
            ui->comboBoxPort->setCurrentIndex(com7Index);
        }
    }
}

/**
 * @brief 打开/关闭串口
 */
void MainWindow::on_openClosePort()
{
    if (m_modbusMaster->isOpen()) {
        // 关闭串口
        m_modbusMaster->closePort();
    } else {
        // 获取串口名称
        QString portName = ui->comboBoxPort->currentData().toString();
        if (portName.isEmpty() || portName == "无可用串口") {
            QMessageBox::warning(this, "警告", "没有可用的串口!");
            return;
        }

        // 获取波特率
        bool ok;
        qint32 baudRate = ui->comboBoxBaud->currentText().toInt(&ok);
        if (!ok) baudRate = 115200;

        // 获取数据位
        QSerialPort::DataBits dataBits;
        switch (ui->comboBoxDataBits->currentIndex()) {
            case 0: dataBits = QSerialPort::Data5; break;
            case 1: dataBits = QSerialPort::Data6; break;
            case 2: dataBits = QSerialPort::Data7; break;
            default: dataBits = QSerialPort::Data8; break;
        }

        // 获取校验位
        QSerialPort::Parity parity;
        switch (ui->comboBoxParity->currentIndex()) {
            case 1: parity = QSerialPort::OddParity; break;
            case 2: parity = QSerialPort::EvenParity; break;
            case 3: parity = QSerialPort::MarkParity; break;
            case 4: parity = QSerialPort::SpaceParity; break;
            default: parity = QSerialPort::NoParity; break;
        }

        // 获取停止位
        QSerialPort::StopBits stopBits;
        switch (ui->comboBoxStopBits->currentIndex()) {
            case 1: stopBits = QSerialPort::OneAndHalfStop; break;
            case 2: stopBits = QSerialPort::TwoStop; break;
            default: stopBits = QSerialPort::OneStop; break;
        }

        // 打开串口
        if (!m_modbusMaster->openPort(portName, baudRate, dataBits, parity, stopBits)) {
            QMessageBox::critical(this, "错误", "无法打开串口!");
        }
    }
}

/**
 * @brief 读取寄存器
 */
void MainWindow::on_readRegisters()
{
    if (!m_modbusMaster->isOpen()) {
        QMessageBox::warning(this, "警告", "请先打开串口!");
        return;
    }

    quint8 slaveAddr = static_cast<quint8>(ui->spinBoxSlave->value());
    quint16 startAddr = static_cast<quint16>(ui->spinBoxAddress->value());
    quint16 quantity = static_cast<quint16>(ui->spinBoxQuantity->value());
    quint8 funcCode = static_cast<quint8>(ui->comboBoxFunctionCode->currentIndex() + 1);

    // 线圈/离散输入最大2000个
    if ((funcCode == 0x01 || funcCode == 0x02) && quantity > 2000) {
        quantity = 2000;
        ui->spinBoxQuantity->setValue(quantity);
    }

    QByteArray frame = m_modbusMaster->buildReadFrame(slaveAddr, funcCode, startAddr, quantity);

    if (m_modbusMaster->sendFrame(frame)) {
        m_bytesSent += frame.size();
        updateStatistics(m_bytesSent, m_bytesReceived);

        QString funcName = ModbusRTUMaster::getFunctionCodeName(funcCode);
        QString log = QString("[%1] TX - %2: %3")
                          .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"))
                          .arg(funcName)
                          .arg(ModbusRTUMaster::bytesToHexString(frame));
        appendLog(log, "#3498db");
    }
}

/**
 * @brief 写入数据
 */
void MainWindow::on_writeSingleRegister()
{
    if (!m_modbusMaster->isOpen()) {
        QMessageBox::warning(this, "警告", "请先打开串口!");
        return;
    }

    quint8 slaveAddr = static_cast<quint8>(ui->spinBoxSlave->value());
    quint8 funcCode = static_cast<quint8>(ui->comboBoxFunctionCode->currentIndex() + 1);
    quint16 address = static_cast<quint16>(ui->spinBoxAddress->value());
    quint16 value = static_cast<quint16>(ui->spinBoxQuantity->value());

    QByteArray frame;

    switch (funcCode) {
        case 0x05: { // 写单个线圈
            frame = m_modbusMaster->buildWriteSingleCoilFrame(slaveAddr, address, value != 0);
            break;
        }
        case 0x06: { // 写单个寄存器
            frame = m_modbusMaster->buildWriteSingleRegisterFrame(slaveAddr, address, value);
            break;
        }
        case 0x0F: { // 写多个线圈
            quint16 quantity = static_cast<quint16>(ui->spinBoxQuantity->value());
            QByteArray coilData;
            for (quint16 i = 0; i < quantity; i++) {
                coilData.append(static_cast<char>(0xFF));
            }
            frame = m_modbusMaster->buildWriteMultipleCoilsFrame(slaveAddr, address, coilData);
            break;
        }
        case 0x10: { // 写多个寄存器
            quint16 quantity = static_cast<quint16>(ui->spinBoxQuantity->value());
            QVector<quint16> registers;
            for (quint16 i = 0; i < quantity; i++) {
                registers.append(value);
            }
            frame = m_modbusMaster->buildWriteMultipleRegistersFrame(slaveAddr, address, registers);
            break;
        }
        default:
            QMessageBox::information(this, "提示", "当前功能码不支持写入操作\n请选择: 0x05写单个线圈, 0x06写单个寄存器, 0x0F写多个线圈, 0x10写多个寄存器");
            return;
    }

    if (m_modbusMaster->sendFrame(frame)) {
        m_bytesSent += frame.size();
        updateStatistics(m_bytesSent, m_bytesReceived);

        QString funcName = ModbusRTUMaster::getFunctionCodeName(funcCode);
        QString log = QString("[%1] TX - %2: %3")
                          .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"))
                          .arg(funcName)
                          .arg(ModbusRTUMaster::bytesToHexString(frame));
        appendLog(log, "#3498db");
    }
}

/**
 * @brief 数据接收处理
 */
void MainWindow::on_dataReceived(const QByteArray &data)
{
    m_bytesReceived += data.size();
    m_frameCount++;
    updateStatistics(m_bytesSent, m_bytesReceived);

    // 解析帧
    ModbusFrame frame = ModbusRTUMaster::parseFrame(data);

    QString timeStr = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");

    if (!frame.isValid) {
        QString log = QString("[%1] RX - 帧格式错误: %2")
                          .arg(timeStr)
                          .arg(ModbusRTUMaster::bytesToHexString(data));
        appendLog(log, "#e74c3c");
        return;
    }

    QString hexData = ModbusRTUMaster::bytesToHexString(data);

    if (frame.isException) {
        QString funcName = ModbusRTUMaster::getFunctionCodeName(frame.functionCode);
        QString exceptName = ModbusRTUMaster::getExceptionName(frame.exceptionCode);
        QString log = QString("[%1] RX - %2 异常: %3 [%4]")
                          .arg(timeStr)
                          .arg(funcName)
                          .arg(exceptName)
                          .arg(hexData);
        appendLog(log, "#e74c3c");
    } else {
        QString funcName = ModbusRTUMaster::getFunctionCodeName(frame.functionCode);
        QString log = QString("[%1] RX - %2: %3")
                          .arg(timeStr)
                          .arg(funcName)
                          .arg(hexData);
        appendLog(log, "#27ae60");

        // 解析数据内容
        if (!frame.data.isEmpty()) {
            parseResponseData(frame);
        }
    }
}

/**
 * @brief 解析响应数据
 */
void MainWindow::parseResponseData(const ModbusFrame &frame)
{
    if (frame.data.isEmpty()) return;

    QString dataInfo;

    switch (frame.functionCode) {
        case 0x01: // 读线圈
        case 0x02: // 读离散输入
        case 0x03: // 读保持寄存器
        case 0x04: // 读输入寄存器
            if (frame.data.size() >= 1) {
                quint8 byteCount = static_cast<quint8>(frame.data[0]);
                QStringList values;
                for (int i = 0; i < byteCount && (i + 1) < frame.data.size(); i++) {
                    values.append(QString::number(static_cast<quint8>(frame.data[i + 1]), 16).toUpper().prepend("0x"));
                }
                dataInfo = QString("数据(%1字节): %2").arg(byteCount).arg(values.join(", "));

                // 对于寄存器，也显示为数值
                if (frame.functionCode == 0x03 || frame.functionCode == 0x04) {
                    QStringList registerValues;
                    for (int i = 1; i + 1 < frame.data.size(); i += 2) {
                        quint16 regValue = ModbusRTUMaster::bytesToUint16(
                            static_cast<quint8>(frame.data[i]),
                            static_cast<quint8>(frame.data[i + 1])
                        );
                        registerValues.append(QString::number(regValue));
                    }
                    dataInfo += QString("\n寄存器值: [%1]").arg(registerValues.join(", "));
                }
            }
            break;

        case 0x05: // 写单个线圈
        case 0x06: // 写单个寄存器
            dataInfo = "写入成功!";
            break;

        case 0x0F: // 写多个线圈
        case 0x10: // 写多个寄存器
            dataInfo = "批量写入成功!";
            break;
    }

    if (!dataInfo.isEmpty()) {
        appendLog("  └─ " + dataInfo, "#f39c12");
    }
}

/**
 * @brief 错误处理
 */
void MainWindow::on_errorOccurred(const QString &error)
{
    appendLog(QString("[错误] %1").arg(error), "#e74c3c");
}

/**
 * @brief 连接状态变化
 */
void MainWindow::on_connectionStateChanged(bool connected)
{
    updateUIState(connected);

    if (connected) {
        appendLog("串口已打开: " + getCurrentSettings(), "#27ae60");
    } else {
        appendLog("串口已关闭", "#e74c3c");
    }
}

/**
 * @brief 清空日志
 */
void MainWindow::on_clearLog()
{
    ui->textEditLog->clear();
    m_bytesSent = 0;
    m_bytesReceived = 0;
    m_frameCount = 0;
    updateStatistics(0, 0);
}

/**
 * @brief 保存日志
 */
void MainWindow::on_saveLog()
{
    QString fileName = QFileDialog::getSaveFileName(this, "保存日志",
        QString("modbus_log_%1.txt").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        "文本文件 (*.txt);;所有文件 (*.*)");

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(ui->textEditLog->toPlainText().toUtf8());
            file.close();
            QMessageBox::information(this, "成功", "日志已保存!");
        } else {
            QMessageBox::critical(this, "错误", "无法保存文件!");
        }
    }
}

/**
 * @brief 更新UI状态
 */
void MainWindow::updateUIState(bool connected)
{
    if (connected) {
        ui->btnOpenClose->setText("关闭串口");
        ui->btnOpenClose->setStyleSheet("background-color: #e74c3c; color: white;");

        // 禁用串口设置
        ui->comboBoxPort->setEnabled(false);
        ui->comboBoxBaud->setEnabled(false);
        ui->comboBoxDataBits->setEnabled(false);
        ui->comboBoxParity->setEnabled(false);
        ui->comboBoxStopBits->setEnabled(false);
        ui->btnRefresh->setEnabled(false);

        // 启用操作按钮
        ui->btnRead->setEnabled(true);
        ui->btnWrite->setEnabled(true);

        ui->labelStatus->setText("● 已连接");
        ui->labelStatus->setStyleSheet("color: #27ae60; font-weight: bold; font-size: 12px;");
    } else {
        ui->btnOpenClose->setText("打开串口");
        ui->btnOpenClose->setStyleSheet("");

        // 启用串口设置
        ui->comboBoxPort->setEnabled(true);
        ui->comboBoxBaud->setEnabled(true);
        ui->comboBoxDataBits->setEnabled(true);
        ui->comboBoxParity->setEnabled(true);
        ui->comboBoxStopBits->setEnabled(true);
        ui->btnRefresh->setEnabled(true);

        // 禁用操作按钮
        ui->btnRead->setEnabled(false);
        ui->btnWrite->setEnabled(false);

        ui->labelStatus->setText("● 未连接");
        ui->labelStatus->setStyleSheet("color: #e74c3c; font-weight: bold; font-size: 12px;");
    }
}

/**
 * @brief 添加日志
 */
void MainWindow::appendLog(const QString &text, const QString &color)
{
    QMutexLocker locker(&m_displayMutex);

    QTextCursor cursor(ui->textEditLog->textCursor());
    cursor.movePosition(QTextCursor::End);

    QTextCharFormat format;
    format.setForeground(QColor(color));
    cursor.setCharFormat(format);
    cursor.insertText(text + "\n");

    ui->textEditLog->setTextCursor(cursor);
    ui->textEditLog->ensureCursorVisible();
}

/**
 * @brief 更新统计
 */
void MainWindow::updateStatistics(quint64 txBytes, quint64 rxBytes)
{
    ui->labelTxBytes->setText(QString::number(txBytes));
    ui->labelRxBytes->setText(QString::number(rxBytes));
    ui->labelFrameCount->setText(QString::number(m_frameCount));
}

/**
 * @brief 获取当前设置
 */
QString MainWindow::getCurrentSettings() const
{
    return QString("%1, %2, %3-%4-%5")
        .arg(ui->comboBoxPort->currentText())
        .arg(ui->comboBoxBaud->currentText())
        .arg(ui->comboBoxDataBits->currentText())
        .arg(ui->comboBoxParity->currentText().left(1))
        .arg(ui->comboBoxStopBits->currentText());
}
