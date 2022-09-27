/*************************************************************
*芯片：STM32F103RC
*实现功能：使用硬件IIC读写EEPROM
*日期：2022-09-26
*************************************************************/

#include "stm32f10x.h"
#include "bsp_led.h"
#include "bsp_usart.h"
#include "delay.h"
#include "bsp_e2prom.h"

uint8_t ReadBuff[100]={0};
uint8_t WriteBuff[100]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
					   0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
					   0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
					   100,200,150,250};

int main(void)
{
	uint8_t i = 0,statusWrite = 1,statusRead = 1;
	USART_Config();
	LED_GPIO_Config();
	delay_init();
	IIC_EEPROM_Init();
	EEPROM_INFO("\r\n这是一个IIC EEPROM 的测试程序！\n");
	/*
	//字节写入函数
	statusWrite = IIC_EEPROM_WriteByte(11,0x32);
	if(statusWrite==0)
	{
		printf("Write Successily\n");
	}
	EEPROM_Waite_Write_End();
	*/
	/*
	//页写入函数(Page:32字节)
	statusWrite = IIC_EEPROM_PageWrite(1,WriteBuff,31);
	if(statusWrite==0)
	{
		printf("Write Successily\n");
	}
	*/
	EEPROM_DEBUG("\r\n这是一个调试信息\n");
	statusWrite = IIC_EEPROM_BuffWrite(1,WriteBuff,100);
	if(statusWrite==0)
	{
		EEPROM_INFO("\r\nWrite Successily\n");
	}

	statusRead = IIC_EEPROM_Read(1,ReadBuff,100);
	if(statusRead==0)
	{
		EEPROM_INFO("\r\nRead Successily\n");
	}
	
	for(i=0;i<100;i++)
	{
		printf("\r\n读到的数据：%d \n",ReadBuff[i]);
	}
	
	while(1)
	{
		
	}
}
