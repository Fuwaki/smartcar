#include "Oled.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// 显示缓冲区
unsigned char OLED_Buffer[OLED_WIDTH * OLED_HEIGHT / 8];

// 当前光标位置
unsigned char OLED_CursorX = 0;
unsigned char OLED_CursorY = 0;
double timestamp = 0; //单位是10ms
unsigned char VALUE_FOR_SWITCH = 10; //开关值

struct OledState
{
    unsigned char States;
    unsigned char LastStates;
};
struct OledState OledState = {0, 0};


// sbit SW1 = P5 ^ 2;
// sbit SW2 = P0 ^ 4;
// sbit SW3 = P0 ^ 3;
// sbit SW4 = P0 ^ 2;
// sbit SW5 = P4 ^ 6;
// sbit SW6 = P4 ^ 5;

// 字体数据 - 6x8像素 ASCII字符集
static const unsigned char Font6x8[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Space (32)
    0x00, 0x00, 0x5F, 0x00, 0x00, 0x00, // !
    0x00, 0x07, 0x00, 0x07, 0x00, 0x00, // "
    0x14, 0x7F, 0x14, 0x7F, 0x14, 0x00, // #
    0x24, 0x2A, 0x7F, 0x2A, 0x12, 0x00, // $
    0x23, 0x13, 0x08, 0x64, 0x62, 0x00, // %
    0x36, 0x49, 0x56, 0x20, 0x50, 0x00, // &
    0x00, 0x08, 0x07, 0x03, 0x00, 0x00, // '
    0x00, 0x1C, 0x22, 0x41, 0x00, 0x00, // (
    0x00, 0x41, 0x22, 0x1C, 0x00, 0x00, // )
    0x2A, 0x1C, 0x7F, 0x1C, 0x2A, 0x00, // *
    0x08, 0x08, 0x3E, 0x08, 0x08, 0x00, // +
    0x00, 0x80, 0x70, 0x30, 0x00, 0x00, // ,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x00, // -
    0x00, 0x00, 0x60, 0x60, 0x00, 0x00, // .
    0x20, 0x10, 0x08, 0x04, 0x02, 0x00, // /
    0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00, // 0
    0x00, 0x42, 0x7F, 0x40, 0x00, 0x00, // 1
    0x72, 0x49, 0x49, 0x49, 0x46, 0x00, // 2
    0x21, 0x41, 0x49, 0x4D, 0x33, 0x00, // 3
    0x18, 0x14, 0x12, 0x7F, 0x10, 0x00, // 4
    0x27, 0x45, 0x45, 0x45, 0x39, 0x00, // 5
    0x3C, 0x4A, 0x49, 0x49, 0x31, 0x00, // 6
    0x41, 0x21, 0x11, 0x09, 0x07, 0x00, // 7
    0x36, 0x49, 0x49, 0x49, 0x36, 0x00, // 8
    0x46, 0x49, 0x49, 0x29, 0x1E, 0x00, // 9
    0x00, 0x00, 0x14, 0x00, 0x00, 0x00, // :
    0x00, 0x40, 0x34, 0x00, 0x00, 0x00, // ;
    0x00, 0x08, 0x14, 0x22, 0x41, 0x00, // <
    0x14, 0x14, 0x14, 0x14, 0x14, 0x00, // =
    0x00, 0x41, 0x22, 0x14, 0x08, 0x00, // >
    0x02, 0x01, 0x59, 0x09, 0x06, 0x00, // ?
    0x3E, 0x41, 0x5D, 0x59, 0x4E, 0x00, // @
    0x7C, 0x12, 0x11, 0x12, 0x7C, 0x00, // A
    0x7F, 0x49, 0x49, 0x49, 0x36, 0x00, // B
    0x3E, 0x41, 0x41, 0x41, 0x22, 0x00, // C
    0x7F, 0x41, 0x41, 0x41, 0x3E, 0x00, // D
    0x7F, 0x49, 0x49, 0x49, 0x41, 0x00, // E
    0x7F, 0x09, 0x09, 0x09, 0x01, 0x00, // F
    0x3E, 0x41, 0x41, 0x51, 0x73, 0x00, // G
    0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00, // H
    0x00, 0x41, 0x7F, 0x41, 0x00, 0x00, // I
    0x20, 0x40, 0x41, 0x3F, 0x01, 0x00, // J
    0x7F, 0x08, 0x14, 0x22, 0x41, 0x00, // K
    0x7F, 0x40, 0x40, 0x40, 0x40, 0x00, // L
    0x7F, 0x02, 0x1C, 0x02, 0x7F, 0x00, // M
    0x7F, 0x04, 0x08, 0x10, 0x7F, 0x00, // N
    0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00, // O
    0x7F, 0x09, 0x09, 0x09, 0x06, 0x00, // P
    0x3E, 0x41, 0x51, 0x21, 0x5E, 0x00, // Q
    0x7F, 0x09, 0x19, 0x29, 0x46, 0x00, // R
    0x26, 0x49, 0x49, 0x49, 0x32, 0x00, // S
    0x03, 0x01, 0x7F, 0x01, 0x03, 0x00, // T
    0x3F, 0x40, 0x40, 0x40, 0x3F, 0x00, // U
    0x1F, 0x20, 0x40, 0x20, 0x1F, 0x00, // V
    0x3F, 0x40, 0x38, 0x40, 0x3F, 0x00, // W
    0x63, 0x14, 0x08, 0x14, 0x63, 0x00, // X
    0x03, 0x04, 0x78, 0x04, 0x03, 0x00, // Y
    0x61, 0x59, 0x49, 0x4D, 0x43, 0x00, // Z
};

