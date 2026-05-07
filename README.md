# RB-A-420-3 机械臂控制

## 简介
基于ESP32的CAN总线机械臂控制系统

## 硬件
- ESP32 DevKit
- RB-A-420-3 机械臂
- CAN总线接口

## 软件
- PlatformIO
- ESP Async WebServer

## 功能
- CAN通信控制
- Web界面控制
- 实时反馈

## 使用方法

### 1. 编译上传
```bash
pio run --target upload
```

### 2. 连接WiFi
在 `include/config.h` 中配置WiFi名称和密码

### 3. 访问Web界面
在浏览器中输入ESP32的IP地址

## 引脚连接
| 功能 | GPIO |
|------|------|
| CAN_TX | GPIO5 |
| CAN_RX | GPIO4 |

## 文件结构
```
RB-A-420-3/
├── src/
│   ├── main.cpp       # 主程序
│   ├── rclib.cpp     # 机械臂控制库
│   ├── rclib.h       # 库头文件
│   └── web_pages.h   # 网页界面
├── include/
│   └── config.h      # 配置文件
├── platformio.ini    # 项目配置
└── README.md        # 说明文档
```

## 技术支持
如有问题请联系技术支持