#include "bsp_e2prom.h"

uint16_t I2C_TimeOut = 0;

static void IIC_EEPROM_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	/*开启IIC所用到GPIO端口的时钟*/
	EEPROM_I2C_GPIO_CLK_FUN(EEPROM_I2C_GPIO_SCK_CLK|EEPROM_I2C_GPIO_SDA_CLK,ENABLE);
	/*开启IIC外设时钟*/
	EEPROM_I2C_CLK_FUN(EEPROM_I2C_CLK,ENABLE);
	
	/*配置SCK、SDA所使用的GPIO引脚*/
	GPIO_InitStruct.GPIO_Pin = EEPROM_I2C_GPIO_SCK_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(EEPROM_I2C_GPIO_SCK_PORT,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = EEPROM_I2C_GPIO_SDA_PIN;
	/*这里可省略对mode和speed的赋值，因为变量的值未曾改变*/
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(EEPROM_I2C_GPIO_SDA_PORT,&GPIO_InitStruct);
}
/*I2C 模式的配置*/
static void IIC_EEPROM_Mode_Config(void)
{
	I2C_InitTypeDef I2C_InitStruct;
	
	I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStruct.I2C_ClockSpeed = IIC_CLK_SPEED;
	I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStruct.I2C_OwnAddress1 = STM32_OWN_ADDR;
	I2C_Init(EEPROM_I2Cx,&I2C_InitStruct);
	/*使能IIC*/
	I2C_Cmd(EEPROM_I2Cx, ENABLE);
}
/*初始化IIC配置*/
void IIC_EEPROM_Init(void)
{
	IIC_EEPROM_GPIO_Config();
	IIC_EEPROM_Mode_Config();
}

static uint8_t IIC_Waite_OutTime(uint8_t errorcode)
{
	EEPROM_DEBUG("IIC总线等待超时，errorcode:%d\n",errorcode);
	return 1;
}

/*
*向EEPROM写一字节
*写入成功返回0，否则返回非0
*/
uint8_t IIC_EEPROM_WriteByte(uint16_t addr,uint8_t data)
{
	I2C_TimeOut = IIC_TIMEOUT;
	uint8_t addrH = (addr>>8)&0xff;
	uint8_t addrL = addr&0xff;
	
	/*等待EEPROM 空闲*/
	while(I2C_GetFlagStatus (EEPROM_I2Cx,I2C_FLAG_BUSY)==SET)
	{
		I2C_TimeOut--;
		if(I2C_TimeOut==0)
			return IIC_Waite_OutTime(1);
	}
	
	/*发送起始信号*/
	I2C_GenerateSTART(EEPROM_I2Cx,ENABLE);
	/*检测EV5事件*/
	while(I2C_CheckEvent(EEPROM_I2Cx,I2C_EVENT_MASTER_MODE_SELECT)==ERROR)
	{
		I2C_TimeOut--;
		if(I2C_TimeOut==0)
			return IIC_Waite_OutTime(1);
	}
	I2C_TimeOut = IIC_TIMEOUT;
	/*检测到EV5事件，发送器件地址，和读写方向*/
	I2C_Send7bitAddress(EEPROM_I2Cx,EEPROM_DEVICE_ADDR,I2C_Direction_Transmitter);
	/*检测EV6事件*/
	while(I2C_CheckEvent(EEPROM_I2Cx,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)==ERROR)
	{
		I2C_TimeOut--;
		if(I2C_TimeOut==0)
			return IIC_Waite_OutTime(2);
	}
	/*检测到EV6事件，发送存储单元地址*/
	I2C_SendData(EEPROM_I2Cx,addrH);
	I2C_TimeOut = IIC_TIMEOUT;
	/*检测EV8事件*/
	while(I2C_CheckEvent(EEPROM_I2Cx,I2C_EVENT_MASTER_BYTE_TRANSMITTING)==ERROR)
	{
		I2C_TimeOut--;
		if(I2C_TimeOut==0)
			return IIC_Waite_OutTime(3);
	}
	I2C_SendData(EEPROM_I2Cx,addrL);
	
	I2C_TimeOut = IIC_TIMEOUT;
	/*检测EV8事件*/
	while(I2C_CheckEvent(EEPROM_I2Cx,I2C_EVENT_MASTER_BYTE_TRANSMITTING)==ERROR)
	{
		I2C_TimeOut--;
		if(I2C_TimeOut==0)
			return IIC_Waite_OutTime(3);
	}
	/*检测到EV8事件，发送一字节数据*/
	I2C_SendData(EEPROM_I2Cx,data);
	I2C_TimeOut = IIC_TIMEOUT;
	/*检测EV8事件*/
	while(I2C_CheckEvent(EEPROM_I2Cx,I2C_EVENT_MASTER_BYTE_TRANSMITTED)==ERROR)
	{
		I2C_TimeOut--;
		if(I2C_TimeOut==0)
			return IIC_Waite_OutTime(4);
	}
	/*发送停止信号*/
	I2C_GenerateSTOP(EEPROM_I2Cx,ENABLE);
	return 0;
}

