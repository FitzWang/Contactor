#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "stdio.h"
#include "timer.h"
#include "iwdg.h"
void Modbus_ask(u8 *send_num);
void Modbus_analys(void);
void LED_Init(void);
unsigned int calc_crc16 (u16 *snd, unsigned char num);
void Buff_Clear(void);

u16 UART2_BUF[12];
int UART2_Con=0;
//u8  Data_send_LED[26]={0xFE,0x5C,0x4B,0x89,0x1A,0x00,0x00,0x00,0x65
//,0x00,0x00,0x00,0x00,0x07,0x00,0x00,0x00,0x29,0x00,0xFF,0xFF,0x02,0x23,0x32,0xFF,0xFF};
/*LED发送相关*/
u8  Data_send_LED[100]={0xFE,0x5C,0x4B,0x89,   //前导标识 0
	0x39,0x00,0x00,0x00,                        //命令总长度 4    *****需要修改******
	0x65,0x00,0x00,0x00,0x00,                   //固定填充 8
	0x26,0x00,0x00,0x00,                        //指令总长 13     *****需要修改******      
	0x29,0x00,0xFF,0xFF,0x04,0x30,0x30,0x2E,0x30,      //具体指令 17   空气温度      *****需要修改******
	0x2A,0x00,0xFF,0xFF,0x04,0x30,0x30,0x2E,0x30, //具体指令           空气湿度      *****需要修改******
	0x2B,0x00,0xFF,0xFF,0x06,0x30,0x30,0x30,0x30,0x30,0x30,       //   光照          *****需要修改******  
	0x2C,0x00,0xFF,0xFF,0x04,0x30,0x30,0x30,0x30,                  //  二氧化碳      *****需要修改******
	0xFF,0xFF};                               //后导标识
/*询问帧相关*/
u8  Temp_Sensor_Send[8]={0x01,0x03,0x00,0x00,0x00,0x02,0xC4,0x0B};
u8  Light_Sensor_Send[8]={0x02,0x03,0x00,0x02,0x00,0x02,0x65,0xF8};
u8  Co2_Sensor_Send[8]={0x03,0x03,0x00,0x02,0x00,0x01,0x24,0x28};
u8  Soil_Sensor_Send[8]={0x04,0x03,0x00,0x00,0x00,0x02,0xC4,0x5E};
u8  RX_Status,Tx_Count;
/*数值计算相关*/
u16 Air_Temp_Value=0,Air_Hum_Value=0; // 空气温湿度
u32 Light_Value=0;                  //光照强度
u16 Co2_Value=0;                    //二氧化碳浓度
u16 Soil_Temp_Value=0,Soil_Hum_Value=0;//土壤温湿度

u8 send_count=0,i;
volatile u8 led=0;

 int main(void)
 {	
  
	delay_init();	    	 //延时函数初始化	
	NVIC_Configuration();// 设置中断优先级分组
	LED_Init();
	TIM3_Int_Init(4999,7199);
	IWDG_Init(4,625);    //与分频数为64,重载值为625,溢出时间为1s	   
	uart1_init(9600);	 //串口初始化
	uart2_init(9600);
//	uart3_init(9600);
	
  delay_ms(1000);
  delay_ms(1000);
  delay_ms(1000);
 
	 while(1)
	 { 	
		
		 delay_ms(1000);
		 /*温湿度,ID:1*/
		 RX_Status=1;Tx_Count=1;
		 Modbus_ask(Temp_Sensor_Send);
  	 delay_ms(500);
  	 Modbus_analys();
		 /*光照,ID:2*/
		 RX_Status=1;Tx_Count=2;
		 Modbus_ask(Light_Sensor_Send);
		 delay_ms(500);
		 Modbus_analys();
		 /*CO2,ID:3*/
		 RX_Status=1;Tx_Count=3;
		 Modbus_ask(Co2_Sensor_Send);
		 delay_ms(500);
		 Modbus_analys();
		 /*土壤,ID:4*/
		 RX_Status=1;Tx_Count=4;
		 Modbus_ask(Soil_Sensor_Send);
		 delay_ms(500);
		 Modbus_analys();
//	   printf("Air Temp:%d   Light_Value:%d   Co2_Value:%d  Air_Hum_Value=%d\r\n",Air_Temp_Value,Light_Value,Co2_Value,Air_Hum_Value);
//     printf("0xFE 0x5C 0x4B 0x89 0x1A 0x00 0x00 0x00 0x65 0x00 0x00 0x00 0x00 0x07 0x00 0x00 0x00 0x29 0x00 0xFF 0xFF 0x02 %x %x 0xFF 0xFF",a,b);
      for(i=0;i<57;i++)		
		 {
			  USART_SendData(USART1,Data_send_LED[i]);
		    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
		 }

	 }

}

