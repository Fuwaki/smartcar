#ifndef __OLED_H__
#define __OLED_H__
#include <STC32G.H>
#include <intrins.h>
#include "i2c.h"
#include "stdlib.h"
#include "string.h"
#include "uart.h"
// SSD1306 OLED 显示屏配置
#define OLED_ADDRESS 0x3C // OLED的I2C 7位地址，常见为0x3C
#define OLED_WIDTH 128    // OLED宽度
#define OLED_HEIGHT 64    // OLED高度

// SSD1306 命令集
#define OLED_CMD 0x00  // 写命令
#define OLED_DATA 0x40 // 写数据

// 基本命令
#define SSD1306_DISPLAY_OFF 0xAE        // 关闭显示
#define SSD1306_DISPLAY_ON 0xAF         // 开启显示
#define SSD1306_NORMAL_DISPLAY 0xA6     // 正常显示
#define SSD1306_INVERSE_DISPLAY 0xA7    // 反色显示
#define SSD1306_SET_CONTRAST 0x81       // 设置对比度
#define SSD1306_SET_DISPLAY_OFFSET 0xD3 // 设置显示偏移
#define SSD1306_SET_COM_PINS 0xDA       // 设置COM引脚配置
#define SSD1306_SET_VCOM_DETECT 0xDB    // 设置VCOMH

// 寻址命令
#define SSD1306_SET_MEMORY_ADDR_MODE 0x20 // 设置内存寻址模式
#define SSD1306_SET_COLUMN_ADDR 0x21      // 设置列地址
#define SSD1306_SET_PAGE_ADDR 0x22        // 设置页地址

// 硬件配置命令
#define SSD1306_SET_START_LINE 0x40      // 设置显示开始行
#define SSD1306_SET_SEGMENT_REMAP 0xA0   // 设置段重映射
#define SSD1306_SET_MULTIPLEX_RATIO 0xA8 // 设置复用比
#define SSD1306_COM_SCAN_NORMAL 0xC0     // COM扫描方向正常
#define SSD1306_COM_SCAN_REMAP 0xC8      // COM扫描方向重映射

// 时序设置命令
#define SSD1306_SET_DISPLAY_CLK_DIV 0xD5 // 设置显示时钟分频
#define SSD1306_SET_PRECHARGE 0xD9       // 设置预充电周期
#define SSD1306_SET_COM_PINS 0xDA        // 设置COM引脚配置

// 电荷泵命令
#define SSD1306_CHARGE_PUMP 0x8D // 电荷泵设置
// OLED初始化
void OLED_Init(void);

// 清空屏幕
void OLED_Clear(void);

// 更新屏幕显示
void OLED_UpdateScreen(void);

// 设置像素点
void OLED_DrawPixel(unsigned char x, unsigned char y, unsigned char color);

// 设置光标位置
void OLED_SetCursor(unsigned char x, unsigned char y);

// 写字符
void OLED_WriteChar(char ch);

// 写字符串
void OLED_WriteString(const char* str);

// 绘制水平线
void OLED_DrawHLine(unsigned char x, unsigned char y, unsigned char length, unsigned char color);

// 绘制垂直线
void OLED_DrawVLine(unsigned char x, unsigned char y, unsigned char length, unsigned char color);

// 绘制矩形
void OLED_DrawRectangle(unsigned char x, unsigned char y, unsigned char width, unsigned char height, unsigned char color);

// 填充矩形
void OLED_FillRectangle(unsigned char x, unsigned char y, unsigned char width, unsigned char height, unsigned char color);

// 绘制圆
void OLED_DrawCircle(unsigned char x0, unsigned char y0, unsigned char radius, unsigned char color);

// 填充圆
void OLED_FillCircle(unsigned char x0, unsigned char y0, unsigned char radius, unsigned char color);

// 显示进度条
void OLED_DrawProgressBar(unsigned char x, unsigned char y, unsigned char width, unsigned char height, unsigned char progress);

// 屏幕反色显示
void OLED_InvertDisplay(unsigned char inverted);

// 屏幕旋转180度
void OLED_Rotate180(unsigned char rotate);

// 休眠模式控制
void OLED_Sleep(unsigned char sleep);

void Timer2_Init(void);

void SwitchUpdater(void);

extern double timestamp;

#endif /* __OLED_H__ */