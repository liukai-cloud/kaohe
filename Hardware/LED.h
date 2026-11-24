#ifndef __LED_H          /* 防止头文件被重复包含：第一次进来时 __LED_H 未定义，才编译下面的内容 */
#define __LED_H          /* 定义标记，第二次再 include 就会跳过 */

#include "stm32f10x.h"   /* 用到了 uint8_t，所以要包含标准库 */

/* 初始化：把两个 LED 引脚配成推挽输出 */
void LED_Init(void);

/* 两个灯的开、关、翻转 */
void LED1_ON(void);
void LED1_OFF(void);
void LED1_Turn(void);        /* Turn = 翻转（亮的变灭，灭的变亮） */

void LED2_ON(void);
void LED2_OFF(void);
void LED2_Turn(void);

/* 查询灯现在的状态：亮返回 1，灭返回 0 */
uint8_t LED1_IsOn(void);
uint8_t LED2_IsOn(void);

#endif                   /* __LED_H 结束 */