void USART2_IRQHandler(void)                	//串口2中断服务程序
	{

	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  
		{
	 			if(RX_Status==1)
				{ UART2_Con=0;
					RX_Status=0;
				}
         UART2_BUF[UART2_Con]=USART_ReceiveData(USART2);
         UART2_Con++;
		
     } 

} 




//传感器询问函数
void Modbus_ask(u8 *send_num)
{
	 for(send_count=0;send_count<8;send_count++)
		{
		 USART_ClearFlag(USART2, USART_FLAG_TC); 	
	   USART_SendData(USART2,send_num[send_count]);
     while(	USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
		}
	
}
//MODBUS数据解析函数
//void Modbus_analys()
//{
//	if(UART2_BUF[0]==0x01)
//	{ if(UART2_BUF[5]<0xf0)
//		Temp_Value=((float)(UART2_BUF[6]+256*UART2_BUF[5]))/10;
//		else
//		Temp_Value=((float)((UART2_BUF[6]+256*UART2_BUF[5])-0xffff-0x0001))/10;
//	}
//	if(UART2_BUF[0]==0x02)
//	{ if(UART2_BUF[3]<0xf0)
//			Soil_Temp_Value=((float)(UART2_BUF[4]+256*UART2_BUF[3]))/100;
//		else
//			Soil_Temp_Value=((float)((UART2_BUF[4]+256*UART2_BUF[3])-0xffff-0x0001))/100;
//		  Soil_Hum_Value=((float)(UART2_BUF[6]+256*UART2_BUF[5]))/100;
//	}
//}
void Modbus_analys()
{
	if(UART2_BUF[0]==0x01)
	{ if(calc_crc16(UART2_BUF,9)==0)
		{
		Air_Temp_Value=UART2_BUF[6]+256*UART2_BUF[5];
			if(Air_Temp_Value>0xff00)
				Air_Temp_Value=0;
		Data_send_LED[22]=(Air_Temp_Value/100)%10+48;
		Data_send_LED[23]=(Air_Temp_Value/10)%10+48;
		Data_send_LED[25]=Air_Temp_Value%10+48;
			
		Air_Hum_Value=UART2_BUF[4]+256*UART2_BUF[3];
		Data_send_LED[31]=(Air_Hum_Value/100)%10+48;
		Data_send_LED[32]=(Air_Hum_Value/10)%10+48;
		Data_send_LED[34]=Air_Hum_Value%10+48;	
		}
	}

	if(UART2_BUF[0]==0x02)
	{if(calc_crc16(UART2_BUF,9)==0)
		{
			Light_Value=UART2_BUF[4]*65536+UART2_BUF[5]*256+UART2_BUF[6];
			Data_send_LED[40]=(Light_Value/100000)%10+48;
			Data_send_LED[41]=(Light_Value/10000)%10+48;
			Data_send_LED[42]=(Light_Value/1000)%10+48;
			Data_send_LED[43]=(Light_Value/100)%10+48;
		  Data_send_LED[44]=(Light_Value/10)%10+48;
		  Data_send_LED[45]=Light_Value%10+48;
		}
	}
 if(UART2_BUF[0]==0x03)
	{if(calc_crc16(UART2_BUF,7)==0)
		{
			Co2_Value=UART2_BUF[3]*256+UART2_BUF[4];
			Data_send_LED[51]=(Co2_Value/1000)%10+48;
			Data_send_LED[52]=(Co2_Value/100)%10+48;
		  Data_send_LED[53]=(Co2_Value/10)%10+48;
		  Data_send_LED[54]=Co2_Value%10+48;
		}
	}
  if(UART2_BUF[0]==0x04)
	{ if(calc_crc16(UART2_BUF,9)==0)
		{
		Soil_Hum_Value=UART2_BUF[6]+256*UART2_BUF[5];
		
		}
	}

	Buff_Clear();
}

void Buff_Clear()
{
	for(i=0;i<10;i++)
	{
		UART2_BUF[i]=0x00;
	}
}

unsigned int calc_crc16 (u16 *snd, unsigned char num)
{
	unsigned char i, j;
	unsigned int c,crc=0xFFFF;
	for(i = 0; i < num; i ++)
	{
		c = snd[i] & 0x00FF; 
		crc ^= c; 
		for(j = 0;j < 8; j ++)
	{
		if (crc & 0x0001)
		{
			crc>>=1; 
			crc^=0xA001; 
		}
		else 
		{
			crc>>=1; 
				}
	}
}
		return(crc); 
}

void TIM3_IRQHandler(void)   //TIM3中断
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源 
		{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //清除TIMx的中断待处理位:TIM 中断源 
		IWDG_Feed();
	  led = !led;
 		PBout(12)= led;

		}
}



