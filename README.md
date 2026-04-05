# Modbus RTU Generator Skill

一个用于快速创建 Modbus RTU 上位机应用的 CodeBuddy Skill。

## 功能特点

- ✅ 完整的 Qt 项目模板
- ✅ 支持 Modbus RTU 协议（0x01 - 0x10 功能码）
- ✅ 内置 CRC16 校验计算
- ✅ 串口自动检测与配置
- ✅ 实时收发统计
- ✅ 日志记录与导出

## 支持的功能码

| 功能码 | 功能 | 读写 |
|--------|------|------|
| 0x01 | 读线圈 | 读 |
| 0x02 | 读离散输入 | 读 |
| 0x03 | 读保持寄存器 | 读 |
| 0x04 | 读输入寄存器 | 读 |
| 0x05 | 写单个线圈 | 写 |
| 0x06 | 写单个寄存器 | 写 |
| 0x0F | 写多个线圈 | 写 |
| 0x10 | 写多个寄存器 | 写 |

## 项目结构

```
modbus-rtu-skill/
├── assets/
│   ├── ModbusRTUMaster.pro     # Qt 项目文件
│   ├── main.cpp               # 程序入口
│   ├── mainwindow.h           # 主窗口头文件
│   ├── mainwindow.cpp         # 主窗口实现
│   ├── mainwindow.ui          # UI 界面文件
│   ├── modbusrtumaster.h      # Modbus 协议头文件
│   └── modbusrtumaster.cpp     # Modbus 协议实现
└── README.md
```

## 使用方法

### 方式一：在 CodeBuddy 中直接使用

在 CodeBuddy 中说：
```
用这个skill创建一个Modbus RTU上位机
```

CodeBuddy 会自动使用此模板生成项目。

### 方式二：克隆到本地

```bash
git clone https://github.com/aninstone/modbus-rtu-skill.git
```

将 `assets/` 文件夹中的文件复制到你的 Qt 项目中即可。

## ⚠️ 重要：安装 Qt SerialPort 库

**首次使用前必须安装**，否则编译会报错！

### 方法一：Qt Maintenance Tool（推荐）

1. 打开 **Qt Maintenance Tool**
   - 位置：`C:/Qt/MaintenanceTool.exe`

2. 点击 **添加或移除组件**

3. 展开 **Qt 6.x.x** → **库**

4. ✅ 勾选 **Qt Serial Port**

5. 点击 **下一步** 完成安装

### 方法二：命令行安装

```bash
pip install aqtinstall
aqt install-qt windows desktop 6.5.3 win64_msvc2019_64 -O Qt6
```

### 验证安装成功

编译时无 "QSerialPort: No such file" 错误即成功。

---

## 在 Qt Creator 中运行

1. 打开 `ModbusRTUMaster.pro`
2. 如有错误先安装 SerialPort（如上）
3. 构建 → 重新构建所有
4. 运行 ▶️

---

## ⚠️ UI文件生成注意事项（重要）

如果需要修改或扩展 `.ui` 文件，必须遵守以下规则：

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

---

## ⚠️ UI 样式注意事项（重要）

生成 UI 控件时，必须设置正确的文字颜色，避免看不清：

### 常见问题

| 问题 | 原因 | 解决方案 |
|------|------|---------|
| 输入框文字看不清 | 浅色背景 + 白色文字 | 设置 `color: #2c3e50;` |
| 日志框文字看不清 | 浅色背景 + 白色文字 | 设置 `color: #2c3e50;` |

### 正确示例

```cpp
// ✅ 正确：深色文字在浅色背景上清晰可见
edit->setStyleSheet(
    "QLineEdit { "
    "padding: 5px; "
    "border: 2px solid #3498db; "
    "border-radius: 3px; "
    "background-color: #ecf0f1; "
    "font-size: 14px; "
    "color: #2c3e50; "  // 深灰色文字
    "}"
);
```

```xml
<!-- ✅ 正确：在 UI 文件中添加 styleSheet 属性 -->
<widget class="QTextEdit" name="textEditLog">
    <property name="readOnly">
        <bool>true</bool>
    </property>
    <property name="styleSheet">
        <string>color: #2c3e50;</string>
    </property>
</widget>
```

### 错误示例

```cpp
// ❌ 错误：白色文字在浅色背景上看不清
edit->setStyleSheet(
    "QLineEdit { "
    "background-color: #ecf0f1; "
    "color: white; "  // 白色文字
    "}"
);
```

### 推荐颜色值

| 用途 | 颜色代码 | 效果 |
|------|---------|------|
| 深色文字 | `#2c3e50` | 在浅色背景上清晰可见 |
| 深灰色 | `#333333` | 同样适合浅色背景 |
| 黑色 | `#000000` | 标准黑色文字 |

