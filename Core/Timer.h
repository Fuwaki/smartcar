#ifndef __TIMER_H__
#define __TIMER_H__
    void Timer0_Init(void);
    void SwitchUpdater(void);
    extern unsigned long timestamp; //单位是100us(0.1ms为1)
#endif /* __TIMER_H__ */