# STM32WB55 ADC to UART Example

A simple STM32WB55 project demonstrating how to read an analog voltage using **ADC1** and print the converted value through **USART1**.

## Features

- ADC1 single-channel conversion
- 12-bit ADC resolution
- Software-triggered ADC conversion
- UART output using `printf()`
- Periodic sampling every 10 ms

---

## Hardware

### MCU

- STM32WB55CGUx

### Pin Assignment

| Peripheral | Pin | Function |
|------------|-----|----------|
| ADC1 | PA0 | Analog Input (ADC1_IN5) |
| USART1_TX | PB6 | UART Transmit |
| USART1_RX | PB7 | UART Receive |

### Potentiometer Connection

| Potentiometer | STM32 |
|---------------|--------|
| VCC | 3.3 V |
| GND | GND |
| OUT | PA0 |

---

## Project Structure

```text
.
├── Core
│   ├── Inc
│   │   └── main.h
│   └── Src
│       ├── main.c
│       ├── stm32wbxx_hal_msp.c
│       ├── stm32wbxx_it.c
│       └── ...
├── Drivers
│   ├── CMSIS
│   └── STM32WBxx_HAL_Driver
└── STM32CubeIDE project files
```

### Main Components

- **main.c**
  - System initialization
  - Peripheral initialization
  - Main application loop
  - ADC sampling and UART output

- **stm32wbxx_hal_msp.c**
  - GPIO configuration
  - ADC peripheral initialization
  - USART1 peripheral initialization

- **Drivers/**
  - STM32 HAL drivers
  - CMSIS core support

---

## ADC Sampling Flow

Each iteration performs one ADC conversion and transmits the result over UART.

```c
HAL_ADC_Start(&hadc1);

if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
{
    uint32_t adc = HAL_ADC_GetValue(&hadc1);
    printf("%lu\r\n", adc);
}

HAL_ADC_Stop(&hadc1);

HAL_Delay(10);
```

### Flow Description

1. Start ADC conversion.
2. Wait until the conversion completes.
3. Read the ADC conversion result.
4. Print the value through USART1.
5. Stop the ADC.
6. Wait for 10 ms before the next conversion.

---

## ADC Characteristics

- Resolution: **12-bit**
- Conversion mode: **Single conversion**
- Trigger: **Software**
- Input channel: **ADC1_IN5 (PA0)**

Typical conversion values:

| Input Voltage | ADC Value |
|---------------|----------:|
| 0.0 V | 0 |
| 1.65 V | ~2048 |
| 3.3 V | ~4095 |

---

## UART Configuration

- Peripheral: USART1
- TX Pin: PB6
- RX Pin: PB7
- Baud Rate: 115200
- Data Bits: 8
- Parity: None
- Stop Bits: 1

Example serial output:

```text
0
153
721
1842
3078
4095
```

---

## Requirements

- STM32CubeIDE
- STM32 HAL Driver
- STM32WB55CGUx
- USB-to-UART adapter (3.3 V)
- Potentiometer or another analog voltage source

---

## License

This project is provided as a simple reference for ADC and UART communication using the STM32 HAL library.