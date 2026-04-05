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

## ⚠️ UI文件生成注意事项（重要）

如果需要修改或扩展 `.ui` 文件，必须遵守以下规则：

### ❌ 禁止在 QScrollArea 上使用 placeholderText

**QScrollArea 不支持 `placeholderText` 属性**，这是 Qt 5 的 QLineEdit/QTextEdit 等输入控件才有的属性。

```xml
<!-- ❌ 错误：QScrollArea 没有 placeholderText -->
<widget class="QScrollArea" name="scrollArea">
  <property name="placeholderText">
    <string/>
  </property>
  ...
</widget>

<!-- ✅ 正确：直接省略 placeholderText -->
<widget class="QScrollArea" name="scrollArea">
  <property name="widgetResizable">
    <bool>true</bool>
  </property>
  ...
</widget>
```

### XML标签闭合规则

```xml
<!-- ✅ 正确写法 -->
<item>
  <widget class="QLabel" name="labelName">
    <property name="text">
      <string>文字内容</string>
    </property>
  </widget>
</item>

<!-- ❌ 错误写法：缺少 </widget> -->
<item>
  <widget class="QLabel" name="labelName">
    <property name="text">
      <string>文字内容</string>
    </property>
  <item>  <!-- 错误：应该是</widget> -->
```

### 关键点

1. **每个 `<widget>` 必须有对应的 `</widget>` 闭合**
2. **`<property>` 标签必须完全包含在 `<widget>` 内部**
3. **生成UI后必须验证**

### 验证方法

```bash
uic mainwindow.ui -o ui_mainwindow.h
```
如果没有报错，说明UI文件格式正确。

## ⚠️ 头文件 include 完整性检查（重要）

在 `.h` 头文件中使用任何 Qt 控件类型时，必须包含对应的头文件：

| 控件类型 | 必须添加的头文件 |
|---------|----------------|
| `QLineEdit*` | `#include <QLineEdit>` |
| `QTextEdit*` | `#include <QTextEdit>` |
| `QPushButton*` | `#include <QPushButton>` |
| `QComboBox*` | `#include <QComboBox>` |
| `QSpinBox*` | `#include <QSpinBox>` |
| `QTableWidget*` | `#include <QTableWidget>` |
| `QTabWidget*` | `#include <QTabWidget>` |
| `QTimer*` | `#include <QTimer>` |
| `QLabel*` | `#include <QLabel>` |
| `QCheckBox*` | `#include <QCheckBox>` |
| `QRadioButton*` | `#include <QRadioButton>` |
| `QSlider*` | `#include <QSlider>` |
| `QProgressBar*` | `#include <QProgressBar>` |

### 检查清单

生成 `.h` 文件后，逐一检查：
1. [ ] 每个自定义 slot 函数是否在 `.h` 中声明
2. [ ] 每个返回 Qt 控件指针的函数是否有对应的 `#include`
3. [ ] `.cpp` 中 include 的头文件是否也在 `.h` 中 include（如果类型在 `.h` 中出现）

## 📝 创建多页面项目 - 提示词模板

### 基本模板

```
帮我创建一个新的HMI页面，要求：
1. 使用 QTabWidget 添加一个"页面名称"标签页
2. 左边放 N 个参数标签（QLabel）
3. 右边放 N 个输入框（QLineEdit）
4. 每个参数旁边放一个"写入"按钮
5. 页面底部有"读取全部"和"写入全部"按钮
6. 修改 mainwindow.h/cpp/ui 三个文件
7. 确保 mainwindow.h 中包含所有需要的头文件（如 QLineEdit）
```

### 完整示例（已验证成功）

```
用 https://github.com/aninstone/modbus-rtu-skill 在 D:/WorkPRJ/QT PRJ 文件夹下创建一个 Modbus RTU 上位机工程，工程名为 motor_control。

要求：
1. 添加一个"电源参数"页面（Tab页面）：
   - 左侧参数标签：电机转速、目标转速、加速时间、减速时间、运行模式、转向控制、过流保护、过速保护、位置闭环使能、使能输出
   - 右侧对应10个输入框（QLineEdit）
   - 每行右侧有"写入"按钮
   - 底部有"读取全部"和"写入全部"按钮

2. 添加一个"运行监控"页面（Tab页面）：
   - 左侧参数标签：当前转速、当前位置、输入状态、输出状态、母线电压、驱动温度、运行时间、故障代码、位置偏差、目标位置
   - 右侧对应10个只读显示框（QSpinBox或QLineEdit只读）
   - 底部有"启动"、"停止"、"复位"三个控制按钮

3. 修改三个文件：mainwindow.h / mainwindow.cpp / mainwindow.ui
4. 确保 mainwindow.h 中包含所有需要的头文件（如 QLineEdit、QSpinBox、QPushButton 等）
5. UI文件标签必须正确闭合，每个 widget 都要有对应的 </widget>
6. 生成后运行 uic mainwindow.ui 验证UI文件格式
```

### 修改文件清单

生成新页面时，必须同时修改以下文件：

| 文件 | 修改内容 |
|-----|---------|
| `mainwindow.ui` | 添加页面布局、控件（注意标签闭合） |
| `mainwindow.h` | 添加 slot 声明、helper 函数、include 头文件 |
| `mainwindow.cpp` | 实现 slot 函数、连接信号槽 |

### 页面命名规范

| 页面类型 | 控件前缀 |
|---------|---------|
| 手动控制 | `tabManual` / `btnManual` |
| 参数设置 | `tabMotor` / `lineEditMotor0-9` / `btnMotorWrite0-9` |
| 运行监控 | `tabMonitor` / `lineEditMonitor0-9` / `btnMonitorStart/Stop/Reset` |
| HMI控制 | `tabHMI` / `lineEditHMI0-9` / `btnHMIWrite0-9` |

### 寄存器分配建议

| 页面 | 寄存器范围 | 说明 |
|------|-----------|------|
| 电源参数 | R0-R9 | 可读写参数 |
| 运行监控 | R100-R109 | 只读状态 |
| HMI控制 | R0-R9 | 可读写参数 |
