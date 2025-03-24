#include <stdio.h>
#include <AI8051U.H>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "GPS_uart.h"
#include "GPS.h"

unsigned char gps_receive_buffer[64];
unsigned char set_rate_10hz[] = {0xF1, 0xD9, 0x06, 0x42, 0x14, 0x00, 0x00, 0x0A, 0x05,
                                 0x00, 0x64, 0x00, 0x00, 0x00, 0x60, 0xEA, 0x00, 0x00, 0xD0, 0x07, 0x00, 0x00,
                                 0xC8, 0x00, 0x00, 0x00, 0xB8, 0xED};

unsigned char cmd_gga[] = {0xF1, 0xD9, 0x06, 0x01, 0x03, 0x00, 0xF0, 0x00, 0x00, 0xFA, 0x0F};
unsigned char cmd_gll[] = {0xF1, 0xD9, 0x06, 0x01, 0x03, 0x00, 0xF0, 0x01, 0x00, 0xFB, 0x11};
unsigned char cmd_gsa[] = {0xF1, 0xD9, 0x06, 0x01, 0x03, 0x00, 0xF0, 0x02, 0x00, 0xFC, 0x13};
unsigned char cmd_gsv[] = {0xF1, 0xD9, 0x06, 0x01, 0x03, 0x00, 0xF0, 0x04, 0x00, 0xFE, 0x17};
unsigned char cmd_vtg[] = {0xF1, 0xD9, 0x06, 0x01, 0x03, 0x00, 0xF0, 0x06, 0x00, 0x00, 0x1B};
unsigned char cmd_zda[] = {0xF1, 0xD9, 0x06, 0x01, 0x03, 0x00, 0xF0, 0x07, 0x00, 0x01, 0x1D};
unsigned char cmd_gst[] = {0xF1, 0xD9, 0x06, 0x01, 0x03, 0x00, 0xF0, 0x20, 0x00, 0x1A, 0x4F};

RMC_Data rmc_data;
NaturePosition naturePosition;

// 将NMEA格式的经纬度转换为标准的十进制度
double nmea_to_decimal(double val)
{
    int degrees;
    double minutes;
    degrees = (int)(val / 100);
    minutes = val - degrees * 100;
    return degrees + minutes / 60.0;
}

// 解析RMC语句
void parse_rmc(char *sentence, RMC_Data *rmc_data)
{
    char buffer[128];
    char *field[15]; // 用于存储分割后的字段
    int field_count = 0;
    int i = 0;
    char *ptr;

    // 初始化数据
    for (i = 0; i < sizeof(RMC_Data); i++)
        ((char *)rmc_data)[i] = 0;
    rmc_data->valid = 0;

    // 复制语句以便进行处理
    i = 0;
    while (sentence[i] != 0 && i < 127)
    {
        buffer[i] = sentence[i];
        i++;
    }
    buffer[i] = 0; // 确保字符串结束

    // 手动分割字符串
    ptr = buffer;
    field[0] = ptr; // 第一个字段
    field_count = 1;

    for (i = 0; buffer[i] != 0 && field_count < 15; i++)
    {
        if (buffer[i] == ',' || buffer[i] == '*')
        {
            buffer[i] = 0;                         // 将分隔符替换为字符串结束符
            field[field_count++] = &buffer[i + 1]; // 记录下一个字段的起始位置
        }
    }

    // 解析各字段
    if (field_count > 1)
    {
        // UTC时间 (字段1)
        if (strlen(field[1]) >= 6)
        {
            rmc_data->hour = (field[1][0] - '0') * 10 + (field[1][1] - '0');
            rmc_data->minute = (field[1][2] - '0') * 10 + (field[1][3] - '0');
            rmc_data->second = atof(&field[1][4]);
        }
    }

    if (field_count > 2)
    {
        // 定位状态 (字段2)
        rmc_data->status = field[2][0];
    }

    if (field_count > 3)
    {
        // 纬度 (字段3)
        if (strlen(field[3]) > 0)
        {
            rmc_data->latitude = nmea_to_decimal(atof(field[3]));
        }
    }

    if (field_count > 4)
    {
        // 南北半球 (字段4)
        rmc_data->ns = field[4][0];
        if (rmc_data->ns == 'S')
        {
            rmc_data->latitude = -rmc_data->latitude;
        }
    }

    if (field_count > 5)
    {
        // 经度 (字段5)
        if (strlen(field[5]) > 0)
        {
            rmc_data->longitude = nmea_to_decimal(atof(field[5]));
        }
    }

    if (field_count > 6)
    {
        // 东西半球 (字段6)
        rmc_data->ew = field[6][0];
        if (rmc_data->ew == 'W')
        {
            rmc_data->longitude = -rmc_data->longitude;
        }
    }

    if (field_count > 7)
    {
        // 速度（节） (字段7)
        rmc_data->speed = atof(field[7]);
    }

    if (field_count > 8)
    {
        // 地面航向角 (字段8)
        rmc_data->course = atof(field[8]);
    }

    if (field_count > 9)
    {
        // 日期 (字段9)
        if (strlen(field[9]) >= 6)
        {
            rmc_data->day = (field[9][0] - '0') * 10 + (field[9][1] - '0');
            rmc_data->month = (field[9][2] - '0') * 10 + (field[9][3] - '0');
            rmc_data->year = 2000 + (field[9][4] - '0') * 10 + (field[9][5] - '0');
        }
    }

    if (field_count > 10)
    {
        // 磁偏角 (字段10)
        rmc_data->mag_var = atof(field[10]);
    }

    if (field_count > 11)
    {
        // 磁偏角方向 (字段11)
        rmc_data->mag_dir = field[11][0];
    }

    if (field_count > 12)
    {
        // 模式指示 (字段12)
        rmc_data->mode = field[12][0];
    }

    // 检查是否有效
    rmc_data->valid = (rmc_data->status == 'A');
}

