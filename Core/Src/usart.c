/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  LL_USART_InitTypeDef USART_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);

  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
  /**USART1 GPIO Configuration
  PA9   ------> USART1_TX
  PA10   ------> USART1_RX
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_9;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_10;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_FLOATING;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USART1 DMA Init */

  /* USART1_TX Init */
  LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_4, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);

  LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_4, LL_DMA_PRIORITY_HIGH);

  LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_4, LL_DMA_MODE_NORMAL);

  LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_4, LL_DMA_PERIPH_NOINCREMENT);

  LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_4, LL_DMA_MEMORY_INCREMENT);

  LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_4, LL_DMA_PDATAALIGN_BYTE);

  LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_4, LL_DMA_MDATAALIGN_BYTE);

  /* USART1 interrupt Init */
  NVIC_SetPriority(USART1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
  NVIC_EnableIRQ(USART1_IRQn);

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  USART_InitStruct.BaudRate = 115200;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART1, &USART_InitStruct);
  LL_USART_ConfigAsyncMode(USART1);
  LL_USART_Enable(USART1);
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/* USER CODE BEGIN 1 */
#if defined __GNUC__
int __io_putchar(int ch)
#else
int fputc(int ch,FILE *f)
#endif
{
	/* check mutex , if don't take mutex , block here */
	//xSemaphoreTake(uart_mutex, osWaitForever);

	LL_USART_TransmitData8(USART1, (uint8_t)ch);
    while(!LL_USART_IsActiveFlag_TC(USART1)) ;
	return ch;
	/* release mutex */
	//xSemaphoreGive(uart_mutex);

}
#if defined __GNUC__
//int _read (int file, char *ptr, int len)
//{
//	int DataIdx; //
//	for (DataIdx = 0; DataIdx < len; DataIdx++)
//	{
//	  *ptr++ = __io_getchar();
//	}
//	return len;
//}

int _write(int file, char *ptr, int len)
{
	int DataIdx;

	(void)file;

	for (DataIdx = 0; DataIdx < len; DataIdx++)
	{
	   __io_putchar( *ptr++ );

	}
	return len;
}
#endif

void uart_init(void)
{
	/* create mutex for uart output */
	//uart_mutex = xSemaphoreCreateMutex();
	MX_USART1_UART_Init();
}

void uart_get_mutex(void)
{
//	xSemaphoreTake(uart_mutex, osWaitForever);
;
}

void uart_release_mutex(void)
{
//	xSemaphoreGive(uart_mutex);
;
}

void uart_raw_send(void * base, uint8_t * data, uint16_t size)
{
	uint16_t i;
	if (data == NULL) {
		return;
	}

#if 0
	for (i = 0; i < size; i++) {
		LL_USART_TransmitData8((USART_TypeDef *)base, data[i]);

		while(!LL_USART_IsActiveFlag_TC((USART_TypeDef *)base)) ;
	}
#endif

	LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4);
	LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_4, LL_DMA_MODE_NORMAL);
	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, size);
	LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_4, LL_USART_DMA_GetRegAddr(base));
	LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_4, (uint32_t)data);
	LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);
	LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_4);
	LL_USART_EnableDMAReq_TX(base);
	LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);


	//HAL_UART_Transmit(&huart1, data, size, HAL_MAX_DELAY);
}

uint32_t get_uart_baud(void *arg)
{
	uint32_t periphclk = LL_RCC_PERIPH_FREQUENCY_NO;
	USART_TypeDef * USARTx;

	if (arg == NULL) {
		return -1;
	}

	USARTx = (USART_TypeDef *)arg;

	/* get periphy clock */
	LL_RCC_ClocksTypeDef rcc_clocks;
	LL_RCC_GetSystemClocksFreq(&rcc_clocks);

	if (USARTx == USART1)
    {
      periphclk = rcc_clocks.PCLK2_Frequency;
    }
    else if (USARTx == USART2)
    {
      periphclk = rcc_clocks.PCLK1_Frequency;
    }
#if defined(USART3)
    else if (USARTx == USART3)
    {
      periphclk = rcc_clocks.PCLK1_Frequency;
    }
#endif /* USART3 */
#if defined(UART4)
    else if (USARTx == UART4)
    {
      periphclk = rcc_clocks.PCLK1_Frequency;
    }
#endif /* UART4 */
#if defined(UART5)
    else if (USARTx == UART5)
    {
      periphclk = rcc_clocks.PCLK1_Frequency;
    }
#endif /* UART5 */
    else
    {
      /* Nothing to do, as error code is already assigned to ERROR value */
    }

	return LL_USART_GetBaudRate(USARTx, periphclk);
}

int set_uart_baud(void *arg, uint32_t baud)
{
	uint32_t periphclk = LL_RCC_PERIPH_FREQUENCY_NO;
	USART_TypeDef * USARTx;

	if (arg == NULL) {
		return -1;
	}

	USARTx = (USART_TypeDef *)arg;

	/* get periphy clock */
	LL_RCC_ClocksTypeDef rcc_clocks;
	LL_RCC_GetSystemClocksFreq(&rcc_clocks);

	if (USARTx == USART1)
    {
      periphclk = rcc_clocks.PCLK2_Frequency;
    }
    else if (USARTx == USART2)
    {
      periphclk = rcc_clocks.PCLK1_Frequency;
    }
#if defined(USART3)
    else if (USARTx == USART3)
    {
      periphclk = rcc_clocks.PCLK1_Frequency;
    }
#endif /* USART3 */
#if defined(UART4)
    else if (USARTx == UART4)
    {
      periphclk = rcc_clocks.PCLK1_Frequency;
    }
#endif /* UART4 */
#if defined(UART5)
    else if (USARTx == UART5)
    {
      periphclk = rcc_clocks.PCLK1_Frequency;
    }
#endif /* UART5 */
    else
    {
      /* Nothing to do, as error code is already assigned to ERROR value */
		return -1;
    }

	LL_USART_SetBaudRate(USARTx, periphclk, baud);
	return 0;
}

int uart_set_parity(void *uart, uint32_t parity)
{
	USART_TypeDef * USARTx;

	USARTx = (USART_TypeDef *)uart;
	LL_USART_SetParity(USARTx, parity);
	return 0;
}

uint32_t uart_get_parity(void *uart)
{
	USART_TypeDef * USARTx;

	USARTx = (USART_TypeDef *)uart;
	return LL_USART_GetParity(USARTx);
}
/* uart interrupt handler */
__weak void irq_uart_user_handler(void *arg, uint8_t data)
{
	(void) arg;
	(void) data;
	;
}

void HAL_UART_HANDLER(USART_TypeDef * base)
{
	uint8_t ch;

	
	if (LL_USART_IsActiveFlag_RXNE(base)) {
		ch = base->DR;

		irq_uart_user_handler(base, ch);
		LL_USART_ClearFlag_RXNE(base);
	}
}


/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
