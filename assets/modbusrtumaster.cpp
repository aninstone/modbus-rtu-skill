#include "modbusrtumaster.h"
#include <QDebug>

ModbusRTUMaster::ModbusRTUMaster(QObject *parent)
    : QObject(parent)
    , m_serialPort(new QSerialPort(this))
{
    // 连接串口信号
    connect(m_serialPort, &QSerialPort::readyRead, this, &ModbusRTUMaster::onReadyRead);
    connect(m_serialPort, &QSerialPort::errorOccurred, this, &ModbusRTUMaster::onSerialError);
}

ModbusRTUMaster::~ModbusRTUMaster()
{
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
    }
}

/**
 * @brief 打开串口
 */
bool ModbusRTUMaster::openPort(const QString &portName, qint32 baudRate,
                               QSerialPort::DataBits dataBits,
                               QSerialPort::Parity parity,
                               QSerialPort::StopBits stopBits)
{
    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(baudRate);
    m_serialPort->setDataBits(dataBits);
    m_serialPort->setParity(parity);
    m_serialPort->setStopBits(stopBits);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (m_serialPort->open(QIODevice::ReadWrite)) {
        emit connectionStateChanged(true);
        return true;
    }

    emit errorOccurred(m_serialPort->errorString());
    return false;
}

/**
 * @brief 关闭串口
 */
void ModbusRTUMaster::closePort()
{
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
        emit connectionStateChanged(false);
    }
}

/**
 * @brief 检查串口是否打开
 */
bool ModbusRTUMaster::isOpen() const
{
    return m_serialPort->isOpen();
}

/**
 * @brief 构建读帧
 */
QByteArray ModbusRTUMaster::buildReadFrame(quint8 slaveAddr, ModbusFunctionCode funcCode,
                                           quint16 startAddr, quint16 quantity)
{
    return buildReadFrame(slaveAddr, static_cast<quint8>(funcCode), startAddr, quantity);
}

QByteArray ModbusRTUMaster::buildReadFrame(quint8 slaveAddr, quint8 funcCode,
                                           quint16 startAddr, quint16 quantity)
{
    QByteArray frame;
    frame.append(static_cast<char>(slaveAddr));
    frame.append(static_cast<char>(funcCode));
    frame.append(static_cast<char>((startAddr >> 8) & 0xFF));
    frame.append(static_cast<char>(startAddr & 0xFF));
    frame.append(static_cast<char>((quantity >> 8) & 0xFF));
    frame.append(static_cast<char>(quantity & 0xFF));

    quint16 crc = calculateCRC16(frame);
    frame.append(static_cast<char>(crc & 0xFF));
    frame.append(static_cast<char>((crc >> 8) & 0xFF));

    return frame;
}

/**
 * @brief 构建写单个线圈帧
 */
QByteArray ModbusRTUMaster::buildWriteSingleCoilFrame(quint8 slaveAddr, quint16 coilAddr, bool value)
{
    QByteArray frame;
    frame.append(static_cast<char>(slaveAddr));
    frame.append(static_cast<char>(0x05)); // 功能码
    frame.append(static_cast<char>((coilAddr >> 8) & 0xFF));
    frame.append(static_cast<char>(coilAddr & 0xFF));
    frame.append(value ? static_cast<char>(0xFF) : static_cast<char>(0x00));
    frame.append(static_cast<char>(0x00));

    quint16 crc = calculateCRC16(frame);
    frame.append(static_cast<char>(crc & 0xFF));
    frame.append(static_cast<char>((crc >> 8) & 0xFF));

    return frame;
}

/**
 * @brief 构建写单个寄存器帧
 */
QByteArray ModbusRTUMaster::buildWriteSingleRegisterFrame(quint8 slaveAddr, quint16 regAddr, quint16 value)
{
    QByteArray frame;
    frame.append(static_cast<char>(slaveAddr));
    frame.append(static_cast<char>(0x06)); // 功能码
    frame.append(static_cast<char>((regAddr >> 8) & 0xFF));
    frame.append(static_cast<char>(regAddr & 0xFF));
    frame.append(static_cast<char>((value >> 8) & 0xFF));
    frame.append(static_cast<char>(value & 0xFF));

    quint16 crc = calculateCRC16(frame);
    frame.append(static_cast<char>(crc & 0xFF));
    frame.append(static_cast<char>((crc >> 8) & 0xFF));

    return frame;
}

