#include "stm32f10x.h"
#include "LED.h"

/* ============================================================================
 *  引脚和极性配置
 *
 *  以后要换 LED 引脚，只改这几行，下面的函数都不用动
 * ==========================================================================*/

#define LED_PORT        GPIOA          /* 两个灯都在 GPIOA 端口 */
#define LED1_PIN        GPIO_Pin_0     /* LED1 = PA0 */
#define LED2_PIN        GPIO_Pin_1     /* LED2 = PA1 */

/* 点亮电平：0 = 低电平点亮，1 = 高电平点亮
 *
 * 两种常见接法：
 *   ① 3.3V → LED → 引脚      → 引脚输出 0 时导通 → 低电平点亮 → 填 0
 *   ② 引脚 → LED → GND       → 引脚输出 1 时导通 → 高电平点亮 → 填 1
 *
 * 用宏包起来，以后改接法只改这里，不用改函数
 */
#define LED_ON_LEVEL    0
#define LED_OFF_LEVEL   1              /* 和 ON 相反 */
void LED_Init(void)
{
	/* ① 开 GPIOA 的时钟
	 *    不使能时钟，引脚的寄存器写了也没反应 —— 这是新手最常忘的一步 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	/* ② 配置引脚
	 *    GPIO_Mode_Out_PP = 推挽输出
	 *      - 推挽：能主动输出高电平，也能主动输出低电平
	 *      - 对比 I2C 用的"开漏"，推挽更适合直接驱动 LED */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;              /* 推挽输出 */
	GPIO_InitStructure.GPIO_Pin   = LED1_PIN | LED2_PIN;           /* 两个引脚一起配（按位或） */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;              /* 翻转速度，LED 不快，50M 也够 */
	GPIO_Init(LED_PORT, &GPIO_InitStructure);

	/* ③ 开机先全部关掉
	 *    不写这句的话，上电瞬间引脚电平是随机的，灯可能闪一下 */
	GPIO_WriteBit(LED_PORT, LED1_PIN | LED2_PIN, (BitAction)LED_OFF_LEVEL);
}

void LED1_ON(void)
{
	/* 只操作 LED1_PIN，不影响 LED2（这是"按位操作"的关键）
	 * 如果写成 GPIO_WriteBit(..., LED1_PIN|LED2_PIN, 0)，两个灯会一起亮 */
	GPIO_WriteBit(LED_PORT, LED1_PIN, (BitAction)LED_ON_LEVEL);
}

void LED1_OFF(void)
{
	GPIO_WriteBit(LED_PORT, LED1_PIN, (BitAction)LED_OFF_LEVEL);
}

void LED1_Turn(void)
{
	/* 先查现在是什么状态，再取反
	 * 常用于"按一下切换"的功能 */
	if (LED1_IsOn()) LED1_OFF();
	else             LED1_ON();
}

void LED2_ON(void)
{
	GPIO_WriteBit(LED_PORT, LED2_PIN, (BitAction)LED_ON_LEVEL);
}

void LED2_OFF(void)
{
	GPIO_WriteBit(LED_PORT, LED2_PIN, (BitAction)LED_OFF_LEVEL);
}

void LED2_Turn(void)
{
	if (LED2_IsOn()) LED2_OFF();
	else             LED2_ON();
}
/* 读【输出寄存器】，不是输入寄存器
 *
 * GPIO_ReadOutputDataBit = 读"我们让它输出什么"
 * GPIO_ReadInputDataBit  = 读"引脚上实际是什么电平"
 *
 * 这里是输出模式，读输出寄存器才是对的。
 * 好处：不管你在哪改的灯（按键、闪烁、串口），查出来的一定是真实状态
 */
uint8_t LED1_IsOn(void)
{
	return (GPIO_ReadOutputDataBit(LED_PORT, LED1_PIN) == LED_ON_LEVEL) ? 1 : 0;
	/*      ↑ 读出来的电平                  ↑ 和"点亮电平"比
	 *      相等 → 灯是亮的 → 返回 1
	 *      不等 → 灯是灭的 → 返回 0
	 */
}

uint8_t LED2_IsOn(void)
{
	return (GPIO_ReadOutputDataBit(LED_PORT, LED2_PIN) == LED_ON_LEVEL) ? 1 : 0;
}