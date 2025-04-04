#include "TrigTable.h"
#include <math.h>

// 定义三角函数表
float sin_table[TRIG_TABLE_SIZE];
float cos_table[TRIG_TABLE_SIZE];

// 初始化三角函数表
void initTrigTables(void)
{
    int i;
    float angle;
    
    for (i = 0; i < TRIG_TABLE_SIZE; i++)
    {
        angle = i * 2.0f * 3.14159265358979323846f / TRIG_TABLE_SIZE;
        sin_table[i] = sin(angle);
        cos_table[i] = cos(angle);
    }
}

// 角度范围归一化到[0, 2π)
static float normalizeAngle(float angle)
{
    while (angle >= 2.0f * 3.14159265358979323846f)
        angle -= 2.0f * 3.14159265358979323846f;
    while (angle < 0)
        angle += 2.0f * 3.14159265358979323846f;
    return angle;
}

// 快速查表获取sin值（使用线性插值提高精度）
float fast_sin(float angle)
{
    float index_float, fraction;
    int index, index_next;
    angle = normalizeAngle(angle);
    
    index_float = angle * TRIG_TABLE_SIZE / (2.0f * 3.14159265358979323846f);
    index = (int)index_float;
    fraction = index_float - index;
    
    index_next = (index + 1) % TRIG_TABLE_SIZE;
    
    return sin_table[index] + fraction * (sin_table[index_next] - sin_table[index]);
}

// 快速查表获取cos值（使用线性插值提高精度）
float fast_cos(float angle)
{
    float index_float, fraction;
    int index, index_next;
    angle = normalizeAngle(angle);
    
    index_float = angle * TRIG_TABLE_SIZE / (2.0f * 3.14159265358979323846f);
    index = (int)index_float;
    fraction = index_float - index;
    
    index_next = (index + 1) % TRIG_TABLE_SIZE;
    
    return cos_table[index] + fraction * (cos_table[index_next] - cos_table[index]);
}
