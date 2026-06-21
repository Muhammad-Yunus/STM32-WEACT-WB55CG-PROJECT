/*
 * sd_spi.h
 *
 *  Created on: Jun 20, 2026
 *      Author: Asus
 */

#ifndef INC_SD_SPI_H_
#define INC_SD_SPI_H_

#include <stdint.h>
#include "stm32wbxx_hal.h"

typedef uint8_t SD_Result;

#define SD_OK     0
#define SD_ERROR  1

SD_Result SD_Init(void);

uint8_t SD_ReadSector(uint32_t sector, uint8_t *buff);
uint8_t SD_WriteSector(uint32_t sector, const uint8_t *buff);
uint32_t SD_GetSectorCount(void);
void SD_WipeBootSectors(void);  // Wipe sectors 0-3 to clear corrupted MBR/FAT

#endif /* INC_SD_SPI_H_ */
