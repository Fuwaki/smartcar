#ifndef __GPS_H__
#define __GPS_H__
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

    typedef struct
    {
        double offsetX;
        double offsetY;
        double x;
        double y;
    } NaturePosition;

    extern RMC_Data rmc_data;
    extern NaturePosition naturePosition;
    

    double nmea_to_decimal(double val);
    void parse_rmc(char *sentence, RMC_Data *rmc_data);
    void GPS_Message_Inputer(char *message, RMC_Data *rmc_data, NaturePosition *naturePosition);
    void GPS_Calculate(NaturePosition *naturePosition, RMC_Data *rmc_data);
    void Init_GPS_Setting(void);
    unsigned char Init_GPS_Offset(NaturePosition *naturePosition, RMC_Data *rmc_data);
    void GPS_SendCommand(unsigned char *cmd, unsigned char length);
    void GPS_Message_Updater(void);
#endif