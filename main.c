#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"
#include "Serial.h"
#include "Key.h"
#include <stdio.h>
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
		OLED_ShowString(4,1,"K4:BACK");
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

int main(void)
{
	uint8_t key;

	OLED_Init();
	Key_Init();

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
			if (key == 4)
			{
				Page = PAGE_MAIN;
				Page_Show();
			}
		}

		Delay_ms(10);
	}
}
