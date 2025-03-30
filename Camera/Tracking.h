#ifndef __TRACKING_H__
#define __TRACKING_H__

    // 巡线相关参数
    #define TRACK_CENTER       64    // CCD中心点位置 (128/2)
    #define TRACK_WIDTH_MIN    10    // 最小赛道宽度
    #define TRACK_WIDTH_MAX    60    // 最大赛道宽度
    #define LOST_TRACK_THRESHOLD 10  // 判定为丢线的阈值

    // 巡线状态枚举
    typedef enum {
        TRACK_NORMAL,          // 正常巡线
        TRACK_LOST_LEFT,       // 丢失左边界
        TRACK_LOST_RIGHT,      // 丢失右边界
        TRACK_LOST_BOTH        // 全部丢线
    } TrackStatus;

    // 巡线结果结构体
    typedef struct {
        TrackStatus status;    // 巡线状态
        int leftEdge;          // 左边界位置
        int rightEdge;         // 右边界位置
        int center;            // 赛道中心位置
        int deviation;         // 偏差值 (正值表示偏右，负值表示偏左)
        int trackWidth;        // 赛道宽度
    } TrackResult;

    // 函数声明
    void Track_Init(void);
    TrackResult Track_Process(void);
    int GetTrackDeviation(void);
    void Track_Debug(void);

#endif // __TRACKING_H__
