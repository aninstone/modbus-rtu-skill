#ifndef MODBUSRTUMASTER_H
#define MODBUSRTUMASTER_H

#include <QObject>
#include <QSerialPort>
#include <QByteArray>
#include <QTimer>

/**
 * @brief Modbus RTU 功能码定义
 */
enum class ModbusFunctionCode : quint8 {
    ReadCoils              = 0x01,  // 读线圈
    ReadDiscreteInputs     = 0x02,  // 读离散输入
    ReadHoldingRegisters   = 0x03,  // 读保持寄存器
    ReadInputRegisters     = 0x04,  // 读输入寄存器
    WriteSingleCoil        = 0x05,  // 写单个线圈
    WriteSingleRegister    = 0x06,  // 写单个寄存器
    WriteMultipleCoils     = 0x0F,  // 写多个线圈
    WriteMultipleRegisters = 0x10   // 写多个寄存器
};

/**
 * @brief Modbus异常码
 */
enum class ModbusExceptionCode : quint8 {
    IllegalFunction        = 0x01,  // 非法功能码
    IllegalDataAddress     = 0x02,  // 非法数据地址
    IllegalDataValue       = 0x03,  // 非法数据值
    ServerDeviceFailure    = 0x04,  // 从机故障
    Acknowledge            = 0x05,  // 确认
    ServerDeviceBusy       = 0x06,  // 从机忙
    MemoryParityError      = 0x08,  // 内存奇偶校验错误
    GatewayPathUnavailable = 0x0A,  // 网关路径不可用
    GatewayDeviceFailed    = 0x0B   // 网关设备响应失败
};

/**
 * @brief Modbus RTU 帧结构
 */
struct ModbusFrame {
    quint8 slaveAddress;     // 从机地址 (1-247)
    quint8 functionCode;     // 功能码
    QByteArray data;         // 数据区
    quint16 crc;             // CRC16校验

    bool isValid;            // 帧是否有效
    bool isException;        // 是否为异常响应
    quint8 exceptionCode;     // 异常码
};

/**
 * @brief ModbusRTUMaster类 - Modbus RTU主站
 */
class ModbusRTUMaster : public QObject
{
    Q_OBJECT

public:
    explicit ModbusRTUMaster(QObject *parent = nullptr);
    ~ModbusRTUMaster();

    // 串口操作
    bool openPort(const QString &portName, qint32 baudRate,
                  QSerialPort::DataBits dataBits = QSerialPort::Data8,
                  QSerialPort::Parity parity = QSerialPort::NoParity,
                  QSerialPort::StopBits stopBits = QSerialPort::OneStop);
    void closePort();
    bool isOpen() const;

    // Modbus命令 - 读取
    QByteArray buildReadFrame(quint8 slaveAddr, ModbusFunctionCode funcCode,
                             quint16 startAddr, quint16 quantity);
    QByteArray buildReadFrame(quint8 slaveAddr, quint8 funcCode,
                             quint16 startAddr, quint16 quantity);

    // Modbus命令 - 写入单个
    QByteArray buildWriteSingleCoilFrame(quint8 slaveAddr, quint16 coilAddr, bool value);
    QByteArray buildWriteSingleRegisterFrame(quint8 slaveAddr, quint16 regAddr, quint16 value);

    // Modbus命令 - 写入多个
    QByteArray buildWriteMultipleCoilsFrame(quint8 slaveAddr, quint16 startAddr,
                                            const QByteArray &coilValues);
    QByteArray buildWriteMultipleRegistersFrame(quint8 slaveAddr, quint16 startAddr,
                                                const QVector<quint16> &registerValues);

    // 发送数据
    bool sendFrame(const QByteArray &frame);

    // CRC计算
    static quint16 calculateCRC16(const QByteArray &data);
    static QString crcToHex(quint16 crc);

    // 解析响应
    static ModbusFrame parseFrame(const QByteArray &data);
    static QString getFunctionCodeName(quint8 code);
    static QString getExceptionName(quint8 code);

    // 数据转换
    static quint16 bytesToUint16(quint8 high, quint8 low);
    static QByteArray uint16ToBytes(quint16 value);
    static QString bytesToHexString(const QByteArray &data);
    static QByteArray hexStringToBytes(const QString &hexStr);

signals:
    // 数据接收
    void dataReceived(const QByteArray &data);
    // 通信错误
    void errorOccurred(const QString &error);
    // 连接状态变化
    void connectionStateChanged(bool connected);

private slots:
    void onReadyRead();
    void onSerialError(QSerialPort::SerialPortError error);

private:
    QSerialPort *m_serialPort;
    QByteArray m_receiveBuffer;

    // 从缓冲区提取完整帧
    QList<QByteArray> extractFrames();
};

#endif // MODBUSRTUMASTER_H
