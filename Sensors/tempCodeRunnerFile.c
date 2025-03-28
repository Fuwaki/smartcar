改Init_GPS函数，使用新的命令发送函数
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