/**
 * @brief 构建写多个线圈帧
 */
QByteArray ModbusRTUMaster::buildWriteMultipleCoilsFrame(quint8 slaveAddr, quint16 startAddr,
                                                         const QByteArray &coilValues)
{
    quint8 byteCount = coilValues.size();

    QByteArray frame;
    frame.append(static_cast<char>(slaveAddr));
    frame.append(static_cast<char>(0x0F)); // 功能码
    frame.append(static_cast<char>((startAddr >> 8) & 0xFF));
    frame.append(static_cast<char>(startAddr & 0xFF));
    frame.append(static_cast<char>((byteCount * 8) >> 8));
    frame.append(static_cast<char>(byteCount * 8));
    frame.append(static_cast<char>(byteCount));
    frame.append(coilValues);

    quint16 crc = calculateCRC16(frame);
    frame.append(static_cast<char>(crc & 0xFF));
    frame.append(static_cast<char>((crc >> 8) & 0xFF));

    return frame;
}

/**
 * @brief 构建写多个寄存器帧
 */
QByteArray ModbusRTUMaster::buildWriteMultipleRegistersFrame(quint8 slaveAddr, quint16 startAddr,
                                                              const QVector<quint16> &registerValues)
{
    quint16 quantity = registerValues.size();
    quint8 byteCount = quantity * 2;

    QByteArray frame;
    frame.append(static_cast<char>(slaveAddr));
    frame.append(static_cast<char>(0x10)); // 功能码
    frame.append(static_cast<char>((startAddr >> 8) & 0xFF));
    frame.append(static_cast<char>(startAddr & 0xFF));
    frame.append(static_cast<char>((quantity >> 8) & 0xFF));
    frame.append(static_cast<char>(quantity & 0xFF));
    frame.append(static_cast<char>(byteCount));

    // 添加寄存器值
    for (quint16 reg : registerValues) {
        frame.append(static_cast<char>((reg >> 8) & 0xFF));
        frame.append(static_cast<char>(reg & 0xFF));
    }

    quint16 crc = calculateCRC16(frame);
    frame.append(static_cast<char>(crc & 0xFF));
    frame.append(static_cast<char>((crc >> 8) & 0xFF));

    return frame;
}

/**
 * @brief 发送帧
 */
bool ModbusRTUMaster::sendFrame(const QByteArray &frame)
{
    if (!m_serialPort->isOpen()) {
        emit errorOccurred("串口未打开");
        return false;
    }

    qint64 written = m_serialPort->write(frame);
    m_serialPort->flush();

    return written == frame.size();
}

/**
 * @brief 计算CRC16 (Modbus标准)
 */
quint16 ModbusRTUMaster::calculateCRC16(const QByteArray &data)
{
    quint16 crc = 0xFFFF;

    for (int i = 0; i < data.size(); ++i) {
        crc ^= static_cast<quint8>(data[i]);

        for (int j = 0; j < 8; ++j) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }

    return crc;
}

/**
 * @brief CRC转十六进制字符串
 */
QString ModbusRTUMaster::crcToHex(quint16 crc)
{
    return QString("%1").arg(crc, 4, 16, QChar('0')).toUpper();
}

/**
 * @brief 解析帧
 */
