#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"
#include "Serial.h"
#include "Key.h"
#include <stdio.h>
#include <stdlib.h>
#include "LED.h"
#include "OLED_Chinese.h"
#include <string.h>

#define PAGE_MAIN   0
#define PAGE_LED    1
#define PAGE_INFO   2
#define PAGE_LINE   3
#define PAGE_EXT    4

#define MENU_NUM    4

uint8_t Page   = PAGE_MAIN;
uint8_t Cursor = 0;


char *MenuText[MENU_NUM] = { "LED", "INFO", "LINE", "EXTEND" };

uint8_t LedMode =0;
uint16_t BlinkCnt=0;
uint8_t BlinkPhase=0;
char *ModeTest[3]={"OFF  ","ON   ","BLINK"};

int16_t VarA=0;

void Menu_Show(void)
{
	uint8_t i;

	OLED_Clear();

	for(i=0;i<MENU_NUM;i++)
	{
		if(i==Cursor) OLED_ShowChar(i+1,1,'>');
		else          OLED_ShowChar(i+1,1,' ');

		OLED_ShowChar(i+1,2,'1'+i);
		OLED_ShowString(i+1,3,MenuText[i]);
	}
}

void Page_Show(void)
{
	switch (Page)
	{
	case PAGE_MAIN:
		Menu_Show();
		break;

	case PAGE_LED:
		OLED_Clear();
		OLED_ShowString(1,1,"LED PAGE");
		OLED_ShowString(2,1,"K1:ON/OFF");
		OLED_ShowString(3,1,"K2:BLINK");
		OLED_ShowString(4,1,"K4:BACK");
		break;

	case PAGE_INFO:
		OLED_Clear();
		OLED_ShowString(1,1,"INFO PAGE");
		OLED_ShowString(2,1,"WangWeizheng");		
		OLED_ShowString(3,1,"ID:32601357");
		
		break;

	case PAGE_LINE:
		OLED_Clear();
		OLED_ShowString(1,1,"LINE PAGE");
		OLED_ShowString(2,1,"SENSOR:");
		OLED_ShowString(4,1,"K4:BACK");
		break;

	case PAGE_EXT:
		OLED_Clear();
		OLED_ShowString(1,1,"EXTEND PAGE");
		OLED_ShowString(2,1,"TODO");
		OLED_ShowString(4,1,"K4:BACK");
		break;

	default:
		break;
	}
}



void LED_Show(void)
{
	OLED_ShowString(2,1,"MODE:     ");
	OLED_ShowString(2,6,ModeTest[LedMode]);

	OLED_ShowString(3,1,"L1:");
	OLED_ShowString(3,4,LED1_IsOn() ?"ON":"OFF");

	OLED_ShowString(3,8,"L2:");
	OLED_ShowString(3,11,LED2_IsOn() ?"ON":"OFF");

}
void Var_Show(void)
{
	OLED_ShowString(4,1,"A=      ");
	OLED_ShowSignedNum(4,3,VarA,5);
}
void Cmd_Handle(void)
{
	char *p;

	if (Serial_GetRxFlag() != 1) return;

	p = Serial_RxPacket;
	if (*p == '@') p++;                    /* 跳过 '@' */

	if (p[0] == '\0') return;              /* 空包不管 */

	/* ---------- @? 或 GET A 读取变量（c 项 3 分） ---------- */
	if (p[0] == '?' || p[0] == 'G' || p[0] == 'g')
	{
		printf("A=%d\r\n", VarA);
		return;
	}

	/* ---------- @111 设置变量并回显（b 项 3 分） ---------- */
	if ((p[0] >= '0' && p[0] <= '9') || p[0] == '-')
	{
		VarA = (int16_t)atoi(p);
		if (Page == PAGE_INFO) Var_Show();
		printf("A=%d\r\n", VarA);
		return;
	}

	printf("ERR\r\n");
}



int main(void)
{
	uint8_t key;

	OLED_Init();
	Key_Init();
	Serial_Init();
	Page_Show();

	while (1)
	{
		key = Key_GetNum();

		if (Page == PAGE_MAIN)
		{
			if (key == 1)
			{
				if (Cursor == 0) Cursor = MENU_NUM - 1;
				else             Cursor--;
				Page_Show();
			}
			else if (key == 2)
			{
				Cursor = (Cursor + 1) % MENU_NUM;
				Page_Show();
			}
			else if (key == 3)
			{
				Page = Cursor + 1;
				Page_Show();
			}
		}
		else
		{
			if(Page==PAGE_LED)
			{
				if(key==1)
				{
					if(LedMode==1)
					{
						LedMode=0;
						LED1_OFF();
						LED2_OFF();
					}
					else
					{
						LedMode=1;
						LED1_ON();
						LED2_ON();
					}

					BlinkCnt=0;
					LED_Show();
				}
				else if(key==2)
				{
					LedMode=2;
					BlinkCnt=0;
					BlinkPhase=0;
					LED1_ON();
					LED2_OFF();

					LED_Show();
				}
			}



			if (key == 4)
			{
				LedMode=0;
				BlinkCnt=0;

				Page = PAGE_MAIN;
				Page_Show();
			}
		}

		if(LedMode==2&&Page==PAGE_LED)
		{
			BlinkCnt++;

			if(BlinkCnt>=50)
			{
				BlinkCnt=0;
				BlinkPhase=!BlinkPhase;

				if(BlinkPhase)
				{
					LED1_ON();
					LED2_OFF();
				}
				else
				{
					LED1_OFF();
					LED2_ON();
				}
				LED_Show();
			}
		}
		Cmd_Handle();  
		Delay_ms(10);
	}
}
