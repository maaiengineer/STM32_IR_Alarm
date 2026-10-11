/* stm32f4xx_it.c - Interrupt handlers */
#include "main.h"
#include "stm32f4xx_it.h"

void NMI_Handler(void)        { while (1) { } }
void HardFault_Handler(void)  { while (1) { } }
void MemManage_Handler(void)  { while (1) { } }
void BusFault_Handler(void)   { while (1) { } }
void UsageFault_Handler(void) { while (1) { } }
void SVC_Handler(void)        { }
void DebugMon_Handler(void)   { }
void PendSV_Handler(void)     { }

/* 1 ms tick - required for HAL_Delay / HAL_GetTick */
void SysTick_Handler(void)
{
  HAL_IncTick();
}

/* EXTI lines 10..15 : IR_OUT (PA10) and B1 (PC13) */
void EXTI15_10_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(IR_OUT_Pin);
  HAL_GPIO_EXTI_IRQHandler(B1_Pin);
}