void Delay50ms(void)	//@35.000MHz
{
	unsigned long edata i;

	_nop_();
	_nop_();
	i = 437498UL;
	while (i) i--;
}


// I2C发送数据
void OLED_I2C_Write(unsigned char reg, unsigned char Senddata)
{
    unsigned char buffer[2];
    buffer[0] = reg;
    buffer[1] = Senddata;
    I2C_WriteToOLED(OLED_ADDRESS, buffer, 2);
}

// 发送命令
void OLED_WriteCommand(unsigned char command)
{
    OLED_I2C_Write(OLED_CMD, command);
}

// 发送数据
void OLED_WriteData(unsigned char Senddata)
{
    OLED_I2C_Write(OLED_DATA, Senddata);
}

// OLED初始化
void OLED_Init(void)
{
    Delay50ms(); // 等待OLED稳定

    // 初始化命令序列
    OLED_WriteCommand(SSD1306_DISPLAY_OFF);              // 关闭显示
    OLED_WriteCommand(SSD1306_SET_DISPLAY_CLK_DIV);      // 设置时钟分频
    OLED_WriteCommand(0x80);                             // 推荐值
    OLED_WriteCommand(SSD1306_SET_MULTIPLEX_RATIO);      // 设置复用比
    OLED_WriteCommand(0x3F);                             // 64行
    OLED_WriteCommand(SSD1306_SET_DISPLAY_OFFSET);       // 设置显示偏移
    OLED_WriteCommand(0x00);                             // 无偏移
    OLED_WriteCommand(SSD1306_SET_START_LINE | 0x00);    // 设置起始行
    OLED_WriteCommand(SSD1306_CHARGE_PUMP);              // 设置电荷泵
    OLED_WriteCommand(0x14);                             // 启用电荷泵
    OLED_WriteCommand(SSD1306_SET_MEMORY_ADDR_MODE);     // 内存寻址模式
    OLED_WriteCommand(0x00);                             // 水平寻址模式
    OLED_WriteCommand(SSD1306_SET_SEGMENT_REMAP | 0x01); // 列地址127映射到SEG0
    OLED_WriteCommand(SSD1306_COM_SCAN_REMAP);           // 颠倒COM扫描方向
    OLED_WriteCommand(SSD1306_SET_COM_PINS);             // 设置COM引脚配置
    OLED_WriteCommand(0x12);                             // 适用于64行
    OLED_WriteCommand(SSD1306_SET_CONTRAST);             // 设置对比度
    OLED_WriteCommand(0xCF);                             // 对比度值
    OLED_WriteCommand(SSD1306_SET_PRECHARGE);            // 设置预充电周期
    OLED_WriteCommand(0xF1);                             // 推荐值
    OLED_WriteCommand(SSD1306_SET_VCOM_DETECT);          // 设置VCOM检测级别
    OLED_WriteCommand(0x40);                             // 推荐值
    OLED_WriteCommand(SSD1306_NORMAL_DISPLAY);           // 正常显示（非反色）
    OLED_WriteCommand(SSD1306_DISPLAY_ON);               // 开启显示

    // 清空显示
    OLED_Clear();
}

