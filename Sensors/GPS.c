#include <AI8051U.H>
#include <string.h>
#include <stdlib.h>

typedef unsigned char uint8_t;
typedef unsigned int uint16_t;
/*
 * GPS踩点
 */
double Point_lat[], Point_lon[]; // 纬度,经度
int Flash_Point_Num = 0;         // 采样点数
int aim1, aim2, aim3, aim4, aim5;

// GPS数据结构
typedef struct
{
    double latitude;    // 纬度
    double longitude;   // 经度
    double altitude;    // 海拔高度
    double speed;       // 速度（节）
    double direction;   // 航向角
    uint8_t satellites; // 卫星数量
    uint8_t status;     // 定位状态 0=未定位，1=非差分定位，2=差分定位
    uint8_t valid;      // 数据有效性 0=无效，1=有效
    // 时间信息
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t day;
    uint8_t month;
    uint8_t year;
} GPS_Data_t;

GPS_Data_t gps_data;

// 分割NMEA语句的辅助函数
void NMEA_Split(char *str, char **fields, int max_fields)
{
    int i = 0;
    fields[i++] = str;

    while (*str && i < max_fields)
    {
        if (*str == ',')
        {
            *str = '\0';
            fields[i++] = str + 1;
        }
        str++;
    }
}

// 计算校验和
char NMEA_Checksum(const char *str, int len)
{
    uint8_t checksum = 0;

    while (len--)
    {
        checksum ^= *str++;
    }

    return checksum;
}

// 将NMEA格式的经纬度转换为度
double NMEA_Deg_To_Decimal(char *deg_str)
{
    double degree;
    double minute;
    char *dot;

    if (!deg_str || !*deg_str)
        return 0.0;

    degree = atof(deg_str) / 100;
    int int_degree = (int)degree;
    minute = degree - int_degree;

    return int_degree + minute * 100 / 60;
}

// 解析GPRMC语句
void NMEA_Parse_RMC(char **fields)
{
    // 检查定位状态
    if (fields[2][0] == 'A')
    {
        gps_data.valid = 1;

        // 解析纬度
        if (fields[3] && *fields[3])
        {
            gps_data.latitude = NMEA_Deg_To_Decimal(fields[3]);
            // 南北半球
            if (fields[4][0] == 'S')
                gps_data.latitude = -gps_data.latitude;
        }

        // 解析经度
        if (fields[5] && *fields[5])
        {
            gps_data.longitude = NMEA_Deg_To_Decimal(fields[5]);
            // 东西半球
            if (fields[6][0] == 'W')
                gps_data.longitude = -gps_data.longitude;
        }

        // 解析速度（节）
        if (fields[7] && *fields[7])
            gps_data.speed = atof(fields[7]);

        // 解析航向角
        if (fields[8] && *fields[8])
            gps_data.direction = atof(fields[8]);

        // 解析日期
        if (fields[9] && strlen(fields[9]) >= 6)
        {
            gps_data.day = (fields[9][0] - '0') * 10 + (fields[9][1] - '0');
            gps_data.month = (fields[9][2] - '0') * 10 + (fields[9][3] - '0');
            gps_data.year = (fields[9][4] - '0') * 10 + (fields[9][5] - '0');
        }
    }
    else
    {
        gps_data.valid = 0;
    }

    // 解析时间
    if (fields[1] && strlen(fields[1]) >= 6)
    {
        gps_data.hour = (fields[1][0] - '0') * 10 + (fields[1][1] - '0');
        gps_data.minute = (fields[1][2] - '0') * 10 + (fields[1][3] - '0');
        gps_data.second = (fields[1][4] - '0') * 10 + (fields[1][5] - '0');
    }
}

// 解析GPGGA语句
void NMEA_Parse_GGA(char **fields)
{
    // 解析定位状态
    if (fields[6] && *fields[6])
    {
        gps_data.status = fields[6][0] - '0';
        if (gps_data.status > 0)
            gps_data.valid = 1;
        else
            gps_data.valid = 0;
    }

    // 解析卫星数量
    if (fields[7] && *fields[7])
        gps_data.satellites = atoi(fields[7]);

    // 解析海拔高度
    if (fields[9] && *fields[9])
        gps_data.altitude = atof(fields[9]);

    // 解析纬度
    if (fields[2] && *fields[2])
    {
        gps_data.latitude = NMEA_Deg_To_Decimal(fields[2]);
        // 南北半球
        if (fields[3][0] == 'S')
            gps_data.latitude = -gps_data.latitude;
    }

    // 解析经度
    if (fields[4] && *fields[4])
    {
        gps_data.longitude = NMEA_Deg_To_Decimal(fields[4]);
        // 东西半球
        if (fields[5][0] == 'W')
            gps_data.longitude = -gps_data.longitude;
    }

    // 解析时间
    if (fields[1] && strlen(fields[1]) >= 6)
    {
        gps_data.hour = (fields[1][0] - '0') * 10 + (fields[1][1] - '0');
        gps_data.minute = (fields[1][2] - '0') * 10 + (fields[1][3] - '0');
        gps_data.second = (fields[1][4] - '0') * 10 + (fields[1][5] - '0');
    }
}

// GPS信号解析主函数
char GPS_Parse(char *gps_str)
{
    char *fields[20]; // 用于存储分割后的字段
    char *checksum_field;
    uint8_t calculated_checksum;
    int str_len;

    // 检查有效性
    if (!gps_str || gps_str[0] != '$')
        return 0;

    // 查找校验和位置
    checksum_field = strchr(gps_str, '*');
    if (!checksum_field) // 没有校验和
        return 0;

    str_len = checksum_field - gps_str - 1;

    // 计算校验和
    calculated_checksum = NMEA_Checksum(gps_str + 1, str_len);

    // 验证校验和
    int checksum_read = 0;
    if (checksum_field[1] >= '0' && checksum_field[1] <= '9')
        checksum_read = (checksum_field[1] - '0') << 4;
    else if (checksum_field[1] >= 'A' && checksum_field[1] <= 'F')
        checksum_read = (checksum_field[1] - 'A' + 10) << 4;

    if (checksum_field[2] >= '0' && checksum_field[2] <= '9')
        checksum_read |= (checksum_field[2] - '0');
    else if (checksum_field[2] >= 'A' && checksum_field[2] <= 'F')
        checksum_read |= (checksum_field[2] - 'A' + 10);

    if (calculated_checksum != checksum_read)
        return 0;

    // 分割语句
    *checksum_field = '\0'; // 替换*为\0以便分割
    NMEA_Split(gps_str, fields, 20);

    // 判断语句类型并解析
    if (strstr(fields[0], "RMC"))
    {
        NMEA_Parse_RMC(fields);
        return 1;
    }
    else if (strstr(fields[0], "GGA"))
    {
        NMEA_Parse_GGA(fields);
        return 2;
    }

    return 0;
}

void GPS_Config()
{
    // 初始化GPS数据结构
    memset(&gps_data, 0, sizeof(GPS_Data_t));

    // 这里可添加GPS模块的初始化配置
    // 例如：设置波特率、输出频率、开启/关闭特定NMEA语句等
}