/*
*向EEPROM写页写入
*写入成功返回0，否则返回非0
*/
uint8_t IIC_EEPROM_PageWrite(uint16_t addr,uint8_t* data,uint8_t numWriteBytes)
{
	I2C_TimeOut = IIC_TIMEOUT;
	uint8_t addrH = (addr>>8)&0xff;
	uint8_t addrL = addr&0xff;
	/*发送起始信号*/
	I2C_GenerateSTART(EEPROM_I2Cx,ENABLE);
	/*检测EV5事件*/
	while(I2C_CheckEvent(EEPROM_I2Cx,I2C_EVENT_MASTER_MODE_SELECT)==ERROR)
	{
		I2C_TimeOut--;
		if(I2C_TimeOut==0)
			return IIC_Waite_OutTime(1);
	}
	I2C_TimeOut = IIC_TIMEOUT;
	/*检测到EV5事件，发送器件地址，和读写方向*/
	I2C_Send7bitAddress(EEPROM_I2Cx,EEPROM_DEVICE_ADDR,I2C_Direction_Transmitter);
	/*检测EV6事件*/
	while(I2C_CheckEvent(EEPROM_I2Cx,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)==ERROR)
	{
		I2C_TimeOut--;
		if(I2C_TimeOut==0)
			return IIC_Waite_OutTime(2);
	}
	/*检测到EV6事件，发送存储单元地址*/
	I2C_SendData(EEPROM_I2Cx,addrH);
	I2C_TimeOut = IIC_TIMEOUT;
	/*检测EV8事件*/
	while(I2C_CheckEvent(EEPROM_I2Cx,I2C_EVENT_MASTER_BYTE_TRANSMITTING)==ERROR)
	{
		I2C_TimeOut--;
		if(I2C_TimeOut==0)
			return IIC_Waite_OutTime(3);
	}
	I2C_SendData(EEPROM_I2Cx,addrL);
	
	I2C_TimeOut = IIC_TIMEOUT;
	/*检测EV8事件*/
	while(I2C_CheckEvent(EEPROM_I2Cx,I2C_EVENT_MASTER_BYTE_TRANSMITTING)==ERROR)
	{
		I2C_TimeOut--;
		if(I2C_TimeOut==0)
			return IIC_Waite_OutTime(3);
	}
	while(numWriteBytes)
	{
		/*检测到EV8事件，发送一字节数据*/
		I2C_SendData(EEPROM_I2Cx,*data);
		I2C_TimeOut = IIC_TIMEOUT;
		/*检测EV8事件*/
		while(I2C_CheckEvent(EEPROM_I2Cx,I2C_EVENT_MASTER_BYTE_TRANSMITTED)==ERROR)
		{
			I2C_TimeOut--;
			if(I2C_TimeOut==0)
				return IIC_Waite_OutTime(4);
		}
		data++;
		numWriteBytes--;
	}
	/*发送停止信号*/
	I2C_GenerateSTOP(EEPROM_I2Cx,ENABLE);
	return 0;
}

