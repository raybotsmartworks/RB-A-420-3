# RB-A-420-3 Robotic Arm Control System - User Manual
# 机械臂控制使用说明书
# English / 中文

---

# English Version

# ESP32 6-Axis Robotic Arm Control System - User Manual

## 1. System Overview

This system is an ESP32-based 6-axis robotic arm control system with web browser remote control. It uses CAN bus to communicate with joint motors and supports teaching programming, motion playback, and data storage.

**Hardware Configuration:**
- Main Controller: ESP32
- Communication Bus: CAN (TX=26, RX=27)
- Joint Count: 6 + 1 gripper
- Motor ID Range: 1-7

## 2. Web Interface Usage

### 2.1 Connection

1. System creates WiFi AP on startup
2. AP Name: esp32
3. Default Password: 12345678
4. Access in browser: `http://192.168.4.1`

### 2.2 Interface Overview

Web console has 4 tabs:

#### (1) Status Monitor
- Status bar showing system status
- Joint angle grid (J1-J6 + gripper)
- "Read Angles" button to get current positions

#### (2) Teaching
- **Record**: Record current pose (max 10 poses)
- **Play**: Execute recorded sequence
- **Clear**: Clear teaching group in memory
- **Flash Storage**: Save/Load/Delete
- **Gripper Control**: Open/Close
- **Save Preset**: Save to preset 1-4
- **Emergency Stop**: Stop all motors

#### (3) Preset Actions
- **Run 1-4**: Execute saved preset actions

#### (4) Safety & Tools
- **Set All Zero**: Reset all joints to zero position

### 2.3 Typical Workflow

**Teaching Workflow:**
1. Move arm to target position
2. Click "Read Angles"
3. Click "Record"
4. Repeat for next positions
5. Click "Play" to execute sequence
6. Click "Save" to persist

**Preset Management:**
1. Complete teaching
2. Click "Preset 1" to save
3. Click "Run 1" to execute later

## 3. Function Modules

### 3.1 CAN Bus Communication (`rclib.cpp`)

| Function | Description |
|----------|-------------|
| `run(id, speed, pos)` | Multi-turn position control |
| `stop(id)` | Stop specified motor |
| `set_zero(id)` | Set current position as zero |
| `re_set(id)` | Reset specified motor |
| `read_Mc_angle(id)` | Read multi-turn angle |
| `read_Mc_encoder_p(id)` | Read encoder position |

### 3.2 Web Server (`main.cpp`)

| Endpoint | Parameter | Description |
|----------|-----------|-------------|
| `/` | GET | Web control page |
| `/cmd` | `action=xxx` | Execute command |
| `/status` | GET | JSON status |

**Commands:**
- `readJoints` - Read joint angles
- `record` - Record pose
- `runTeach` - Play teaching
- `clearTeach` - Clear
- `saveFlash` / `loadFlash` / `deleteFlash` - Flash operations
- `clawOpen` / `clawClose` - Gripper
- `savePreset1-4` - Save preset
- `preset1-4` - Run preset
- `emergency` - Emergency stop
- `setZero` - Set all zero

### 3.3 Data Storage

Uses ESP32 NVS Flash for persistent storage:
- **Teaching**: Max 10 poses, FNV-1a checksum
- **Presets**: 4 groups, max 10 poses each

### 3.4 Motion Sequence

```cpp
struct Pose {
    int16_t joint[6];   // 6 joint angles
    uint16_t holdMs;      // Hold time in ms
};
```

## 4. Pin Definitions (`config.h`)

| Function | GPIO |
|----------|------|
| CAN_TX | GPIO26 |
| CAN_RX | GPIO27 |
| CAN_EN | GPIO23 |
| RS485_EN | GPIO17 |
| 5V_EN | GPIO16 |

## 5. Important Notes

1. **Emergency Stop**: Click immediately if abnormal
2. **CAN Connection**: Ensure CAN bus is connected
3. **Flash Storage**: Teaching data persists after power loss
4. **Zero Position**: Ensure arm is at physical zero before "Set Zero"
5. **Safety**: Do not touch arm during motion execution

---

# 中文版本

# ESP32 六轴机械臂控制系统使用说明

## 一、系统概述

本系统是基于ESP32微控制器的六轴机械臂控制系统，通过Web浏览器进行远程控制。系统采用CAN总线与各关节电机通信，支持示教编程、动作回放、数据存储等功能。