---

## ⚠️ 头文件 include 完整性检查（重要）

### 核心原则

**在 `.h` 文件中出现的任何 Qt 类型，都必须在文件顶部添加对应的 `#include`**！

这不是建议，是必须遵守的规则！

### 常见错误案例

```cpp
// ❌ 错误：只加了 QLineEdit，漏了 QPushButton
#include <QLineEdit>

class MainWindow : public QMainWindow {
private:
    QVector<QLineEdit*> motorParamEdits;           // ← 加了 include
    QVector<QPushButton*> motorParamWriteButtons;  // ← 漏了！报错！
    QPushButton *btnMonitorStart;                  // ← 漏了！报错！
};
```

```cpp
// ✅ 正确：所有用到的类型都要 include
#include <QLineEdit>
#include <QPushButton>
#include <QVector>

class MainWindow : public QMainWindow {
private:
    QVector<QLineEdit*> motorParamEdits;
    QVector<QPushButton*> motorParamWriteButtons;
    QPushButton *btnMonitorStart;
};
```

### 常用控件头文件清单

| 控件类型 | 必须添加的头文件 |
|---------|----------------|
| `QLineEdit*` | `#include <QLineEdit>` |
| `QTextEdit*` | `#include <QTextEdit>` |
| `QPushButton*` | `#include <QPushButton>` |
| `QComboBox*` | `#include <QComboBox>` |
| `QSpinBox*` | `#include <QSpinBox>` |
| `QTimer*` | `#include <QTimer>` |
| `QLabel*` | `#include <QLabel>` |
| `QTabWidget*` | `#include <QTabWidget>` |
| `QVector*` | `#include <QVector>` |

### 检查流程（必须逐项核对）

生成 `.h` 文件后，按以下顺序检查：

1. [ ] **扫描所有成员变量声明**，找出所有 Qt 指针类型
2. [ ] **对照上表**，确保每个类型都有对应的 `#include`
3. [ ] **检查 private slots**，确保所有 slot 都有声明
4. [ ] **检查返回值**，如果函数返回 Qt 控件指针，对应类型也要 include
5. [ ] **对照检查一遍**：打开 UI 文件，列出所有控件，确认 .h 中都有 include

### 快速检查技巧

```cpp
// 在 .h 文件中搜索这些关键字，确保都有对应的 include
grep -n "QLineEdit" mainwindow.h  // 检查是否有 QLineEdit* 但没有 include
grep -n "QPushButton" mainwindow.h // 检查是否有 QPushButton* 但没有 include
```

---

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

### 完整示例（✅ 已验证成功）

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

---

## 界面预览

```
┌─────────────────────────────────────────────┐
│  串口设置          │  Modbus 设置           │
│  ─────────────     │  ─────────────         │
│  端口: [COM1 ▼]   │  从机地址: [1]        │
│  波特率: [115200▼] │  功能码: [0x03 ▼]     │
│  校验: [无 ▼]     │  起始地址: [0]        │
│  [刷新] [打开]    │  数量: [10]           │
├─────────────────────────────────────────────┤
│  数据格式: [Hex ▼]                           │
│  ┌─────────────────────────────────────────┐│
│  │  00 01 02 03 04 05 06 07  08 09 ...   ││
│  │  00 00 00 00 00 00 00 00  00 00 ...   ││
│  └─────────────────────────────────────────┘│
│  [读取数据]  [写入数据]  [清除]             │
├─────────────────────────────────────────────┤
│  发送统计    接收统计    帧计数             │
│  Tx: 0      Rx: 0      Frames: 0          │
├─────────────────────────────────────────────┤
│  日志:                                       │
│  [20:30:15.123] TX - 读保持寄存器: 01 03... │
│  [20:30:15.456] RX - 正常响应: 01 03 14...  │
└─────────────────────────────────────────────┘
```

## Modbus RTU 协议说明

### 帧格式

```
┌─────────┬────────┬──────────┬────────┬──────┬────────┐
│ 从机地址 │ 功能码 │  数据    │  数据  │ CRC  │  CRC   │
│  1字节  │ 1字节  │  N字节   │  N字节 │ 低字节│ 高字节 │
└─────────┴────────┴──────────┴────────┴──────┴────────┘
```

### CRC16 计算

采用 Modbus 标准 CRC16，初始值 0xFFFF，多项式 0xA001。

## 技术栈

- **框架**: Qt 5.x / Qt 6.x
- **语言**: C++
- **串口**: QSerialPort
- **协议**: Modbus RTU

## License

MIT License - 可以自由使用、修改和商用。

## 作者

GitHub: [@aninstone](https://github.com/aninstone)
