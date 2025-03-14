#ifndef __BSP_USART_H__
#define __BSP_USART_H__

#include "stm32f10x.h"
#include <stdio.h>
//����2-USART2
#define DEBUG_USARTx				USART2
#define	DEBUG_USART_CLK				RCC_APB1Periph_USART2
#define	DEBUG_USART_APBxClkCmd		RCC_APB1PeriphClockCmd
#define DEBUG_USART_BAUDRATE		115200

//USART GPIO ���ź궨��
#define	DEBUG_USART_GPIO_CLK		(RCC_APB2Periph_GPIOA)
#define DEBUG_USART_GPIO_APBxClkCmd	RCC_APB2PeriphClockCmd

#define	DEBUG_USART_TX_GPIO_PORT	GPIOA
#define	DEBUG_USART_TX_GPIO_PIN		GPIO_Pin_2
#define	DEBUG_USART_RX_GPIO_PORT	GPIOA
#define	DEBUG_USART_RX_GPIO_PIN		GPIO_Pin_3
#define DEBUG_USART_IRQ				USART2_IRQn
#define DEBUG_USART_IRQHandler		USART2_IRQHandler

void USART_Config(void);
//static void NVIC_Configuration(void);
void Usart_SendByte(USART_TypeDef * pUSARTx,uint8_t data);
uint8_t USART_RecevieByte(USART_TypeDef * pUSARTx);

#endif /*__BSP_USART_H__*/