**硬件配置：**
- 主控芯片：ESP32
- 通信总线：CAN (TX=26, RX=27)
- 关节数量：6个 + 1个夹爪
- 电机ID范围：1-7

## 二、Web端使用说明

### 2.1 连接方式

1. 系统启动后会自动创建WiFi热点
2. 热点的名称为：esp32
3. 默认密码：12345678
4. 连接后，在浏览器中访问：`http://192.168.4.1`

### 2.2 界面介绍

Web控制台包含四个功能标签页：

#### (1) 状态监控
- 状态显示栏：显示系统当前状态和提示信息
- 关节角度网格：实时显示J1-J6六个关节角度和夹爪位置
- 读取角度按钮：点击获取当前各关节角度

#### (2) 示教
- 记录：将当前机械臂位姿记录到示教组（最多10个位姿）
- 执行：按顺序回放已记录的示教动作
- 清空：清空当前示教组（内存）
- Flash存储：保存/加载/删除示教数据
- 夹爪控制：张开/闭合
- 写入预设：将示教组保存到预设1-4
- 紧急停止：立即停止所有电机动作

#### (3) 预设动作
- 运行1-4：直接运行预设1-4中保存的动作组

#### (4) 安全与工具
- 全关节置零：将所有关节设置为零点位置

### 2.3 典型操作流程

示教编程流程：
1. 手动将机械臂移动到第一个目标位置
2. 点击读取角度确认当前位姿
3. 点击记录保存第一个位姿
4. 移动到下一个位置，重复记录步骤
5. 记录完成后点击执行回放整个动作序列
6. 如需保存，点击保存存入Flash

预设管理流程：
1. 先完成示教编程
2. 点击写入预设中的预设1将示教组保存到预设1
3. 之后可直接在预设动作页面点击运行1执行

## 三、功能模块介绍

### 3.1 CAN总线通信模块 (`rclib.cpp`)

| 功能 | 函数 | 说明 |
|------|------|------|
| 电机运行 | run(id, speed, pos) | 多圈位置控制 |
| 电机停止 | stop(id) | 停止指定电机 |
| 设置零点 | set_zero(id) | 设置当前位姿为零点 |
| 电机复位 | re_set(id) | 复位指定电机 |
| 读取角度 | read_Mc_angle(id) | 读取多圈角度 |
| 读取编码器 | read_Mc_encoder_p(id) | 读取多圈编码器位置 |

### 3.2 Web服务器模块 (`main.cpp`)

| 端点 | 参数 | 功能 |
|------|------|------|
| / | GET | 返回Web控制页面 |
| /cmd | action=xxx | 执行控制命令 |
| /status | GET | 返回JSON格式系统状态 |

支持的命令列表：
- readJoints - 读取关节角度
- record - 记录当前位姿
- runTeach - 执行示教动作
- clearTeach - 清空示教组
- saveFlash/loadFlash/deleteFlash - Flash操作
- clawOpen/clawClose - 夹爪控制
- savePreset1-4 - 保存预设
- preset1-4 - 运行预设
- emergency - 紧急停止
- setZero - 全关节置零

### 3.3 数据存储模块

系统使用ESP32的NVS Flash进行数据持久化存储：
- 示教数据：最多10个位姿，FNV-1a校验
- 预设数据：4组预设，每组最多10个位姿

### 3.4 动作序列控制模块

```cpp
struct Pose {
    int16_t joint[6];   // 6个关节角度
    uint16_t holdMs;     // 该位姿保持时间(毫秒)
};
```

## 四、硬件引脚定义 (config.h)

| 功能 | 引脚 |
|------|------|
| CAN_TX | GPIO26 |
| CAN_RX | GPIO27 |
| CAN_EN | GPIO23 |
| RS485_EN | GPIO17 |
| 5V_EN | GPIO16 |

## 五、注意事项

1. 急停操作：发生异常时立即点击紧急停止，系统会切断所有电机输出
2. CAN连接：确保CAN总线连接正常，否则关节读取和动作执行会失败
3. Flash存储：示教数据存储在ESP32的NVS分区，掉电不丢失
4. 初始零点：使用全关节置零功能前，请确保机械臂处于物理零点位置
5. 动作执行��执行动作时不要触碰机械臂，以免造成意外损伤

---

More documentation: https://github.com/raybotsmartworks/RB-A-420-3