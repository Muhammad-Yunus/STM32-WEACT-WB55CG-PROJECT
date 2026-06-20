/*
 * sd_spi.c
 *
 *  Created on: Jun 20, 2026
 *      Author: Asus
 */

#include "sd_spi.h"
#include <string.h>

// ===== USER CONFIG =====
extern SPI_HandleTypeDef hspi1;

#define SD_CS_LOW()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET)
#define SD_CS_HIGH()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET)

// ===== SD COMMANDS =====
#define CMD0  (0x40+0)
#define CMD8  (0x40+8)
#define CMD17 (0x40+17)
#define CMD24 (0x40+24)
#define CMD55 (0x40+55)
#define CMD41 (0x40+41)
#define CMD58 (0x40+58)

// ===== CARD TYPE =====
static uint8_t isSDHC = 0;

// ===== LOW LEVEL SPI =====
static uint8_t SPI_Xfer(uint8_t data)
{
    uint8_t rx;
    HAL_SPI_TransmitReceive(&hspi1, &data, &rx, 1, HAL_MAX_DELAY);
    return rx;
}

static void SPI_ClockDummy(uint32_t n)
{
    for (uint32_t i = 0; i < n; i++)
        SPI_Xfer(0xFF);
}

// ===== SEND CMD =====
static uint8_t SD_SendCmd(uint8_t cmd, uint32_t arg, uint8_t crc)
{
    uint8_t res;
    uint8_t packet[6];

    SPI_Xfer(0xFF);

    packet[0] = cmd;
    packet[1] = (arg >> 24) & 0xFF;
    packet[2] = (arg >> 16) & 0xFF;
    packet[3] = (arg >> 8) & 0xFF;
    packet[4] = (arg) & 0xFF;
    packet[5] = crc;

    for (int i = 0; i < 6; i++)
        SPI_Xfer(packet[i]);

    for (int i = 0; i < 10; i++)
    {
        res = SPI_Xfer(0xFF);
        if (!(res & 0x80))
            break;
    }

    return res;
}

// ===== INIT SD =====
SD_Result SD_Init(void)
{
    SD_CS_HIGH();
    SPI_ClockDummy(10);

    SD_CS_LOW();

    // CMD0: reset
    if (SD_SendCmd(CMD0, 0, 0x95) != 0x01)
    {
        SD_CS_HIGH();
        return SD_ERROR;
    }

    // CMD8: check voltage (SDHC detect)
    uint8_t r7[4];
    uint8_t res = SD_SendCmd(CMD8, 0x1AA, 0x87);

    if (res == 0x01)
    {
        for (int i = 0; i < 4; i++)
            r7[i] = SPI_Xfer(0xFF);

        if (r7[2] == 0x01 && r7[3] == 0xAA)
            isSDHC = 1;
    }

    // ACMD41 init loop
    uint32_t timeout = 100000;
    do {
        SD_SendCmd(CMD55, 0, 0x65);
        res = SD_SendCmd(CMD41, 0x40000000, 0x77);
        timeout--;
    } while (res != 0x00 && timeout);

    SD_CS_HIGH();
    SPI_Xfer(0xFF);

    return (timeout) ? SD_OK : SD_ERROR;
}

// ===== READ SECTOR =====
uint8_t SD_ReadSector(uint32_t sector, uint8_t *buff)
{
    uint8_t token;

    if (!isSDHC)
        sector *= 512;

    SD_CS_LOW();

    if (SD_SendCmd(CMD17, sector, 0xFF) != 0x00)
    {
        SD_CS_HIGH();
        return 1;
    }

    // wait data token
    uint32_t timeout = 100000;
    do {
        token = SPI_Xfer(0xFF);
    } while (token == 0xFF && --timeout);

    if (token != 0xFE)
    {
        SD_CS_HIGH();
        return 1;
    }

    for (int i = 0; i < 512; i++)
        buff[i] = SPI_Xfer(0xFF);

    SPI_Xfer(0xFF); // CRC
    SPI_Xfer(0xFF);

    SD_CS_HIGH();
    SPI_Xfer(0xFF);

    return 0;
}

// ===== WRITE SECTOR =====
uint8_t SD_WriteSector(uint32_t sector, const uint8_t *buff)
{
    uint8_t resp;

    if (!isSDHC)
        sector *= 512;

    SD_CS_LOW();

    if (SD_SendCmd(CMD24, sector, 0xFF) != 0x00)
    {
        SD_CS_HIGH();
        return 1;
    }

    SPI_Xfer(0xFF);
    SPI_Xfer(0xFE); // data token

    for (int i = 0; i < 512; i++)
        SPI_Xfer(buff[i]);

    SPI_Xfer(0xFF); // CRC
    SPI_Xfer(0xFF);

    resp = SPI_Xfer(0xFF);
    if ((resp & 0x1F) != 0x05)
    {
        SD_CS_HIGH();
        return 1;
    }

    while (SPI_Xfer(0xFF) == 0x00);

    SD_CS_HIGH();
    SPI_Xfer(0xFF);

    return 0;
}
