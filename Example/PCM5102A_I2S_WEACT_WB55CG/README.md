# STM32WB55CGUx + PCM5102A I2S Audio Demo

A simple STM32CubeIDE project demonstrating audio playback using the **WeAct STM32WB55CGUx** development board and a **PCM5102A I2S DAC**.

The project generates a continuous sine wave in software and transmits it to the PCM5102A using the STM32 SAI peripheral in I2S mode with DMA.

---

## Hardware

### MCU Board

- WeAct STM32WB55CGUx
- STM32WB55CGU6 MCU
- STM32CubeIDE
- STM32 HAL Driver

### Audio DAC

- PCM5102A I2S DAC Module (Purple PCB)

---

## Features

- SAI configured as I2S Master Transmitter
- DMA continuous audio streaming
- Stereo output
- 32-bit audio samples
- 16 kHz sample rate
- Software-generated sine wave
- Adjustable digital volume

---

## Hardware Connection

| STM32 Pin | SAI Signal | PCM5102A Pin |
|------------|------------|--------------|
| PA8  | SAI1_SCK_A (BCLK) | BCK |
| PB9  | SAI1_FS_A (LRCK / WS) | LCK |
| PA10 | SAI1_SD_A (DATA) | DIN |
| MCLK | SAI1_MCLK_A | SCK (MCLK) |
| 3.3V | Power | VCC |
| GND | Ground | GND |

> **Note**
>
> - **PA8** → Bit Clock (BCLK)
> - **PB9** → Left/Right Clock (LRCK / Word Select)
> - **PA10** → Serial Audio Data (SD)
> - **MCLK** must also be connected to the PCM5102A **SCK** input when the module is configured for an external master clock. :contentReference[oaicite:0]{index=0}


---

## PCM5102A Jumper Configuration

The tested configuration is:

| Jumper | Setting |
|----------|---------|
| H1L | LOW |
| H2L | LOW |
| H3L | HIGH |
| H4L | LOW |

This configuration enables:

- I2S format
- External Master Clock (MCLK)
- Normal operation

---

## STM32CubeMX Configuration

### SAI

```
Mode               : Master Transmit
Protocol           : I2S Standard
Data Size          : 32-bit
Channels           : Stereo
Audio Frequency    : 16 kHz
FIFO               : Half Full
Output Drive       : Enable
```

### DMA

```
Mode        : Circular
Direction   : Memory -> Peripheral
Priority    : High
```

---

## Audio Format

```
Sample Rate : 16000 Hz

Channels    : 2 (Stereo)

Sample Size : 32-bit signed

Format

Left Sample
Right Sample
Left Sample
Right Sample
...
```

---

## Example DMA Buffer

```c
  uint8_t volume = 5;      // 0..100
  const int32_t max_amp = 0x60000000;

  int32_t amplitude = (max_amp * volume) / 100;
  int32_t tx[DMA_SAMPLES];

  for (int i = 0; i < SAMPLES_PER_PERIOD; i++)
  {
      int32_t sample = ((i / SAMPLES_PER_HALF) & 1) ?
    		  	  	  	  amplitude :
						  -amplitude;

      tx[i * AUDIO_CHANNELS + 0] = sample;   // Left
      tx[i * AUDIO_CHANNELS + 1] = sample;   // Right
  }

  HAL_SAI_Transmit_DMA(
      &hsai_BlockA1,
      (uint8_t *)tx,
      DMA_SAMPLES
  );
```

---

---

## Project Structure

```
Core/
 ├── Inc/
 │     main.h
 │
 ├── Src/
 │     main.c
 │     stm32wbxx_hal_msp.c
 │
 ├── Startup/
 │
Drivers/
```

---

## Build Environment

- STM32CubeIDE
- STM32CubeMX
- STM32 HAL
- GCC ARM Embedded

---

## Tested Hardware

- WeAct STM32WB55CGUx
- PCM5102A Purple Module