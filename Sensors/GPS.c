#include <stdio.h>
#include <AI8051U.H>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "uart.h"

unsigned char receive_buffer[64];
// RMC数据结构体
typedef struct
{
    int hour;         // 时
    int minute;       // 分
    float second;     // 秒
    char status;      // 定位状态，A=有效，V=无效
    double latitude;  // 纬度(度)
    char ns;          // 纬度方向，N=北半球，S=南半球
    double longitude; // 经度(度)
    char ew;          // 经度方向，E=东经，W=西经
    float speed;      // 地面速度(节)
    float course;     // 地面航向角(度)
    int day;          // 日
    int month;        // 月
    int year;         // 年
    float mag_var;    // 磁偏角
    char mag_dir;     // 磁偏角方向，E=东，W=西
    char mode;        // 模式指示，A=自动，D=差分，E=估算，N=数据无效
    int valid;        // 数据是否有效
} RMC_Data;

struct NaturePosition
{
    double offsetX;
    double offsetY;
    double x;
    double y;
};

RMC_Data rmc_data;

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

// 修改函数定义，添加参数类型和输出参数
void GPS_Message_Inputer(char *message, RMC_Data *rmc_data)
{
    // 直接调用parse_rmc处理消息并更新传入的rmc_data
    parse_rmc(message, rmc_data);
}

void Init_offset(struct NaturePosition *naturePosition, RMC_Data *rmc_data)
{
    // 初始化偏移量
    naturePosition->offsetX = rmc_data->latitude;
    naturePosition->offsetY = rmc_data->longitude;
    naturePosition->x = 0;
    naturePosition->y = 0;
}

void GPS_Calculate(struct NaturePosition *naturePosition, RMC_Data *rmc_data)
{
    // 计算当前位置
    naturePosition->x = rmc_data->latitude - naturePosition->offsetX;
    naturePosition->y = rmc_data->longitude - naturePosition->offsetY;
}

void GPS_Message_Updater()
{
    if(UART_Available())
    {
        unsigned char len = UART_Read(receive_buffer, 32);
        if (len > 0)
        {
            GPS_Message_Inputer(receive_buffer, &rmc_data);
            UART_SendStr("GPS数据更新成功!\0");
        }
    }
    else
    {
        UART_SendByte('N'); // 无数据
    }
}

//算法部分写完辣！！！！！！！ 10/3/2025 16:47
// Ciallo～(∠・ω< )⌒★
// Ciallo～(∠・ω< )⌒★
// Ciallo～(∠・ω< )⌒★
// Ciallo～(∠・ω< )⌒★
// Ciallo～(∠・ω< )⌒★            
// Ciallo～(∠・ω< )⌒★
// Ciallo～(∠・ω< )⌒★
// Ciallo～(∠・ω< )⌒★
// Ciallo～(∠・ω< )⌒★
// Ciallo～(∠・ω< )⌒★


// 测试部分
// 打印RMC数据
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
//     const char *test_sentence = "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A\r\n";
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
//     return 0;
// }