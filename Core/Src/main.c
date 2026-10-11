/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    main.c
  * @brief   ระบบสัญญาณกันขโมย IR + Buzzer สำหรับ NUCLEO-F411RE
  *
  *  การต่อสาย:
  *    IR sensor  : VCC->3V3, GND->GND, OUT->D2  (PA10)  [LOW = เจอวัตถุ]
  *    Buzzer     : VCC->3V3, GND->GND, I/O->D12 (PA6)   [TIM3_CH1 PWM]
  *    ปุ่ม B1 (PC13) : เปิด/ปิดระบบ
  *    LED LD2 (PA5)  : ดับ=ปิดระบบ, ติด=เปิดระบบ, กะพริบ=กำลังเตือน
  *    USART2 (115200) : ส่ง log ไปคอมผ่านสาย USB ของ ST-Link
  *
  *  การตั้งค่า CubeMX ที่ต้องทำก่อน Generate:
  *    - PA10 : GPIO_EXTI10, Label = IR_OUT, Falling edge, Pull-up
  *    - PC13 : GPIO_EXTI13 (B1), Falling edge
  *    - NVIC : เปิด EXTI line[15:10] interrupts
  *    - TIM3 : Channel1 = PWM Generation CH1 (PA6)
  *    - Clock: HCLK = 84 MHz (APB1 timer clock = 84 MHz)
  ******************************************************************************
  */
/* USER CODE END Header */
#include "main.h"

/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
/* USER CODE END Includes */

TIM_HandleTypeDef htim3;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
typedef enum { DISARMED, ARMED, ALARM } State;

volatile State   state       = DISARMED;
volatile uint8_t irTriggered = 0;
volatile uint8_t btnPressed  = 0;
uint32_t intrusionCount      = 0;
/* USER CODE END PV */

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART2_UART_Init(void);

/* USER CODE BEGIN 0 */

/* ส่งข้อความออก UART แบบเดียวกับ printf */
static void Log(const char *fmt, ...)
{
  char buf[96];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  HAL_UART_Transmit(&huart2, (uint8_t *)buf, strlen(buf), 100);
}

/* สั่งความถี่เสียง (Hz), 0 = เงียบ  — timer นับที่ 1 MHz */
static void Buzzer_Tone(uint32_t freq)
{
  if (freq == 0) {
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
    return;
  }
  uint32_t arr = 1000000UL / freq - 1;
  __HAL_TIM_SET_AUTORELOAD(&htim3, arr);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, arr / 2);   /* duty 50% */
  __HAL_TIM_SET_COUNTER(&htim3, 0);
}

static void Beep(uint32_t freq, uint32_t ms)
{
  Buzzer_Tone(freq);
  HAL_Delay(ms);
  Buzzer_Tone(0);
}

/* ใน interrupt ทำแค่ตั้ง flag แล้วออกให้เร็วที่สุด */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  static uint32_t lastBtn = 0;

  if (GPIO_Pin == IR_OUT_Pin) {
    irTriggered = 1;
  } else if (GPIO_Pin == B1_Pin) {
    uint32_t now = HAL_GetTick();
    if (now - lastBtn > 200) {          /* debounce 200 ms */
      btnPressed = 1;
      lastBtn = now;
    }
  }
}
/* USER CODE END 0 */

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_TIM3_Init();
  MX_USART2_UART_Init();

  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  Buzzer_Tone(0);
  Beep(1500, 80);                       /* เสียงสั้นบอกว่าบอร์ดพร้อม */
  Log("\r\n=== IR Alarm ready === (press B1 to arm)\r\n");
  /* USER CODE END 2 */

  while (1)
  {
    /* USER CODE BEGIN 3 */

    /* ---------- ปุ่ม B1: เปิด/ปิดระบบ ---------- */
    if (btnPressed) {
      btnPressed = 0;
      if (state == DISARMED) {
        state = ARMED;
        Beep(2000, 100); HAL_Delay(80); Beep(2000, 100);   /* ติ๊ด ติ๊ด */
        Log("[%lu ms] ARMED\r\n", HAL_GetTick());
      } else {
        state = DISARMED;
        Buzzer_Tone(0);
        Beep(1000, 300);                                   /* ปี๊ดยาว */
        Log("[%lu ms] DISARMED\r\n", HAL_GetTick());
      }
      irTriggered = 0;                  /* ทิ้งสัญญาณ IR ที่ค้างระหว่างกดปุ่ม */
    }

    /* ---------- IR จับได้ ---------- */
    if (irTriggered) {
      irTriggered = 0;
      if (state == ARMED) {
        state = ALARM;
        intrusionCount++;
        Log("[%lu ms] !!! INTRUSION #%lu !!!\r\n", HAL_GetTick(), intrusionCount);
      }
    }

    /* ---------- เสียงและไฟตามสถานะ ---------- */
    if (state == ALARM) {
      static int32_t f = 800, dir = 20;           /* ไซเรนกวาด 800–1600 Hz */
      Buzzer_Tone((uint32_t)f);
      f += dir;
      if (f >= 1600 || f <= 800) dir = -dir;
      HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin,
                        ((HAL_GetTick() / 100) % 2) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_Delay(5);
    } else {
      HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin,
                        (state == ARMED) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
    /* USER CODE END 3 */
  }
}

/* ---------- Clock: HSI 16 MHz -> PLL -> 84 MHz ---------- */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM            = 16;
  RCC_OscInitStruct.PLL.PLLN            = 336;
  RCC_OscInitStruct.PLL.PLLP            = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ            = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();

  RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                   | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;   /* APB1 42 MHz, timer 84 MHz */
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) Error_Handler();
}

/* ---------- TIM3 CH1 PWM (PA6) : 84 MHz / 84 = 1 MHz tick ---------- */
static void MX_TIM3_Init(void)
{
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  htim3.Instance               = TIM3;
  htim3.Init.Prescaler         = 83;
  htim3.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim3.Init.Period            = 999;
  htim3.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK) Error_Handler();

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK) Error_Handler();

  sConfigOC.OCMode     = TIM_OCMODE_PWM1;
  sConfigOC.Pulse      = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;   /* โมดูล buzzer แบบ low-level trigger */
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) Error_Handler();

  HAL_TIM_MspPostInit(&htim3);          /* ตั้งขา PA6 เป็น AF2 (อยู่ใน stm32f4xx_hal_msp.c) */
}

/* ---------- USART2 115200 8N1 (ผ่าน ST-Link Virtual COM) ---------- */
static void MX_USART2_UART_Init(void)
{
  huart2.Instance          = USART2;
  huart2.Init.BaudRate     = 115200;
  huart2.Init.WordLength   = UART_WORDLENGTH_8B;
  huart2.Init.StopBits     = UART_STOPBITS_1;
  huart2.Init.Parity       = UART_PARITY_NONE;
  huart2.Init.Mode         = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK) Error_Handler();
}

/* ---------- GPIO ---------- */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /* B1 (PC13) : ปุ่มบนบอร์ด มี pull-up ภายนอกแล้ว */
  GPIO_InitStruct.Pin  = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /* LD2 (PA5) */
  GPIO_InitStruct.Pin   = LD2_Pin;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /* IR_OUT (PA10) : LOW เมื่อเจอวัตถุ */
  GPIO_InitStruct.Pin  = IR_OUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(IR_OUT_GPIO_Port, &GPIO_InitStruct);

  /* PA10 และ PC13 ใช้ IRQ เดียวกัน */
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void Error_Handler(void)
{
  __disable_irq();
  while (1) { }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  (void)file; (void)line;
}
#endif
