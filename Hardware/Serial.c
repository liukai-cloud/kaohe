#include "stm32f10x.h"
#include <stdio.h>

/*==============================================================================
 * 9-5 串口收发文本数据包
 *
 *   数据包格式：  @内容\r\n
 *                  ↑    ↑
 *                包头  包尾
 *
 *   例如电脑发：  @LED_ON\r\n
 *        板子收到 Serial_RxPacket = "LED_ON"
 *============================================================================*/

char Serial_TxPacket[4];      /* 发送的 4 个字符 */
char Serial_RxPacket[100];    /* 接收缓冲区 */
uint8_t Serial_RxFlag;        /* 收到一包的标志 */


void Serial_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate=9600;
	USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode=USART_Mode_Tx|USART_Mode_Rx;
	USART_InitStructure.USART_Parity=USART_Parity_No;
	USART_InitStructure.USART_StopBits=USART_StopBits_1;
	USART_InitStructure.USART_WordLength=USART_WordLength_8b;
	USART_Init(USART1,&USART_InitStructure);

	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel=USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;
	NVIC_Init(&NVIC_InitStructure);

	USART_Cmd(USART1,ENABLE);
}

void Serial_SendByte(uint8_t Byte)
{
	USART_SendData(USART1,Byte);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
}

void Serial_SendArray(uint8_t *Array,uint16_t Length)
{
	uint16_t i;
	for(i=0;i<Length;i++)
	{
		Serial_SendByte(Array[i]);
	}
}

void Serial_SendString(char *String)
{
	uint8_t i;
	for(i=0;String[i]!=0;i++)
	{
		Serial_SendByte(String[i]);
	}
}

uint32_t Serial_Pow(uint32_t x,uint32_t y)
{
	uint32_t r=1;
	while (y--)
	{
		r*=x;
	}
	return r;
}

void Serial_SendNumber(uint32_t number,uint8_t L)
{
	uint8_t i;
	for (i=0;i<L;i++)
	{
		Serial_SendByte(number/Serial_Pow(10,L-i-1)%10+'0');
	}
}


/* 发一包： @内容\r\n */
void Serial_SendPacket(void)
{
	Serial_SendByte('@');
	Serial_SendArray((uint8_t *)Serial_TxPacket,4);
	Serial_SendString("\r\n");
}


uint8_t Serial_GetRxFlag(void)
{
	if(Serial_RxFlag==1)
	{
		Serial_RxFlag = 0;
		return 1;
	}
	return 0;
}


/*==============================================================================
 *  接收中断：解析 @内容\r\n
 *
 *   S = 0  等包头 '@'
 *   S = 1  一直存字符，直到收到 '\r'
 *   S = 2  等 '\n'，收到说明一包结束
 *============================================================================*/
void USART1_IRQHandler(void)
{
	static uint8_t RxState = 0;
	static uint8_t pRxPacket = 0;

	if(USART_GetITStatus(USART1,USART_IT_RXNE)==SET)
	{
		uint8_t RxData = USART_ReceiveData(USART1);

		if(RxState==0)
		{
			if(RxData=='@' && Serial_RxFlag==0)
			{
				RxState = 1;
				pRxPacket = 0;
			}
		}
		else if(RxState==1)
		{
			if(RxData=='\r')
			{
				RxState = 2;
			}
			else
			{
				Serial_RxPacket[pRxPacket] = RxData;
				pRxPacket++;
				if(pRxPacket>=100) pRxPacket = 0;
			}
		}
		else if(RxState==2)
		{
			if(RxData=='\n')
			{
				Serial_RxPacket[pRxPacket] = '\0';
				Serial_RxFlag = 1;
				RxState = 0;
			}
		}

		USART_ClearITPendingBit(USART1,USART_IT_RXNE);
	}
}