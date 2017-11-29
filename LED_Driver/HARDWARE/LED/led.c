#include "led.h"


void LED_Init()
{
  GPIO_InitTypeDef GPIO_InitStructure;

	//��PB��ʱ��
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	//PB12��������
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	//�˿��ٶ�
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	//�˿�ģʽ����Ϊ�������ģʽ
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	//��ʼ����Ӧ�Ķ˿�
  GPIO_Init(GPIOB, &GPIO_InitStructure);
	 GPIO_SetBits(GPIOB,GPIO_Pin_12);	
}
