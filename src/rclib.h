#ifndef __RCLIB_H__
#define __RCLIB_H__
#include <Arduino.h>
#include "driver/twai.h"

#include "config.h" //引脚配置
#include <HardwareSerial.h>
#include <SPI.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <WiFi.h>

extern int num1;
extern int num2;
extern int Speed;
extern int Position;
extern int shijian;
extern int panduan;
extern int panduan1;
extern int yanshi;
extern int ii, j;
extern int c[99];
extern float a1, a2, a3, a4, a5, a6, a7, a8, p, q, sg;
extern float b1[99], b2[99], b3[99], b4[99], b5[99], b6[99], b7[99], b8[99];
extern double Ang1, Ang2, ang1, ang2;
extern int txid;
extern int n, i;
extern double x1, yy1, x2, y2, r1, r2, L1, L2, X[99], Y[99], theta, theta1, theta2, theta3, phi1, phi2, ang1, ang2, v1, v2, p1, p2;
extern double x, y, r, phi, Ang1, Ang2, XX, YY, xstart, ystart, l, w, startx, starty, rr;
extern int txid, sid;
#define LILYGO

#ifdef LILYGO
#define TX_PIN 27
#define RX_PIN 26
#else
#define TX_PIN 17
#define RX_PIN 18
#endif

bool rc_init();

bool can_is_idle();
void can_begin_request(uint8_t motor_id, uint8_t cmd, uint32_t rx_id);
bool can_poll_response(float* out_result);

void run(int, int, int); // 运行
void stop(int);          // 停止
int read_rd(int, int);   // 读取位置
void set_zero(int);      // 设置零点
void re_set(int);        // 复位

float read_PID(int, int);               // 读取PID参数
void write_PID_RAM(int, int, float);    // 写入PID参数到RAM
void write_PID_ROM(int, int, float);    // 写入PID参数到ROM
float read_a(int);                      // 读取加速度
void write_a(int);                      // 写入加减速度到ROM和RAM
float read_Mc_encoder_p(int);           // 读取多圈编码器位置
float read_Mc_encoder_op(int);          // 读取多圈编码器原始位置
float read_Mc_encoder_zerobd(int);      // 读取多圈编码器零偏数据
void Write_Mc_encoder_ztoROM(int);      // 写入编码器多圈值到ROM作为零点
void Write_Mc_encoder_ztoRAM(int);      // 写入编码器当前多圈位置到ROM作为零点
float read_Sc_encoder_p(int);           // 读取单圈编码器
float read_Mc_angle(int);          // 读取多圈角度
float read_Sc_angle(int, int);          // 读取单圈角度
float read_motor_status_1(int, int);    // 读取电机状态1和错误标志命令
float read_motor_status_2(int, int);    // 读取电机状态2命令
float read_motor_status_3(int, int);    // 读取电机状态3命令
void shutdown(int);                     // 电机关闭命令
void Torque_control(int, float);               // 转矩闭环控制命令
void Speed_control(int, int);                // 速度闭环控制命令
void Absolute_position_control(int, int, int);    // 绝对位置闭环控制命令
void Single_position_control(int, int, int);      // 单圈位置控制命令
void Incremental_position_control(int, int, int); // 增量位置闭环控制命令
void reset(int);                        // 系统复位指令

#endif