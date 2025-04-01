#ifndef TRIG_TABLE_H
#define TRIG_TABLE_H

// 三角函数表的大小
#define TRIG_TABLE_SIZE 256

// 外部声明三角函数表
extern float sin_table[TRIG_TABLE_SIZE];
extern float cos_table[TRIG_TABLE_SIZE];

// 初始化三角函数表
void initTrigTables(void);

// 从表中查询sin值
float fast_sin(float angle);

// 从表中查询cos值
float fast_cos(float angle);

#endif // TRIG_TABLE_H