void GPS_Calculate(NaturePosition *naturePosition, RMC_Data *rmc_data)
{
    // 计算当前位置
    naturePosition->x = rmc_data->latitude - naturePosition->offsetX;
    naturePosition->y = rmc_data->longitude - naturePosition->offsetY;
}

// 修改函数定义，添加参数类型和输出参数
void GPS_Message_Inputer(char *message, RMC_Data *rmc_data , NaturePosition *naturePosition)
{
    // 直接调用parse_rmc处理消息并更新传入的rmc_data
    parse_rmc(message, rmc_data);
    GPS_Calculate(naturePosition, rmc_data);
}

void UART_SendCommand(unsigned char *cmd, unsigned char length)
{
    unsigned char i;
    for(i = 0; i < length; i++)
    {
        GPS_UART_SendByte(cmd[i]);
    }
}

void Init_GPS_Setting()
{
    UART_SendCommand(set_rate_10hz, sizeof(set_rate_10hz));
    UART_SendCommand(cmd_gga, sizeof(cmd_gga));
    UART_SendCommand(cmd_gll, sizeof(cmd_gll));
    UART_SendCommand(cmd_gsa, sizeof(cmd_gsa));
    UART_SendCommand(cmd_gsv, sizeof(cmd_gsv));
    UART_SendCommand(cmd_vtg, sizeof(cmd_vtg));
    UART_SendCommand(cmd_zda, sizeof(cmd_zda));
    UART_SendCommand(cmd_gst, sizeof(cmd_gst));
}

// 修改Init_GPS函数，使用新的命令发送函数
void Init_GPS(NaturePosition *naturePosition, RMC_Data *rmc_data)
{
    // 初始化偏移量
    naturePosition->offsetX = rmc_data->latitude;
    naturePosition->offsetY = rmc_data->longitude;
    naturePosition->x = 0;
    naturePosition->y = 0;

    // 初始化GPS设置
    Init_GPS_Setting();
    GPS_UART_SendStr("GPS初始化成功!\0");
}

void GPS_Message_Updater()
{
    if (GPS_UART_Available())
    {
        unsigned char len = GPS_UART_Read(gps_receive_buffer, 32);
        if (len > 0)
        {
            GPS_Message_Inputer(gps_receive_buffer, &rmc_data, &naturePosition);
            // UART_SendStr("GPS数据更新成功!\0");
        }
    }
    else
    {
        GPS_UART_SendByte('N'); // 无数据
    }
}

// 算法部分写完辣！！！！！！！ 10/3/2025 16:47
// 之后要测试一下这个算法

// // 测试部分
// // 打印RMC数据
// void print_rmc_data(const RMC_Data *data)
// {
//     printf("===== RMC数据解析结果 =====\n");
//     printf("时间: %02d:%02d:%05.2f UTC\n", data->hour, data->minute, data->second);
//     printf("日期: %04d-%02d-%02d\n", data->year, data->month, data->day);
//     printf("定位状态: %c (%s)\n", data->status, data->status == 'A' ? "有效" : "无效");
//     printf("纬度: %.6f° %c\n", fabs(data->latitude), data->ns);
//     printf("经度: %.6f° %c\n", fabs(data->longitude), data->ew);
//     printf("速度: %.2f节 (%.2f公里/小时)\n", data->speed, data->speed * 1.852);
//     printf("航向角: %.1f°\n", data->course);
//     printf("磁偏角: %.1f° %c\n", data->mag_var, data->mag_dir);
//     printf("模式指示: %c\n", data->mode);
//     printf("==========================\n");
// }

// void test_rmc_parser(void)
// {
//     const char *test_sentence = "$GNRMC,153630.000,A,2754.15158,N,11255.09901,E,0.627,336.31,220325,,,A*4E";
//     RMC_Data rmc_data;

//     parse_rmc(test_sentence, &rmc_data);
//     print_rmc_data(&rmc_data);
// }

// int main()
// {
//     RMC_Data rmc_data;
//     char message[128];

//     scanf("%s", message);
//     GPS_Message_Inputer(message, &rmc_data);  // 传递rmc_data的地址
//     print_rmc_data(&rmc_data);

//     scanf("%s", message);
//     GPS_Message_Inputer(message, &rmc_data);  // 传递rmc_data的地址
//     print_rmc_data(&rmc_data);
//     test_rmc_parser();
//     return 0;
// }