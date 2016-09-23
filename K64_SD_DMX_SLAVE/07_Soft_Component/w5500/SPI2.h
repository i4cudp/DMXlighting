#ifndef __SPI2_H
#define __SPI2_H

#include "spi.h"
#include "psptypes.h"
#include "gpio.h"

#define WIZ_SCS			    (PTB_NUM|20)	// out
#define WIZ_SCLK			(PTB_NUM|21)	// out
#define WIZ_MISO			(PTB_NUM|23)	// in
#define WIZ_MOSI			(PTB_NUM|22)	// out

void WIZ_SPI_Init(void);
void WIZ_CS(uint8_t val);
uint8_t SPI2_SendByte(uint8_t byte);
#endif

