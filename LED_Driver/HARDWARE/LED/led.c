#include "led.h"


void LED_Init()
{
  GPIO_InitTypeDef GPIO_InitStructure;

	//打开PB口时钟
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	//PB12引脚设置
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	//端口速度
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	//端口模式，此为输出推挽模式
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	//初始化对应的端口
  GPIO_Init(GPIOB, &GPIO_InitStructure);
	 GPIO_SetBits(GPIOB,GPIO_Pin_12);	
}
