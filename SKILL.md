---
name: modbus-rtu-generator
description: |
  This skill should be used when the user wants to create a Modbus RTU master communication application using Qt. It provides complete project templates, Modbus protocol implementation, and step-by-step guidance for building a Modbus RTU debugging tool. Triggers: "create modbus rtu", "modbus rtu上位机", "modbus", "build modbus master".
---

# Modbus RTU Generator

Generate a complete Modbus RTU master communication application based on Qt.

## Project Structure

```
project-folder/
├── ModbusRTUMaster.pro    # Qt project file
├── main.cpp               # Application entry point
├── mainwindow.h/cpp       # Main window
├── mainwindow.ui          # UI file
├── modbusrtumaster.h/cpp  # Modbus protocol implementation
└── README.md              # Documentation
```

## Modbus RTU Protocol

### Frame Format

```
| Slave Address | Function Code | Data | CRC16 |
|    1 byte    |    1 byte    | N bytes | 2 bytes |
```

### Supported Function Codes

| Code | Name | Description |
|------|------|-------------|
| 0x01 | Read Coils | Read multiple coils |
| 0x02 | Read Discrete Inputs | Read discrete inputs |
| 0x03 | Read Holding Registers | Read holding registers |
| 0x04 | Read Input Registers | Read input registers |
| 0x05 | Write Single Coil | Write single coil |
| 0x06 | Write Single Register | Write single register |
| 0x0F | Write Multiple Coils | Write multiple coils |
| 0x10 | Write Multiple Registers | Write multiple registers |

### CRC16 Calculation

Modbus uses standard CRC16 with polynomial 0xA001 (reflected):

```cpp
quint16 ModbusRTUMaster::calculateCRC16(const QByteArray &data)
{
    quint16 crc = 0xFFFF;
    for (int i = 0; i < data.size(); ++i) {
        crc ^= data[i];
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
```

## Usage

### Step 1: Copy Project Templates

Copy all files from `assets/` directory to the target project folder:

- `ModbusRTUMaster.pro`
- `main.cpp`
- `mainwindow.h`
- `mainwindow.cpp`
- `mainwindow.ui`
- `modbusrtumaster.h`
- `modbusrtumaster.cpp`

### Step 2: Open in Qt Creator

1. Open `ModbusRTUMaster.pro` in Qt Creator
2. Select a Kit (e.g., MinGW)
3. Run qmake
4. Build and run

### Step 3: Configure Project (if needed)

If file names or structure need to be customized:

1. Update `.pro` file to match actual file names
2. Ensure UI class name in `.ui` matches `ui_mainwindow.h`
3. Verify all widget names in `.ui` match code references

## Common Issues and Solutions

### Issue: Missing `QVector` include
**Solution**: Add `#include <QVector>` in mainwindow.cpp

### Issue: `ModbusFrame` type not recognized
**Solution**: Include `modbusrtumaster.h` in mainwindow.h

### Issue: Unused slot function declared
**Solution**: Remove slots that have no corresponding implementation

### Issue: UI widget name mismatch
**Solution**: Ensure all `ui->widgetName` references match names in `.ui` file

## Widget Names Reference

UI widgets and their object names:

| Widget Type | Object Name | Purpose |
|------------|-------------|---------|
| QComboBox | comboBoxPort | Serial port selection |
| QComboBox | comboBoxBaud | Baud rate selection |
| QComboBox | comboBoxDataBits | Data bits selection |
| QComboBox | comboBoxParity | Parity selection |
| QComboBox | comboBoxStopBits | Stop bits selection |
| QComboBox | comboBoxFunctionCode | Modbus function code |
| QSpinBox | spinBoxSlave | Slave address (1-247) |
| QSpinBox | spinBoxAddress | Start address (0-65535) |
| QSpinBox | spinBoxQuantity | Quantity/value |
| QPushButton | btnRefresh | Refresh ports |
| QPushButton | btnOpenClose | Open/close serial port |
| QPushButton | btnRead | Read operation |
| QPushButton | btnWrite | Write operation |
| QPushButton | btnClearLog | Clear log |
| QPushButton | btnSaveLog | Save log |
| QTextEdit | textEditLog | Communication log |
| QLabel | labelTxBytes | TX byte count |
| QLabel | labelRxBytes | RX byte count |
| QLabel | labelFrameCount | Frame count |
| QLabel | labelStatus | Connection status |

## Default Settings

| Parameter | Default Value |
|-----------|---------------|
| Baud Rate | 115200 |
| Data Bits | 8 |
| Parity | None |
| Stop Bits | 1 |
| Slave Address | 1 |
| Function Code | 0x03 (Read Holding Registers) |
| Start Address | 0 |
| Quantity | 10 |
