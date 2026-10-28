/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "main.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "tm1934.h"
#include "tim.h"
#include "dma.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LED1_EFFECT_PERIOD_MS 8000U
#define LED1_FRAME_MS 20U
#define LED1_CHASE_STEP_MS 80U
#define LED1_CHASE_TAIL 4U
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void LED1_ColorWheel(uint8_t position, uint8_t brightness, uint8_t *red, uint8_t *green, uint8_t *blue);
static void LED1_Gradient(uint32_t tick);
static void LED1_Running(uint32_t tick);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void LED1_ColorWheel(uint8_t position, uint8_t brightness, uint8_t *red, uint8_t *green, uint8_t *blue)
{
  uint8_t r;
  uint8_t g;
  uint8_t b;

  if (position < 85U)
  {
    r = (uint8_t)(255U - position * 3U);
    g = (uint8_t)(position * 3U);
    b = 0U;
  }
  else if (position < 170U)
  {
    position -= 85U;
    r = 0U;
    g = (uint8_t)(255U - position * 3U);
    b = (uint8_t)(position * 3U);
  }
  else
  {
    position -= 170U;
    r = (uint8_t)(position * 3U);
    g = 0U;
    b = (uint8_t)(255U - position * 3U);
  }

  *red = (uint8_t)((uint16_t)r * brightness / 255U);
  *green = (uint8_t)((uint16_t)g * brightness / 255U);
  *blue = (uint8_t)((uint16_t)b * brightness / 255U);
}

static void LED1_Gradient(uint32_t tick)
{
  uint32_t phase = tick / LED1_FRAME_MS;
  uint32_t brightness = phase % 512U;

  if (brightness > 255U)
  {
    brightness = 511U - brightness;
  }

  for (uint16_t i = 0U; i < TM1934_LED_COUNT; i++)
  {
    uint8_t position = (uint8_t)(phase + (uint32_t)i * 256U / TM1934_LED_COUNT);
    uint8_t r;
    uint8_t g;
    uint8_t b;

    LED1_ColorWheel(position, (uint8_t)brightness, &r, &g, &b);
    TM1934_SetPixel(i, r, g, b);
  }

  TM1934_Show();
}

static void LED1_Running(uint32_t tick)
{
  if (TM1934_LED_COUNT == 1U)
  {
    uint8_t r;
    uint8_t g;
    uint8_t b;

    LED1_ColorWheel((uint8_t)(tick / 8U), 255U, &r, &g, &b);
    TM1934_SetPixel(0U, r, g, b);
    TM1934_Show();
    return;
  }

  if (TM1934_LED_COUNT == 0U)
  {
    return;
  }

  uint32_t head = (tick / LED1_CHASE_STEP_MS) % (TM1934_LED_COUNT + LED1_CHASE_TAIL);

  TM1934_Clear();

  for (uint16_t tail = 0U; tail < LED1_CHASE_TAIL; tail++)
  {
    if (head < tail)
    {
      continue;
    }

    uint16_t index = (uint16_t)(head - tail);

    if (index >= TM1934_LED_COUNT)
    {
      continue;
    }

    uint8_t r;
    uint8_t g;
    uint8_t b;

    LED1_ColorWheel((uint8_t)(tick / 8U + index * 8U), (uint8_t)(255U >> tail), &r, &g, &b);
    TM1934_SetPixel(index, r, g, b);
  }

  TM1934_Show();
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  TM1934_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    uint32_t tick = HAL_GetTick();

    if (((tick / LED1_EFFECT_PERIOD_MS) & 1U) == 0U)
    {
      LED1_Gradient(tick);
    }
    else
    {
      LED1_Running(tick);
    }

    HAL_Delay(LED1_FRAME_MS);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV2;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
