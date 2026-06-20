/*
 * sd_spi_cfg.h
 *
 *  Created on: Jun 20, 2026
 *      Author: Asus
 */

#ifndef INC_SD_SPI_CFG_H_
#define INC_SD_SPI_CFG_H_

#include "main.h"

extern SPI_HandleTypeDef hspi1;

#define SD_SPI             hspi1

#define SD_CS_GPIO_Port    GPIOB
#define SD_CS_Pin          GPIO_PIN_0

#endif /* INC_SD_SPI_CFG_H_ */
