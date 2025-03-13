#ifndef __ENCODER_H__
#define __ENCODER_H__
    struct EncoderData
    {
        unsigned char direction; // 编码器旋转方向
        float position;          // 编码器位置
        float speed;             // 编码器速度
    };
    
    void Encoder_Init(void);
    void Encoder_InterruptEnable(unsigned char intType);
    void Encoder_InterruptDisable(unsigned char intType);
    void Encoder_Update(void);
    int Encoder_Read(unsigned char channel);
    void Encoder_Clear(unsigned char channel);
    void Encoder_DetectZero(void);
#endif