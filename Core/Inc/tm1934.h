#ifndef __TM1934_H
#define __TM1934_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#ifndef TM1934_LED_COUNT
#define TM1934_LED_COUNT 8U
#endif

void TM1934_Init(void);
void TM1934_SetPixel(uint16_t index, uint8_t red, uint8_t green, uint8_t blue);
void TM1934_SetAll(uint8_t red, uint8_t green, uint8_t blue);
void TM1934_Clear(void);
void TM1934_Show(void);

#ifdef __cplusplus
}
#endif

#endif
