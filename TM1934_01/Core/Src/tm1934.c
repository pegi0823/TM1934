#include "tm1934.h"

#define TM1934_GPIO_PORT GPIOA
#define TM1934_GPIO_PIN GPIO_PIN_0
#define TM1934_DIR_PORT GPIOF
#define TM1934_DIR_PIN GPIO_PIN_2

#define TM1934_T0H_NS 340U
#define TM1934_T1H_NS 720U
#define TM1934_PERIOD_NS 1250U
#define TM1934_RESET_US 300U

static uint8_t tm1934_buffer[TM1934_LED_COUNT * 3U];
static uint32_t tm1934_cycles_per_us;
static uint32_t tm1934_t0h_cycles;
static uint32_t tm1934_t1h_cycles;
static uint32_t tm1934_period_cycles;

static void TM1934_DelayUs(uint32_t us)
{
  uint32_t start = DWT->CYCCNT;
  uint32_t cycles = us * tm1934_cycles_per_us;
  while ((DWT->CYCCNT - start) < cycles)
  {
  }
}

static void TM1934_WaitUntil(uint32_t target)
{
  while ((int32_t)(DWT->CYCCNT - target) < 0)
  {
  }
}

static void TM1934_WriteBit(uint8_t value)
{
  uint32_t start = DWT->CYCCNT;
  TM1934_GPIO_PORT->BSRR = TM1934_GPIO_PIN;
  TM1934_WaitUntil(start + (value ? tm1934_t1h_cycles : tm1934_t0h_cycles));
  TM1934_GPIO_PORT->BSRR = (uint32_t)TM1934_GPIO_PIN << 16U;
  TM1934_WaitUntil(start + tm1934_period_cycles);
}

static void TM1934_WriteByte(uint8_t value)
{
  for (uint8_t mask = 0x80U; mask != 0U; mask >>= 1U)
  {
    TM1934_WriteBit((value & mask) != 0U);
  }
}

void TM1934_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();

  GPIO_InitStruct.Pin = TM1934_DIR_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(TM1934_DIR_PORT, &GPIO_InitStruct);
  HAL_GPIO_WritePin(TM1934_DIR_PORT, TM1934_DIR_PIN, GPIO_PIN_SET);

  GPIO_InitStruct.Pin = TM1934_GPIO_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(TM1934_GPIO_PORT, &GPIO_InitStruct);
  HAL_GPIO_WritePin(TM1934_GPIO_PORT, TM1934_GPIO_PIN, GPIO_PIN_RESET);

  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0U;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  tm1934_cycles_per_us = SystemCoreClock / 1000000U;
  tm1934_t0h_cycles = (SystemCoreClock / 1000U) * TM1934_T0H_NS / 1000000U;
  tm1934_t1h_cycles = (SystemCoreClock / 1000U) * TM1934_T1H_NS / 1000000U;
  tm1934_period_cycles = (SystemCoreClock / 1000U) * TM1934_PERIOD_NS / 1000000U;

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
  uint32_t primask = __get_PRIMASK();

  __disable_irq();

  TM1934_GPIO_PORT->BSRR = (uint32_t)TM1934_GPIO_PIN << 16U;
  TM1934_DelayUs(TM1934_RESET_US);

  for (uint16_t i = 0U; i < sizeof(tm1934_buffer); i++)
  {
    TM1934_WriteByte(tm1934_buffer[i]);
  }

  TM1934_GPIO_PORT->BSRR = (uint32_t)TM1934_GPIO_PIN << 16U;
  TM1934_DelayUs(TM1934_RESET_US);

  __set_PRIMASK(primask);
}