// 清空屏幕
void OLED_Clear(void)
{
    memset(OLED_Buffer, 0, sizeof(OLED_Buffer));
    OLED_UpdateScreen();
    OLED_CursorX = 0;
    OLED_CursorY = 0;
}

// 更新屏幕
void OLED_UpdateScreen(void)
{
    unsigned char i;

    // 设置地址模式
    OLED_WriteCommand(SSD1306_SET_MEMORY_ADDR_MODE);
    OLED_WriteCommand(0x00); // 水平寻址模式

    // 设置列地址范围
    OLED_WriteCommand(SSD1306_SET_COLUMN_ADDR);
    OLED_WriteCommand(0);   // 起始列
    OLED_WriteCommand(127); // 结束列

    // 设置页地址范围
    OLED_WriteCommand(SSD1306_SET_PAGE_ADDR);
    OLED_WriteCommand(0); // 起始页
    OLED_WriteCommand(7); // 结束页

    // 使用分段方式发送显示数据，避免一次发送数据过多
    for (i = 0; i < 8; i++)
    {
        unsigned char buffer[129]; // 多一个字节用于OLED_DATA命令字
        buffer[0] = OLED_DATA;
        memcpy(&buffer[1], &OLED_Buffer[i * 128], 128);
        I2C_WriteToOLED(OLED_ADDRESS, buffer, 129);
    }
}

// 设置像素
void OLED_DrawPixel(unsigned char x, unsigned char y, unsigned char color)
{
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT)
    {
        return;
    }

    if (color == 1)
    {
        OLED_Buffer[x + (y / 8) * OLED_WIDTH] |= 1 << (y % 8);
    }
    else
    {
        OLED_Buffer[x + (y / 8) * OLED_WIDTH] &= ~(1 << (y % 8));
    }
}

// 设置光标位置
void OLED_SetCursor(unsigned char x, unsigned char y)
{
    OLED_CursorX = x;
    OLED_CursorY = y;
}

// 写字符
void OLED_WriteChar(char ch)
{
    unsigned char i;
    unsigned char j;
    if (ch < 32 || ch > 126) //目前只支持ASCII字符32到126
    {
        return;
    }

    if (OLED_CursorX + 6 > OLED_WIDTH)
    {
        OLED_CursorX = 0;
        OLED_CursorY += 8; // 换行
    }

    if (OLED_CursorY + 8 > OLED_HEIGHT) // 超出屏幕高度
    {
        OLED_CursorY = 0; // 重置光标位置
    }

    for (i = 0; i < 6; i++)
    {
        unsigned char line = Font6x8[(ch - 32) * 6 + i];
        for (j = 0; j < 8; j++)
        {
            if (line & (1 << j))
            {
                OLED_DrawPixel(OLED_CursorX + i, OLED_CursorY + j, 1);
            }
            else
            {
                OLED_DrawPixel(OLED_CursorX + i, OLED_CursorY + j, 0);
            }
        }
    }

    OLED_CursorX += 6;
}

// 写字符串
void OLED_WriteString(const char *str)
{
    while (*str)
    {
        OLED_WriteChar(*(str++));
    }
}

