#include <math.h>
#include <stdio.h>
#include "cameraController.h"
#include "Tracking.h"

// 全局变量
TrackResult trackResult;            // 巡线结果
int lastValidCenter = TRACK_CENTER; // 上一次有效的中心点位置

/*

  osu!mania
  4k woc这个cop怎么buch
-
 -
   -
    -
-
  -
   -
    -
  - -
 - -
-
 -
  -
    -
   -
-
  -

让cop写一段
lmaoing
让cop写铺子
lmao

该上楼梯了
buggy cop
*/


// 初始化巡线模块
void Track_Init(void)
{
    // 初始化CCD摄像头
    CCD_Init();

    // 初始化巡线结果
    trackResult.status = TRACK_LOST_BOTH;
    trackResult.leftEdge = 0;
    trackResult.rightEdge = CCD_LENGTH - 1;
    trackResult.center = TRACK_CENTER;
    trackResult.deviation = 0;
    trackResult.trackWidth = 0;

    // 初始化上一次有效中心点
    lastValidCenter = TRACK_CENTER;
}

// 寻找赛道边缘
// 从中心向两边搜索跳变点
static void FindTrackEdges(unsigned char *binaryData)
{
    int i;
    int leftFound = 0, rightFound = 0;
    int leftEdge = 0, rightEdge = CCD_LENGTH - 1;

    // 从中心向左搜索左边界
    for (i = TRACK_CENTER; i > 0; i--)
    {
        if (binaryData[i] == 1 && binaryData[i - 1] == 0)
        {
            leftEdge = i;
            leftFound = 1;
            break;
        }
    }

    // 从中心向右搜索右边界
    for (i = TRACK_CENTER; i < CCD_LENGTH - 1; i++)
    {
        if (binaryData[i] == 1 && binaryData[i + 1] == 0)
        {
            rightEdge = i;
            rightFound = 1;
            break;
        }
    }

    // 更新边缘状态
    if (leftFound && rightFound)
    {
        trackResult.status = TRACK_NORMAL;
    }
    else if (leftFound)
    {
        trackResult.status = TRACK_LOST_RIGHT;
    }
    else if (rightFound)
    {
        trackResult.status = TRACK_LOST_LEFT;
    }
    else
    {
        trackResult.status = TRACK_LOST_BOTH;
    }

    // 更新边缘位置
    trackResult.leftEdge = leftEdge;
    trackResult.rightEdge = rightEdge;
}

// 处理巡线逻辑
TrackResult Track_Process(void)
{
    unsigned char *binaryData;
    int center;

    // 采集CCD数据并进行二值化处理
    StartCCD();

    // 获取二值化后的数据
    binaryData = CCD_GetBinaryData();

    // 寻找赛道边缘
    FindTrackEdges(binaryData);

    // 计算赛道宽度
    trackResult.trackWidth = trackResult.rightEdge - trackResult.leftEdge;

    // 根据边缘状态计算中心点位置
    switch (trackResult.status)
    {
    case TRACK_NORMAL:
        // 正常赛道，计算中心位置
        center = (trackResult.leftEdge + trackResult.rightEdge) / 2;
        lastValidCenter = center;
        break;

    case TRACK_LOST_LEFT:
        // 丢失左边界，根据赛道预期宽度估算
        if (trackResult.trackWidth > TRACK_WIDTH_MIN)
        {
            center = trackResult.rightEdge - trackResult.trackWidth / 2;
        }
        else
        {
            center = trackResult.rightEdge - TRACK_WIDTH_MIN;
        }
        break;

    case TRACK_LOST_RIGHT:
        // 丢失右边界，根据赛道预期宽度估算
        if (trackResult.trackWidth > TRACK_WIDTH_MIN)
        {
            center = trackResult.leftEdge + trackResult.trackWidth / 2;
        }
        else
        {
            center = trackResult.leftEdge + TRACK_WIDTH_MIN;
        }
        break;

    case TRACK_LOST_BOTH:
    default:
        // 完全丢线，使用上一次有效的中心点
        center = lastValidCenter;
        break;
    }

    // 限制中心点范围
    if (center < 0)
        center = 0;
    if (center >= CCD_LENGTH)
        center = CCD_LENGTH - 1;

    // 更新中心点和偏差值
    trackResult.center = center;
    trackResult.deviation = center - TRACK_CENTER;

    return trackResult;
}

// 获取巡线偏差值 - 用于提供给控制系统
int GetTrackDeviation(void)
{
    return trackResult.deviation;
}

// // 调试输出函数
// void Track_Debug(void)
// {
//     printf("状态: %d, 左边界: %d, 右边界: %d\n",
//            trackResult.status, trackResult.leftEdge, trackResult.rightEdge);
//     printf("中心点: %d, 偏差: %d, 宽度: %d\n",
//            trackResult.center, trackResult.deviation, trackResult.trackWidth);
// }

// // 其他可能需要的巡线辅助函数可以在此处添加
