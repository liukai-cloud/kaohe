#include "stm32f10x.h"

/**
  * @brief  LED初始化
  *
  *   LED1 = PC13  蓝板【板载 LED】（低电平点亮）
  *   LED2 = PA2   外接 LED（低电平点亮，没接也不影响）
  *
  *   注意：蓝板的板载灯在 PC13，不是 PA1！
  *         江协的板子才是 PA1/PA2。
  */
void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 使能 GPIOA 和 GPIOC 两个时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);

	/* PC13：板载 LED，推挽输出 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;   /* PC13 驱动能力弱，用 2MHz */
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_SetBits(GPIOC, GPIO_Pin_13);                  /* 默认熄灭（高电平） */

	/* PA2：外接 LED（如果接了） */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_2);
}

/* ---------------- LED1 = PC13 板载灯 ---------------- */
void LED1_ON(void)
{
	GPIO_ResetBits(GPIOC, GPIO_Pin_13);     /* 低电平点亮 */
}
void LED1_OFF(void)
{
	GPIO_SetBits(GPIOC, GPIO_Pin_13);
}
void LED1_Turn(void)
{
	if (GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_13) == 0)
		GPIO_SetBits(GPIOC, GPIO_Pin_13);
	else
		GPIO_ResetBits(GPIOC, GPIO_Pin_13);
}

/* ---------------- LED2 = PA2 外接灯 ---------------- */
void LED2_ON(void)
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_2);
}
void LED2_OFF(void)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_2);
}
void LED2_Turn(void)
{
	if (GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_2) == 0)
		GPIO_SetBits(GPIOA, GPIO_Pin_2);
	else
		GPIO_ResetBits(GPIOA, GPIO_Pin_2);
}