ModbusFrame ModbusRTUMaster::parseFrame(const QByteArray &data)
{
    ModbusFrame frame;
    frame.isValid = false;
    frame.isException = false;

    // 最小帧长度: 地址(1) + 功能码(1) + CRC(2) = 4字节
    if (data.size() < 4) {
        return frame;
    }

    // 提取地址和功能码
    frame.slaveAddress = static_cast<quint8>(data[0]);
    frame.functionCode = static_cast<quint8>(data[1]);

    // 检查是否为异常响应
    if (frame.functionCode & 0x80) {
        frame.isException = true;
        frame.exceptionCode = frame.functionCode & 0x7F;

        if (data.size() >= 5) { // 地址 + 功能码(异常) + 异常码 + CRC(2)
            frame.data = data.mid(2, 1);
            quint16 receivedCRC = (static_cast<quint8>(data[3]) << 8) | static_cast<quint8>(data[4]);
            quint16 calculatedCRC = calculateCRC16(data.left(3));
            frame.crc = receivedCRC;
            frame.isValid = (receivedCRC == calculatedCRC);
        }
        return frame;
    }

    // 正常响应处理
    switch (frame.functionCode) {
        case 0x01: // 读线圈
        case 0x02: // 读离散输入
        case 0x03: // 读保持寄存器
        case 0x04: // 读输入寄存器
            if (data.size() >= 5) {
                quint8 byteCount = static_cast<quint8>(data[2]);
                int minSize = 3 + byteCount + 2; // 地址 + 功能码 + 字节数 + 数据 + CRC
                if (data.size() >= minSize) {
                    frame.data = data.mid(2, 1 + byteCount);
                    quint16 receivedCRC = (static_cast<quint8>(data[minSize - 1]) << 8) |
                                          static_cast<quint8>(data[minSize - 2]);
                    quint16 calculatedCRC = calculateCRC16(data.left(minSize - 2));
                    frame.crc = receivedCRC;
                    frame.isValid = (receivedCRC == calculatedCRC);
                }
            }
            break;

        case 0x05: // 写单个线圈
        case 0x06: // 写单个寄存器
            if (data.size() >= 8) { // 完整帧: 地址 + 功能码 + 地址/值(2) + CRC(2)
                frame.data = data.mid(2, 4);
                quint16 receivedCRC = (static_cast<quint8>(data[7]) << 8) | static_cast<quint8>(data[6]);
                quint16 calculatedCRC = calculateCRC16(data.left(6));
                frame.crc = receivedCRC;
                frame.isValid = (receivedCRC == calculatedCRC);
            }
            break;

        case 0x0F: // 写多个线圈
        case 0x10: // 写多个寄存器
            if (data.size() >= 8) { // 完整帧: 地址 + 功能码 + 起始地址(2) + 数量(2) + CRC(2)
                frame.data = data.mid(2, 4);
                quint16 receivedCRC = (static_cast<quint8>(data[7]) << 8) | static_cast<quint8>(data[6]);
                quint16 calculatedCRC = calculateCRC16(data.left(6));
                frame.crc = receivedCRC;
                frame.isValid = (receivedCRC == calculatedCRC);
            }
            break;

        default:
            qDebug() << "Unknown function code:" << QString::number(frame.functionCode, 16);
            break;
    }

    return frame;
}

/**
 * @brief 获取功能码名称
 */
QString ModbusRTUMaster::getFunctionCodeName(quint8 code)
{
    switch (code & 0x7F) { // 忽略异常标志
        case 0x01: return "读线圈 (0x01)";
        case 0x02: return "读离散输入 (0x02)";
        case 0x03: return "读保持寄存器 (0x03)";
        case 0x04: return "读输入寄存器 (0x04)";
        case 0x05: return "写单个线圈 (0x05)";
        case 0x06: return "写单个寄存器 (0x06)";
        case 0x0F: return "写多个线圈 (0x0F)";
        case 0x10: return "写多个寄存器 (0x10)";
        default: return QString("未知 (0x%1)").arg(code, 2, 16, QChar('0')).toUpper();
    }
}

/**
 * @brief 获取异常名称
 */