uint8_t IIC_EEPROM_BuffWrite(uint16_t addr,uint8_t* buff,uint8_t numBuffBytes)
{
	uint8_t Countaddr = addr%EEPROM_PAGESIZE;
	uint8_t CountFullPages = numBuffBytes/EEPROM_PAGESIZE;
	uint8_t CountMorePages = numBuffBytes%EEPROM_PAGESIZE;
	/*等待EEPROM 空闲*/
	while(I2C_GetFlagStatus (EEPROM_I2Cx,I2C_FLAG_BUSY)==SET)
	{
		I2C_TimeOut--;
		if(I2C_TimeOut==0)
			return IIC_Waite_OutTime(1);
	}
	/*判断地址是否对齐*/
	if(Countaddr==0)
	{
		if(CountMorePages==0)
		{
			while(CountFullPages--)
			{
				IIC_EEPROM_PageWrite(addr,buff,EEPROM_PAGESIZE);
				EEPROM_Waite_Write_End();
				addr += EEPROM_PAGESIZE;
				buff += EEPROM_PAGESIZE;
			}
		}
		else
		{
			if(CountFullPages>0)
			{
				while(CountFullPages--)
				{
					IIC_EEPROM_PageWrite(addr,buff,EEPROM_PAGESIZE);
					EEPROM_Waite_Write_End();
					addr += EEPROM_PAGESIZE;
					buff += EEPROM_PAGESIZE;
				}
			}
			IIC_EEPROM_PageWrite(addr,buff,CountMorePages);
			EEPROM_Waite_Write_End();
		}
	}
	else
	{
		IIC_EEPROM_PageWrite(addr,buff,EEPROM_PAGESIZE-Countaddr);
		EEPROM_Waite_Write_End();
		addr += (EEPROM_PAGESIZE-Countaddr);
		buff += (EEPROM_PAGESIZE-Countaddr);
		CountFullPages = (numBuffBytes-EEPROM_PAGESIZE+Countaddr)/EEPROM_PAGESIZE;
		CountMorePages = (numBuffBytes-EEPROM_PAGESIZE+Countaddr)%EEPROM_PAGESIZE;
		if(CountMorePages==0)
		{
			while(CountFullPages--)
			{
				IIC_EEPROM_PageWrite(addr,buff,EEPROM_PAGESIZE);
				EEPROM_Waite_Write_End();
				addr += EEPROM_PAGESIZE;
				buff += EEPROM_PAGESIZE;
			}
		}
		else
		{
			if(CountFullPages>0)
			{
				while(CountFullPages--)
				{
					IIC_EEPROM_PageWrite(addr,buff,EEPROM_PAGESIZE);
					EEPROM_Waite_Write_End();
					addr += EEPROM_PAGESIZE;
					buff += EEPROM_PAGESIZE;
				}
			}
			IIC_EEPROM_PageWrite(addr,buff,CountMorePages);
			EEPROM_Waite_Write_End();
		}
	}
	return 0;
}

