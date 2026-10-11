/* main.h - NUCLEO-F411RE IR Alarm */
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
void Error_Handler(void);

#define B1_Pin              GPIO_PIN_13
#define B1_GPIO_Port        GPIOC
#define B1_EXTI_IRQn        EXTI15_10_IRQn
#define USART_TX_Pin        GPIO_PIN_2
#define USART_TX_GPIO_Port  GPIOA
#define USART_RX_Pin        GPIO_PIN_3
#define USART_RX_GPIO_Port  GPIOA
#define LD2_Pin             GPIO_PIN_5
#define LD2_GPIO_Port       GPIOA
#define BUZZER_Pin          GPIO_PIN_6
#define BUZZER_GPIO_Port    GPIOA
#define IR_OUT_Pin          GPIO_PIN_10
#define IR_OUT_GPIO_Port    GPIOA
#define IR_OUT_EXTI_IRQn    EXTI15_10_IRQn

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