// 绘制水平线
void OLED_DrawHLine(unsigned char x, unsigned char y, unsigned char length, unsigned char color)
{
    unsigned char i;
    for (i = 0; i < length; i++)
    {
        OLED_DrawPixel(x + i, y, color);
    }
}

// 绘制垂直线
void OLED_DrawVLine(unsigned char x, unsigned char y, unsigned char length, unsigned char color)
{
    unsigned char i;
    for (i = 0; i < length; i++)
    {
        OLED_DrawPixel(x, y + i, color);
    }
}

// 绘制矩形
void OLED_DrawRectangle(unsigned char x, unsigned char y, unsigned char width, unsigned char height,
                        unsigned char color)
{
    OLED_DrawHLine(x, y, width, color);
    OLED_DrawHLine(x, y + height - 1, width, color);
    OLED_DrawVLine(x, y, height, color);
    OLED_DrawVLine(x + width - 1, y, height, color);
}

// 填充矩形
void OLED_FillRectangle(unsigned char x, unsigned char y, unsigned char width, unsigned char height,
                        unsigned char color)
{
    unsigned char i;
    for (i = 0; i < height; i++)
    {
        OLED_DrawHLine(x, y + i, width, color);
    }
}

// 绘制圆
void OLED_DrawCircle(unsigned char x0, unsigned char y0, unsigned char radius, unsigned char color)
{
    signed int x = radius;
    signed int y = 0;
    signed int err = 0;

    while (x >= y)
    {
        OLED_DrawPixel(x0 + x, y0 + y, color);
        OLED_DrawPixel(x0 + y, y0 + x, color);
        OLED_DrawPixel(x0 - y, y0 + x, color);
        OLED_DrawPixel(x0 - x, y0 + y, color);
        OLED_DrawPixel(x0 - x, y0 - y, color);
        OLED_DrawPixel(x0 - y, y0 - x, color);
        OLED_DrawPixel(x0 + y, y0 - x, color);
        OLED_DrawPixel(x0 + x, y0 - y, color);

        y += 1;
        if (err <= 0)
        {
            err += 2 * y + 1;
        }
        if (err > 0)
        {
            x -= 1;
            err -= 2 * x + 1;
        }
    }
}

// 填充圆
void OLED_FillCircle(unsigned char x0, unsigned char y0, unsigned char radius, unsigned char color)
{
    signed int x = radius;
    signed int y = 0;
    signed int err = 0;

    while (x >= y)
    {
        OLED_DrawHLine(x0 - x, y0 + y, 2 * x, color);
        OLED_DrawHLine(x0 - y, y0 + x, 2 * y, color);
        OLED_DrawHLine(x0 - x, y0 - y, 2 * x, color);
        OLED_DrawHLine(x0 - y, y0 - x, 2 * y, color);

        y += 1;
        if (err <= 0)
        {
            err += 2 * y + 1;
        }
        if (err > 0)
        {
            x -= 1;
            err -= 2 * x + 1;
        }
    }
}

// 显示进度条
void OLED_DrawProgressBar(unsigned char x, unsigned char y, unsigned char width, unsigned char height,
                          unsigned char progress)
{
    unsigned char fill_width = (progress * width) / 100;

    // 绘制外框
    OLED_DrawRectangle(x, y, width, height, 1);

    // 填充进度
    if (fill_width > 0)
    {
        OLED_FillRectangle(x + 1, y + 1, fill_width - 1, height - 2, 1);
    }
}

// 屏幕反色显示
void OLED_InvertDisplay(unsigned char inverted)
{
    if (inverted)
    {
        OLED_WriteCommand(SSD1306_INVERSE_DISPLAY);
    }
    else
    {
        OLED_WriteCommand(SSD1306_NORMAL_DISPLAY);
    }
}

