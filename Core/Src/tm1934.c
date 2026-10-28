#include "tm1934.h"
#include "tim.h"
#include "stm32g4xx_hal_tim.h"

#define TM1934_DIR_PORT GPIOF
#define TM1934_DIR_PIN GPIO_PIN_2

#define TM1934_T0H 61U
#define TM1934_T1H 122U
#define TM1934_PERIOD 212U
#define TM1934_RESET_COUNT 300U

extern TIM_HandleTypeDef htim2;

static uint8_t tm1934_buffer[TM1934_LED_COUNT * 3U];
static uint32_t pwm_buffer[(TM1934_LED_COUNT * 24U) + TM1934_RESET_COUNT];

void TM1934_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOF_CLK_ENABLE();

  GPIO_InitStruct.Pin = TM1934_DIR_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(TM1934_DIR_PORT, &GPIO_InitStruct);
  HAL_GPIO_WritePin(TM1934_DIR_PORT, TM1934_DIR_PIN, GPIO_PIN_SET);

  TM1934_Clear();
  TM1934_Show();
}

void TM1934_SetPixel(uint16_t index, uint8_t red, uint8_t green, uint8_t blue)
{
  if (index >= TM1934_LED_COUNT)
  {
    return;
  }

  uint16_t offset = index * 3U;
  tm1934_buffer[offset] = red;
  tm1934_buffer[offset + 1U] = green;
  tm1934_buffer[offset + 2U] = blue;
}

void TM1934_SetAll(uint8_t red, uint8_t green, uint8_t blue)
{
  for (uint16_t i = 0U; i < TM1934_LED_COUNT; i++)
  {
    TM1934_SetPixel(i, red, green, blue);
  }
}

void TM1934_Clear(void)
{
  for (uint16_t i = 0U; i < sizeof(tm1934_buffer); i++)
  {
    tm1934_buffer[i] = 0U;
  }
}

void TM1934_Show(void)
{
  uint32_t idx = 0U;

  for (uint16_t i = 0U; i < (uint16_t)sizeof(tm1934_buffer); i++)
  {
    uint8_t byte = tm1934_buffer[i];
    for (int8_t bit = 7; bit >= 0; bit--)
    {
      if (byte & (1U << bit))
      {
        pwm_buffer[idx++] = TM1934_T1H;
      }
      else
      {
        pwm_buffer[idx++] = TM1934_T0H;
      }
    }
  }

  for (uint32_t r = 0U; r < TM1934_RESET_COUNT; r++)
  {
    pwm_buffer[idx++] = 0U;
  }

  HAL_TIM_PWM_Stop_DMA(&htim2, TIM_CHANNEL_1);
  __HAL_TIM_DISABLE_DMA(&htim2, TIM_DMA_UPDATE);
  __HAL_TIM_SET_COUNTER(&htim2, 0);
  __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE | TIM_FLAG_CC1);

  HAL_DMA_Start_IT(htim2.hdma[TIM_DMA_ID_CC1], (uint32_t)pwm_buffer, (uint32_t)&htim2.Instance->CCR1, idx);

  __HAL_TIM_ENABLE_DMA(&htim2, TIM_DMA_UPDATE);
  TIM_CCxChannelCmd(htim2.Instance, TIM_CHANNEL_1, TIM_CCx_ENABLE);
  __HAL_TIM_ENABLE(&htim2);
}