QString ModbusRTUMaster::getExceptionName(quint8 code)
{
    switch (code) {
        case 0x01: return "非法功能码";
        case 0x02: return "非法数据地址";
        case 0x03: return "非法数据值";
        case 0x04: return "从机故障";
        case 0x05: return "确认";
        case 0x06: return "从机忙";
        case 0x08: return "内存奇偶校验错误";
        case 0x0A: return "网关路径不可用";
        case 0x0B: return "网关设备响应失败";
        default: return QString("未知异常 (0x%1)").arg(code, 2, 16, QChar('0')).toUpper();
    }
}

/**
 * @brief 字节转uint16
 */
quint16 ModbusRTUMaster::bytesToUint16(quint8 high, quint8 low)
{
    return (static_cast<quint16>(high) << 8) | low;
}

/**
 * @brief uint16转字节
 */
QByteArray ModbusRTUMaster::uint16ToBytes(quint16 value)
{
    QByteArray bytes;
    bytes.append(static_cast<char>((value >> 8) & 0xFF));
    bytes.append(static_cast<char>(value & 0xFF));
    return bytes;
}

/**
 * @brief 字节数组转十六进制字符串
 */
QString ModbusRTUMaster::bytesToHexString(const QByteArray &data)
{
    QString hex;
    for (int i = 0; i < data.size(); ++i) {
        hex += QString("%1 ").arg(static_cast<quint8>(data[i]), 2, 16, QChar('0')).toUpper();
    }
    return hex.trimmed();
}

/**
 * @brief 十六进制字符串转字节数组
 */
QByteArray ModbusRTUMaster::hexStringToBytes(const QString &hexStr)
{
    QByteArray data;
    QStringList hexList = hexStr.split(' ', Qt::SkipEmptyParts);

    for (const QString &hex : hexList) {
        bool ok;
        quint8 byte = hex.toUInt(&ok, 16);
        if (ok) {
            data.append(static_cast<char>(byte));
        }
    }

    return data;
}

/**
 * @brief 串口数据接收
 */
void ModbusRTUMaster::onReadyRead()
{
    m_receiveBuffer.append(m_serialPort->readAll());

    // 提取完整帧
    QList<QByteArray> frames = extractFrames();
    for (const QByteArray &frame : frames) {
        emit dataReceived(frame);
    }
}

/**
 * @brief 从缓冲区提取完整帧
 */
QList<QByteArray> ModbusRTUMaster::extractFrames()
{
    QList<QByteArray> frames;

    // 简单策略: 如果缓冲区有至少4字节，尝试提取一帧
    // 在实际应用中可能需要更复杂的帧边界检测逻辑
    while (m_receiveBuffer.size() >= 4) {
        // 尝试解析当前帧
        quint8 funcCode = static_cast<quint8>(m_receiveBuffer[1]) & 0x7F;
        int frameSize = 0;

        // 根据功能码确定帧长度
        if (funcCode >= 0x01 && funcCode <= 0x04) {
            // 读操作: 需要至少5字节才能知道数据长度
            if (m_receiveBuffer.size() >= 5) {
                quint8 byteCount = static_cast<quint8>(m_receiveBuffer[2]);
                frameSize = 5 + byteCount; // 地址 + 功能码 + 字节数 + 数据 + CRC
            }
        } else if (funcCode == 0x05 || funcCode == 0x06) {
            frameSize = 8; // 固定长度帧
        } else if (funcCode == 0x0F || funcCode == 0x10) {
            frameSize = 8; // 固定长度帧
        }

        // 如果无法确定帧大小，至少提取4字节(最小帧)
        if (frameSize == 0) {
            frameSize = 4;
        }

        // 确保缓冲区有足够的字节
        if (m_receiveBuffer.size() >= frameSize) {
            frames.append(m_receiveBuffer.left(frameSize));
            m_receiveBuffer.remove(0, frameSize);
        } else {
            break; // 数据不完整，等待更多数据
        }
    }

    return frames;
}

/**
 * @brief 串口错误处理
 */
void ModbusRTUMaster::onSerialError(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError && error != QSerialPort::TimeoutError) {
        emit errorOccurred(m_serialPort->errorString());

        if (error == QSerialPort::ResourceError || error == QSerialPort::DeviceNotFoundError) {
            closePort();
        }
    }
}
