/*
 * waiter.h
 *
 *  Created on: Feb 14, 2023
 *      Author: martin
 */

#pragma once

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

static TIM_HandleTypeDef *waiter_us_tim;

static inline void waiter_us_init(TIM_HandleTypeDef *htimx)
{
	waiter_us_tim = htimx;
	HAL_TIM_Base_Start(waiter_us_tim);
}

static inline void delay_us(uint16_t us)
{
	__HAL_TIM_SET_COUNTER(waiter_us_tim, 0);
	while(__HAL_TIM_GET_COUNTER(waiter_us_tim) < us);
}

#ifdef __cplusplus
}           /* closing brace for extern "C" */
#endif
