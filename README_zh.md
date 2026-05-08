# RB-A-420-3 机械臂控制系统

## 简介

基于ESP32的CAN总线机械臂控制系统，支持Web界面远程控制。

![平台](https://img.shields.io/badge/平台-ESP32-blue)
![框架](https://img.shields.io/badge/框架-PlatformIO-green)
![许可证](https://img.shields.io/badge/许可证-MIT-yellow)

## 硬件

- ESP32 DevKit 开发板
- RB-A-420-3 六轴机械臂（含夹爪）
- CAN 总线接口

## 软件

- PlatformIO 开发环境
- ESP Async WebServer
- Arduino 框架

## 功能特点

- CAN总线与关节电机通信
- Web界面远程控制
- 实时状态监控
- 示教与回放功能
- Flash存储预设动作
- 紧急停止按钮

## 快速开始

### 1. 编译与上传

```bash
# 编译
pio run

# 通过USB上传
pio run --target upload

#通过网络上传
pio run --target upload --upload-port 192.168.1.100
```

### 2. WiFi配置

编辑 `include/config.h` 文件：

```c
#define WIFI_SSID "你的WiFi名称"
#define WIFI_PASSWORD "你的WiFi密码"
```

### 3. 访问Web控制界面

在浏览器中输入ESP32的IP地址（AP模式下默认为 `http://192.168.4.1`）

## 引脚定义

| 功能 | GPIO |
|------|------|
| CAN_TX | GPIO26 |
| CAN_RX | GPIO27 |
| CAN_EN | GPIO23 |

## 项目结构

```
RB-A-420-3/
├── src/
│   ├── main.cpp       # 主程序
│   ├── rclib.cpp    # 机械臂控制库
│   ├── rclib.h      # 库头文件
│   └── web_pages.h   # 网页界面
├── include/
│   └── config.h     # 配置文件
├── platformio.ini   # 项目配置
├── README.md      # 英文说明
├── README_zh.md  # 中文说明
└── LICENSE      # MIT许可证
```

## API接口

| 端点 | 参数 | 说明 |
|------|------|------|
| `/` | GET | Web控制页面 |
| `/cmd` | `action=xxx` | 执行命令 |
| `/status` | GET | JSON状态 |

**支持的命令：**
- `readJoints` - 读取关节角度
- `record` - 记录当前位姿
- `runTeach` - 执行示教序列
- `clawOpen` / `clawClose` - 夹爪控制
- `preset1-4` - 执行预设动作
- `emergency` - 紧急停止

## 文档

- [English README](README.md)
- [中文说明](README_zh.md)
- [详细使用说明](RB-A-420-3机械臂使用及代码说明.md)

## 许可证

MIT 许可证 - 见 [LICENSE](LICENSE) 文件。

## 技术支持

如有问题请在GitHub上提交Issue。