/*从EEPROM读取数据
*函数执行成功返回0，否则返回非0
*/
uint8_t IIC_EEPROM_Read(uint16_t addr,uint8_t* data,uint8_t numReadToBytes)
{
	I2C_TimeOut = IIC_TIMEOUT;
	uint8_t addrH = (addr>>8)&0xff;
	uint8_t addrL = addr&0xff;
	/*发送起始信号*/
	I2C_GenerateSTART(EEPROM_I2Cx,ENABLE);
	/*检测EV5事件*/
	while(I2C_CheckEvent(EEPROM_I2Cx,I2C_EVENT_MASTER_MODE_SELECT)==ERROR)
	{
		I2C_TimeOut--;
		if(I2C_TimeOut==0)
			return IIC_Waite_OutTime(5);
	}
	/*检测到EV5事件，发送器件地址，和读写方向*/
	I2C_Send7bitAddress(EEPROM_I2Cx,EEPROM_DEVICE_ADDR,I2C_Direction_Transmitter);
	I2C_TimeOut = IIC_TIMEOUT;
	/*检测EV6事件*/
	while(I2C_CheckEvent(EEPROM_I2Cx,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)==ERROR)
	{
		I2C_TimeOut--;
		if(I2C_TimeOut==0)
			return IIC_Waite_OutTime(6);
	}
	/*检测到EV6事件，发送存储单元地址*/
	I2C_SendData(EEPROM_I2Cx,addrH);
	I2C_TimeOut = IIC_TIMEOUT;
	/*检测EV8事件*/
	while(I2C_CheckEvent(EEPROM_I2Cx,I2C_EVENT_MASTER_BYTE_TRANSMITTING)==ERROR)
	{
		I2C_TimeOut--;
		if(I2C_TimeOut==0)
			return IIC_Waite_OutTime(7);
	}
	/*检测到EV8事件，发送存储单元地址*/
	I2C_SendData(EEPROM_I2Cx,addrL);
	I2C_TimeOut = IIC_TIMEOUT;
	/*检测EV8事件*/
	while(I2C_CheckEvent(EEPROM_I2Cx,I2C_EVENT_MASTER_BYTE_TRANSMITTING)==ERROR)
	{
		I2C_TimeOut--;
		if(I2C_TimeOut==0)
			return IIC_Waite_OutTime(7);
	}
	
	/*再次发送起始信号*/
	I2C_GenerateSTART(EEPROM_I2Cx,ENABLE);
	I2C_TimeOut = IIC_TIMEOUT;
	/*检测EV5事件*/
	while(I2C_CheckEvent(EEPROM_I2Cx,I2C_EVENT_MASTER_MODE_SELECT)==ERROR)
	{
		I2C_TimeOut--;
		if(I2C_TimeOut==0)
			return IIC_Waite_OutTime(8);
	}
	/*检测到EV5事件，发送器件地址，和读写方向*/
	I2C_Send7bitAddress(EEPROM_I2Cx,EEPROM_DEVICE_ADDR,I2C_Direction_Receiver);
	I2C_TimeOut = IIC_TIMEOUT;
	/*检测EV6事件*/
	while(I2C_CheckEvent(EEPROM_I2Cx,I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)==ERROR)
	{
		I2C_TimeOut--;
		if(I2C_TimeOut==0)
			return IIC_Waite_OutTime(9);
	}
	I2C_TimeOut = IIC_TIMEOUT;
	/*检测EV7事件*/
	while(I2C_CheckEvent(EEPROM_I2Cx,I2C_EVENT_MASTER_BYTE_RECEIVED)==ERROR)
	{
		I2C_TimeOut--;
		if(I2C_TimeOut==0)
			return IIC_Waite_OutTime(10);
	}
	while(numReadToBytes)
	{
		if(numReadToBytes==1)
		{
			I2C_AcknowledgeConfig(EEPROM_I2Cx,DISABLE);
		}
		while(I2C_CheckEvent(EEPROM_I2Cx,I2C_EVENT_MASTER_BYTE_RECEIVED)==ERROR)
		{
			I2C_TimeOut--;
			if(I2C_TimeOut==0)
				return IIC_Waite_OutTime(11);
		}
		/*检测到EV7事件，接收数据*/
		*data = I2C_ReceiveData(EEPROM_I2Cx);

		data++;
		numReadToBytes--;
	}
	/*发送停止信号*/
	I2C_GenerateSTOP(EEPROM_I2Cx,ENABLE);
	/*通讯周期结束后，恢复默认主机ack响应使能*/
	I2C_AcknowledgeConfig(EEPROM_I2Cx,ENABLE);
	return 0;
}

uint8_t EEPROM_Waite_Write_End(void)
{
	I2C_TimeOut = IIC_TIMEOUT;
	do
	{
		/*发送起始信号*/
		I2C_GenerateSTART(EEPROM_I2Cx,ENABLE);
		/*检测EV5事件*/
		while(I2C_GetFlagStatus (EEPROM_I2Cx,I2C_FLAG_SB)==RESET)
		{
			I2C_TimeOut--;
			if(I2C_TimeOut==0)
				return IIC_Waite_OutTime(12);
		}
		/*检测到EV5事件，发送器件地址，和读写方向*/
		I2C_Send7bitAddress(EEPROM_I2Cx,EEPROM_DEVICE_ADDR,I2C_Direction_Transmitter);
		/*检测EV6事件*/
	}while(I2C_GetFlagStatus(EEPROM_I2Cx,I2C_FLAG_ADDR)==RESET);

	/*产生stop信号*/
	I2C_GenerateSTOP(EEPROM_I2Cx,ENABLE);
	return 0;
}
