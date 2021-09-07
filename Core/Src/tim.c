/**
  ******************************************************************************
  * @file    tim.c
  * @brief   This file provides code for the configuration
  *          of the TIM instances.
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
#include "tim.h"

/* USER CODE BEGIN 0 */
#include "sci.h"
/* USER CODE END 0 */

/* TIM2 init function */
void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  LL_TIM_InitTypeDef TIM_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);

  /* TIM2 interrupt Init */
  NVIC_SetPriority(TIM2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
  NVIC_EnableIRQ(TIM2_IRQn);

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  TIM_InitStruct.Prescaler = 71;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 65535;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM2, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM2);
  LL_TIM_SetClockSource(TIM2, LL_TIM_CLOCKSOURCE_INTERNAL);
  LL_TIM_SetOnePulseMode(TIM2, LL_TIM_ONEPULSEMODE_SINGLE);
  LL_TIM_SetTriggerOutput(TIM2, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM2);
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}
/* TIM3 init function */
void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  LL_TIM_InitTypeDef TIM_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);

  /* TIM3 interrupt Init */
  NVIC_SetPriority(TIM3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
  NVIC_EnableIRQ(TIM3_IRQn);

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  TIM_InitStruct.Prescaler = 71;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 1000;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM3, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM3);
  LL_TIM_SetClockSource(TIM3, LL_TIM_CLOCKSOURCE_INTERNAL);
  LL_TIM_SetOnePulseMode(TIM3, LL_TIM_ONEPULSEMODE_SINGLE);
  LL_TIM_SetTriggerOutput(TIM3, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM3);
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}
/* TIM4 init function */
void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  LL_TIM_InitTypeDef TIM_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM4);

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  TIM_InitStruct.Prescaler = 71;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 5000;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM4, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM4);
  LL_TIM_SetClockSource(TIM4, LL_TIM_CLOCKSOURCE_INTERNAL);
  LL_TIM_SetOnePulseMode(TIM4, LL_TIM_ONEPULSEMODE_SINGLE);
  LL_TIM_SetTriggerOutput(TIM4, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM4);
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/* USER CODE BEGIN 1 */
//void timer_init(void *tim_baseHandle)
//{
//	TIM_TypeDef * tim_base;
//
//	tim_base = (TIM_TypeDef *)tim_baseHandle;
//
//	if (tim_base == TIM6) {
//		MX_TIM6_Init();
//	}
//	else if (tim_base == TIM7) {
//		MX_TIM7_Init();
//	}
//
//	/* stop timer */
//	LL_TIM_DisableCounter(tim_base);
//	/* disable interrupt */
//	LL_TIM_DisableIT_UPDATE(tim_base);
//	/* clear update flag */
//	LL_TIM_ClearFlag_UPDATE(tim_base);
//}

int tim_stop(void *tim_baseHandle)
{
	TIM_TypeDef * tim_base;

	tim_base = (TIM_TypeDef *)tim_baseHandle;
	if (tim_base == NULL) {
		sys_log_e("input invalid parameter");
		return -1;
	}

	/*claer all interrupt */
	tim_base->SR = 0x00;
	tim_base->CR1 &= ~(TIM_CR1_CEN);

	return 0;
}

int tim_phy_int_en(void *tim_baseHandle, uint32_t en)
{
	TIM_TypeDef * tim_base;

	tim_base = (TIM_TypeDef *)tim_baseHandle;

	if (en)
		LL_TIM_EnableIT_UPDATE(tim_base);
	else
		LL_TIM_DisableIT_UPDATE(tim_base);
}

int tim_start(void *tim_baseHandle)
{	
	TIM_TypeDef * tim_base;
	tim_base = (TIM_TypeDef *)tim_baseHandle;
	if (tim_base == NULL) {
		sys_log_e("input invalid parameter");
		return -1;
	}

	tim_base->CR1 |= (TIM_CR1_CEN);

	return 0;
}

__weak void irq_tim_handler(void *arg)
{
	(void)arg;
}

void _timer_handeler(void *tim_baseHandle)
{
	TIM_TypeDef * tim_base;
	tim_base = (TIM_TypeDef *)tim_baseHandle;
	if (tim_base == NULL) {
		sys_log_e("input invalid parameter");
		return ;
	}
	if (LL_TIM_IsActiveFlag_UPDATE(tim_base))
		irq_tim_handler((void *)tim_base);

	/* clear update flag */
	LL_TIM_ClearFlag_UPDATE(tim_base);
}

void reload_timer_counter_value(void *tim_baseHandle, uint32_t val, uint32_t fore_en)
{
	TIM_TypeDef *tim_base;
	uint32_t tim_cr1,int_flag;

	tim_base = (TIM_TypeDef *)tim_baseHandle;
	if (tim_base == NULL) {
		sys_log_e("input invalid parameter");
		return ;
	}

	/* backup */
	tim_cr1 = tim_base->CR1;
	int_flag = tim_base->DIER;

	/* stop timer */
	LL_TIM_DisableCounter(tim_base);
	/* disable interrupt */
	LL_TIM_DisableIT_UPDATE(tim_base);
	/* load ARR value */
	LL_TIM_SetAutoReload(tim_base, val);
	/* generate update event  */
	LL_TIM_GenerateEvent_UPDATE(tim_base);
	/* clear update flag */
	LL_TIM_ClearFlag_UPDATE(tim_base);

	/* restore */
	tim_base->DIER = int_flag;
	if (fore_en) {
		tim_base->CR1 = TIM_CR1_CEN;
	}
	else
		tim_base->CR1 = tim_cr1;
}

// max 5000 us
void udelay(uint32_t xus)
{

	LL_TIM_DisableCounter(TIM4);
	LL_TIM_ClearFlag_UPDATE(TIM4);
	LL_TIM_SetCounter(TIM4, 5000 - xus);
	LL_TIM_EnableCounter(TIM4);

	while (!LL_TIM_IsActiveFlag_UPDATE(TIM4));
	
	LL_TIM_DisableCounter(TIM4);
}

void mdelay(uint32_t xms)
{
	while(xms --)
		udelay(1000);
}


/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
