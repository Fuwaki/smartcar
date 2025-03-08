#ifndef __GPS_H__
#define __GPS_H__
    struct GPS_Data_t;
    void GPS_Config();
    char GPS_Parse(char *gps_str);
    void NMEA_Split(char *str, char **fields, int max_fields);
    char NMEA_Checksum(const char *str, int len);
    double NMEA_Deg_To_Decimal(char *deg_str);
    void NMEA_Parse_RMC(char **fields);
    void NMEA_Parse_GGA(char **fields);
#endif