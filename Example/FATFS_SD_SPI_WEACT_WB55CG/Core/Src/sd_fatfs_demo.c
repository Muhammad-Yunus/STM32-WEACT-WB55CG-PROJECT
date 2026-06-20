/*
 * sd_fatfs_demo.c
 *
 *  Created on: Jun 20, 2026
 *      Author: Asus
 */


#include "ff.h"
#include <stdio.h>

void SD_FATFS_ReadWriteDemo(void)
{
    FATFS fs;
    FIL fil;
    FIL fil2;
    UINT bw, br;

    char buffer[64] = {0};

    // 1. Mount filesystem
    FRESULT res = f_mount(&fs, "", 1);
    if (res != FR_OK)
    {
        printf("Mount failed: %d\r\n", res);
        return;
    }

    // 2. WRITE FILE
    res = f_open(&fil, "test.txt", FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK)
    {
        printf("Open write failed: %d\r\n", res);
        return;
    }

    res = f_write(&fil, "HELLO STM32 SD SPI\r\n", 21, &bw);
    printf("Write bytes: %u\r\n", bw);

    f_close(&fil);

    // 3. READ FILE BACK
    res = f_open(&fil2, "test.txt", FA_READ);
    if (res != FR_OK)
    {
        printf("Open read failed: %d\r\n", res);
        return;
    }

    res = f_read(&fil2, buffer, sizeof(buffer) - 1, &br);
    if (res == FR_OK)
    {
        buffer[br] = '\0'; // null terminate
        printf("Read bytes: %u\r\n", br);
        printf("Content:\r\n%s\r\n", buffer);
    }
    else
    {
        printf("Read failed: %d\r\n", res);
    }

    f_close(&fil2);
}
