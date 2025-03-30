#ifndef __CAMERA_CONTROLLER_H__
#define __CAMERA_CONTROLLER_H__

    // CCD参数定义
    #define CCD_LENGTH 128   // CCD像素点数量

    // 函数声明
    void CCD_Init(void);
    void CCD_CollectData(void);
    unsigned char CCD_CalculateThreshold(void);
    void CCD_BinarizeData(unsigned char useAutoThreshold);
    void CCD_SetThreshold(unsigned char threshold);
    unsigned char CCD_GetThreshold(void);
    unsigned char* CCD_GetBinaryData(void);
    unsigned char* CCD_GetRawData(void);
    void CCD_Process(unsigned char useAutoThreshold);
    void StartCCD(void);
#endif
