#include <AI8051U.H>
#include <intrins.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "GPS_uart.h"
#include "GPS.h"
#include "uart.h"

unsigned char gps_receive_buffer[128];
unsigned char gps_buffer_index = 0;
unsigned char set_rate_10hz[] = {0xF1, 0xD9, 0x06, 0x42, 0x14, 0x00, 0x00, 0x0A, 0x05,
                                 0x00, 0x64, 0x00, 0x00, 0x00, 0x60, 0xEA, 0x00, 0x00, 0xD0, 0x07, 0x00, 0x00, 0xC8, 0x00, 0x00, 0x00, 0xB8, 0xED};

unsigned char cmd_gga[] = {0xF1, 0xD9, 0x06, 0x01, 0x03, 0x00, 0xF0, 0x00, 0x00, 0xFA, 0x0F};
unsigned char cmd_gll[] = {0xF1, 0xD9, 0x06, 0x01, 0x03, 0x00, 0xF0, 0x01, 0x00, 0xFB, 0x11};
unsigned char cmd_gsa[] = {0xF1, 0xD9, 0x06, 0x01, 0x03, 0x00, 0xF0, 0x02, 0x00, 0xFC, 0x13};
unsigned char cmd_gsv[] = {0xF1, 0xD9, 0x06, 0x01, 0x03, 0x00, 0xF0, 0x04, 0x00, 0xFE, 0x17};
unsigned char cmd_vtg[] = {0xF1, 0xD9, 0x06, 0x01, 0x03, 0x00, 0xF0, 0x06, 0x00, 0x00, 0x1B};
unsigned char cmd_zda[] = {0xF1, 0xD9, 0x06, 0x01, 0x03, 0x00, 0xF0, 0x07, 0x00, 0x01, 0x1D};
unsigned char cmd_gst[] = {0xF1, 0xD9, 0x06, 0x01, 0x03, 0x00, 0xF0, 0x20, 0x00, 0x1A, 0x4F};

RMC_Data rmc_data;
NaturePosition naturePosition;

float NATURE_POSITION_X = 0.0f; // 初始经度差
float NATURE_POSITION_Y = 0.0f; // 初始纬度差

void GPS_Delay(void)	//为初始化GPS模块提供延时
{
	unsigned long edata i;

	_nop_();
	_nop_();
	_nop_();
	i = 2998UL;
	while (i) i--;
}

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
        //memset

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
        // 速度(m/s) (字段7)
        rmc_data->speed = atof(field[7])*0.51444f; // 转换为米每秒
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

void gps_to_meters(RMC_Data *rmc_data, NaturePosition *naturePosition)
{
    double lat_diff, lon_diff, lat_rad;
    lat_rad = rmc_data->latitude * 3.141593 / 180.0; // 将纬度转换为弧度
    lat_diff = rmc_data->latitude - naturePosition->offsetY;
    lon_diff = rmc_data->longitude - naturePosition->offsetX;

    rmc_data -> latitude_decimal = lat_diff * 111120.0; // 纬度差转换为米
    rmc_data -> longitude_decimal = lon_diff * 111320.0 * cos(lat_rad); // 经度差转换为米
}

void GPS_Calculate(NaturePosition *naturePosition, RMC_Data *rmc_data)
{
    gps_to_meters(rmc_data, naturePosition);
    // 计算当前位置
    naturePosition->y = rmc_data->latitude_decimal;
    naturePosition->x = rmc_data->longitude_decimal;
}

// 修改函数定义，添加参数类型和输出参数
void GPS_Message_Inputer(char *message, RMC_Data *rmc_data , NaturePosition *naturePosition)
{
    // 直接调用parse_rmc处理消息并更新传入的rmc_data
    parse_rmc(message, rmc_data);
    GPS_Calculate(naturePosition, rmc_data);
}

void GPS_SendCommand(unsigned char *cmd, unsigned char length)
{
    unsigned char i;
    if (cmd == NULL || length == 0)
        return;
        
    for(i = 0; i < length; i++)
    {
        GPS_UART_SendByte(cmd[i]);
    }
}