// 屏幕旋转180度
void OLED_Rotate180(unsigned char rotate)
{
    if (rotate)
    {
        OLED_WriteCommand(SSD1306_SET_SEGMENT_REMAP | 0x00); // 重置段重映射
        OLED_WriteCommand(SSD1306_COM_SCAN_NORMAL);          // 正常COM扫描
    }
    else
    {
        OLED_WriteCommand(SSD1306_SET_SEGMENT_REMAP | 0x01); // 设置段重映射
        OLED_WriteCommand(SSD1306_COM_SCAN_REMAP);           // 颠倒COM扫描方向
    }
}

// 休眠模式控制
void OLED_Sleep(unsigned char sleep)
{
    if (sleep)
    {
        OLED_WriteCommand(SSD1306_DISPLAY_OFF);
    }
    else
    {
        OLED_WriteCommand(SSD1306_DISPLAY_ON);
    }
}

// void SwitchUpdater()
// {
//     static unsigned char detectedTimes = 0; // 记录按键按下的次数
//     char buffer[64];    
//     // // OLED_Clear(); // 清空屏幕 确保刷新率足够快
//     // #pragma region  检测开关状态
//     // if (SW1 == 0 || SW2 == 0 || SW3 == 0 || SW4 == 0 || SW5 == 0 || SW6 == 0) //请不要当傻逼,同时按下多个开关
//     // {
//     //     detectedTimes++;
//     // }

//     // if (SW1 == 0 && detectedTimes > VALUE_FOR_SWITCH)
//     // {
//     //     // OLED_WriteString("SW1 Pressed");
//     //     OledState.States = 1; // 设置状态为1
//     // }
//     // else if (SW2 == 0 && detectedTimes > VALUE_FOR_SWITCH)
//     // {
//     //     OledState.States = 2; // 设置状态为2
//     // }
//     // else if (SW3 == 0 && detectedTimes > VALUE_FOR_SWITCH)
//     // {
//     //     OledState.States = 3; // 设置状态为3
//     // }
//     // else if (SW4 == 0 && detectedTimes > VALUE_FOR_SWITCH)
//     // {
//     //     OledState.States = 4; // 设置状态为4
//     // }
//     // else if (SW5 == 0 && detectedTimes > VALUE_FOR_SWITCH)
//     // {
//     //     OledState.States = 5; // 设置状态为5
//     // }
//     // else if (SW6 == 0 && detectedTimes > VALUE_FOR_SWITCH)
//     // {
//     //     OledState.States = 6; // 设置状态为6
//     // }
//     // else
//     // {
//     //     detectedTimes = 0;
//     // }
//     // #pragma endregion 


//     #pragma region  显示状态

//     switch (OledState.States)
//     {
//         case 1:
//             OLED_SetCursor(11, 45); // 设置光标位置取决你想放where
//             sprintf(buffer, "Count:%d Th:%d", detectedTimes, VALUE_FOR_SWITCH);//对啊用sprintf函数来格式化字符串
//             OLED_WriteString(buffer);
//             break;
//         case 2:
//             OLED_SetCursor(0 , 0);
//             // sprintf(buffer, "Sensor:%d Th:%d", sensor_data.IMU_Acc_X, sensor_data.IMU_Acc_Y);
//             OLED_WriteString(buffer);
//             break;
//         case 0:
//             OLED_WriteString("SW3 Pressed");
//             break;
//         case 4:
//             OLED_WriteString("SW4 Pressed");
//             break;
//         case 5:
//             OLED_WriteString("SW5 Pressed");
//             break;
//         case 6:
//             OLED_WriteString("SW6 Pressed");
//             break;
//         default:
//             break;
//     }

//     OLED_UpdateScreen(); // 更新屏幕显示
//     OledState.LastStates = OledState.States; // 更新上一个状态

//     #pragma endregion
// }


void Timer2_Init(void)		//100微秒@35.000MHz
{
	AUXR |= 0x04;			//定时器时钟1T模式
	T2L = 0x54;				//设置定时初始值
	T2H = 0xF2;				//设置定时初始值
	AUXR |= 0x10;			//定时器2开始计时
}

void Timer2_Isr(void) interrupt 12
{
    timestamp += 0.01; // 每次中断增加0.1ms

    // if (timestamp == floor(timestamp))

}
