#include "stm32f10x.h"
#include "Key.h"
#include "Delay.h"

/**
  * @brief  按键初始化
  *
  *   K1 = PA4     K2 = PA6     K3 = PB1     K4 = PB11
  *   每个按键：一端接引脚，另一端接 GND
  *   上拉输入：不按读到 1，按下读到 0
  *
  *   注意：PA4/PA6 在 GPIOA，PB1/PB11 在 GPIOB，
  *         所以要开【两个】端口时钟，并且分两次初始化
  *
  * @param  无
  * @retval 无
  */
void Key_Init(void)
{
	/* 两个端口时钟都要开 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			/* 上拉输入 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	/* ---------- GPIOA：PA4、PA6 ---------- */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_6;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* ---------- GPIOB：PB1、PB11 ---------- */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_11;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/**
  * @brief  读按键
  * @retval 0 = 没按
  *         1 = K1 (PA4)
  *         2 = K2 (PA6)
  *         3 = K3 (PB1)
  *         4 = K4 (PB11)
  */
uint8_t Key_GetNum(void)
{
	uint8_t KeyNum = 0;

	/* ---------------- K1 = PA4 ---------------- */
	if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4)==0)
	{
		Delay_ms(20);											/* 消抖 */
		while (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4)==0);		/* 等松手，分号不能少 */
		Delay_ms(20);											/* 松手后再消抖 */
		KeyNum=1;
	}

	/* ---------------- K2 = PA6 ---------------- */
	if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_6)==0)
	{
		Delay_ms(20);
		while (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_6)==0);
		Delay_ms(20);
		KeyNum=2;
	}

	/* ---------------- K3 = PB1 ---------------- */
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)==0)
	{
		Delay_ms(20);
		while (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)==0);
		Delay_ms(20);
		KeyNum=3;
	}

	/* ---------------- K4 = PB11 ---------------- */
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11)==0)
	{
		Delay_ms(20);
		while (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11)==0);
		Delay_ms(20);
		KeyNum=4;
	}

	return KeyNum;
}