void Init_GPS_Setting()
{
    naturePosition.x = 0;
    naturePosition.y = 0;
    naturePosition.offsetX = 0;
    naturePosition.offsetY = 0;
    
    rmc_data.hour = 0;
    rmc_data.minute = 0;
    rmc_data.second = 0.0;
    rmc_data.status = 'V';
    rmc_data.latitude = 0.0;
    rmc_data.ns = 'N';
    rmc_data.longitude = 0.0;
    rmc_data.ew = 'E';
    rmc_data.speed = 0.0;
    rmc_data.course = 0.0;
    rmc_data.day = 0;
    rmc_data.month = 0;
    rmc_data.year = 0;
    rmc_data.mag_var = 0.0;
    rmc_data.mag_dir = 'E';
    rmc_data.mode = 'V';
    rmc_data.valid = 0; // 初始化为无效状态

    GPS_SendCommand(set_rate_10hz, sizeof(set_rate_10hz));
    GPS_Delay();
    GPS_SendCommand(cmd_gga, sizeof(cmd_gga));
    GPS_Delay();
    GPS_SendCommand(cmd_gll, sizeof(cmd_gll));
    GPS_Delay();
    GPS_SendCommand(cmd_gsa, sizeof(cmd_gsa));
    GPS_Delay();
    GPS_SendCommand(cmd_gsv, sizeof(cmd_gsv));
    GPS_Delay();
    GPS_SendCommand(cmd_vtg, sizeof(cmd_vtg));
    GPS_Delay();
    GPS_SendCommand(cmd_zda, sizeof(cmd_zda));
    GPS_Delay();
    GPS_SendCommand(cmd_gst, sizeof(cmd_gst));
    GPS_Delay();
}

unsigned char Init_GPS_Offset(NaturePosition *naturePosition, RMC_Data *rmc_data)
{
    // 初始化偏移量
    if (rmc_data->valid == 0)
    {
        return 1; // 返回错误，数据无效
    }
    // 设置偏移量为当前GPS数据
    naturePosition->offsetY = rmc_data->latitude;
    naturePosition->offsetX = rmc_data->longitude;
    naturePosition->x = 0;
    naturePosition->y = 0;
    return 0; // 返回成功
}

void GPS_Message_Updater()
{
    if (GPS_UART_Available())
    {
        unsigned char byte;
        // 逐字节读取
        while (GPS_UART_Available())
        {
            byte = GPS_UART_ReadByte();
            
            // 添加到缓冲区
            if (byte == '\n' || gps_buffer_index >= sizeof(gps_receive_buffer)-1)
            {
                // 行结束或缓冲区满
                gps_receive_buffer[gps_buffer_index] = '\0';  // 添加终止符
                
                // 只处理$GNRMC或$GPRMC语句
                if (strncmp((char*)gps_receive_buffer, "$GNRMC", 6) == 0 || 
                    strncmp((char*)gps_receive_buffer, "$GPRMC", 6) == 0)
                {
                    GPS_Message_Inputer((char*)gps_receive_buffer, &rmc_data, &naturePosition);
                    // UART_SendStr(gps_receive_buffer);
                }
                
                // 清空缓冲区准备接收下一句
                gps_buffer_index = 0;
            }
            else if (byte != '\r')  // 忽略回车符
            {
                gps_receive_buffer[gps_buffer_index++] = byte;
            }
        }
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
//     const char *test_sentence[] = {"$GNRMC,140415.800,A,2754.17413,N,11255.02251,E,0.420,10.58,230325,,,A*70", "$GNRMC,140416.600,A,2754.17419,N,11255.02250,E,0.711,351.65,230325,,,A*4F","$GNRMC,140415.900,A,2754.17414,N,11255.02251,E,0.349,27.18,230325,,,A*7E", "$GNRMC,140416.000,A,2754.17415,N,11255.02252,E,0.215,171.24,230325,,,A*43"};
//     parse_rmc(test_sentence[1], &rmc_data);
//     print_rmc_data(&rmc_data);
// }

// int main()
// {
    // char message[128];

    // scanf("%s", message);
    // GPS_Message_Inputer(message, &rmc_data, &naturePosition);  // 传递rmc_data的地址
    // print_rmc_data(&rmc_data);

    // scanf("%s", message);
    // GPS_Message_Inputer(message, &rmc_data, &naturePosition);  // 传递rmc_data的地址
    // print_rmc_data(&rmc_data);
    // test_rmc_parser();
    // return 0;
// }

//byd GPSBUG 在4/18/2025被修复