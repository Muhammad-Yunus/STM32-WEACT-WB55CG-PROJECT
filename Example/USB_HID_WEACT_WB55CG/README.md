# USB HID Mouse with Rotary Encoder (WEACT STM32WB55CG)

STM32CubeIDE project for the WEACT STM32WB55CGUx board implementing a USB HID Mouse device controlled by a rotary encoder.

## Hardware Overview

| Component | Pin | Function |
|-----------|-----|----------|
| Encoder A | PB0 | Clock pulse (EXTI0 interrupt, rising edge) |
| Encoder B | PB1 | Data pulse (input with pull-up) |
| Encoder SW | PE4 | Switch button (EXTI4 interrupt, rising edge) |
| UART1 | USART1 | Debug output (115200 baud) |

## Features

- **USB Full-Speed HID Mouse** — class-compliant, no driver installation needed on Windows, macOS, or Linux
- **Rotary Encoder Scroll Control** — clockwise rotation sends scroll up (+1), counter-clockwise sends scroll down (-1)
- **Encoder Button = Left Click** — pressing the encoder switch triggers a mouse left-click event
- **Interrupt-driven encoder decoding** — low CPU overhead, direction detected via EXTI callbacks on PB0
- **Button debounced in software** — state tracked via EXTI4 interrupt on PE4

## Hardware Connection

```
Encoder Pin A  ---> PB0 (STM32WB55CG)
Encoder Pin B  ---> PB1 (STM32WB55CG)
Encoder Pin SW ---> PE4 (STM32WB55CG)
Encoder Pin GND ---> GND
Encoder Pin VCC ---> 3.3V
```

## Architecture

```
+------------------+         +----------------+         +--------------+
|  Rotary Encoder  |  EXTI   |  STM32WB55CG   |  USB FS |   PC / Host  |
|  PB0 (CLK)       | ------->|  STM32CubeIDE  | ------> |  (HID Mouse) |
|  PB1 (DT)        |  Input  |  HAL + CDC     |         |              |
|  PE4 (SW)        | ------->|                |         |              |
+------------------+         +----------------+         +--------------+
                                      |
                                      v
                             +----------------+
                             |  HID Report    |
                             |  [button, x, y,|
                             |   wheel]       |
                             +----------------+
```

### Source Files

| File | Description |
|------|-------------|
| `Core/Src/main.c` | Application entry point, main loop, GPIO/UART initialization |
| `Core/Src/encoder.c` | Rotary encoder direction detection via EXTI interrupt |
| `Core/Inc/encoder.h` | Encoder types and function declarations |
| `Core/Src/usb_mouse.c` | USB HID mouse report sending (move, wheel, click) |
| `Core/Inc/usb_mouse.h` | Mouse function declarations |
| `USB_Device/App/usbd_desc.c` | USB device descriptors (VID: 0x0483, PID: 0x5740) |
| `USB_Device/App/usb_device.c` | USB device lifecycle management |

## Clock Configuration

- **HSE** + PLL -> SYSCLK at 64 MHz
- **HSI48** enabled for USB clock (48 MHz nominal)
- **APB1/APB2** at 64 MHz, **AHB** at 64 MHz
- **SMPS** powered by HSI

## USB Descriptor

| Field | Value |
|-------|-------|
| Vendor ID (VID) | 0x0483 (STMicroelectronics) |
| Product ID (PID) | 0x5740 (22315 decimal) |
| Product String | "STM32 Human interface" |
| Manufacturer | STMicroelectronics |
| BCD USB | 2.00 |
| Speed | Full-Speed (12 Mbps) |
| Class | 0x00 (Defined per interface) |
| Interface Class | 0x03 (HID) |
| Endpoint | EP1 IN (interrupt, 4 bytes) |

## How It Works

### Encoder Direction Detection

The encoder A pin (PB0) triggers an EXTI interrupt on rising edge. The callback reads the B pin state:
- **B = HIGH** -> Counter-clockwise rotation -> scroll down (-1)
- **B = LOW** -> Clockwise rotation -> scroll up (+1)

### Mouse Report

A standard 4-byte HID mouse report is sent each cycle:

| Byte | Content |
|------|---------|
| 0 | Button state (0x01 = left pressed, 0x00 = released) |
| 1 | X movement (signed, not used) |
| 2 | Y movement (signed, not used) |
| 3 | Wheel movement (signed, +1 or -1) |

After each report, an empty report `[0, 0, 0, 0]` is sent to release the button state.

### Left Click

Pressing the encoder switch (PE4) triggers an EXTI interrupt which sets a flag. In the main loop, `Encoder_ButtonPressed()` checks and clears the flag, then sends a click sequence: left-down -> 10ms delay -> left-up.

## Build & Flash

1. Open the project in **STM32CubeIDE**
2. Select the **Debug** configuration
3. Connect your WEACT STM32WB55CG board via SWD (PA13/PA14) or USB-C power
4. Build the project (Ctrl+B)
5. Flash and run (Run / F11)

## Debug Output

UART1 at 115200 baud, 8N1. On startup it prints:

```
==== DEMO USB HID - MOUSE CONTROL ====
```

## Platform

- **IDE:** STM32CubeIDE 1.18.1
- **MCU:** STM32WB55CGUx (ARM Cortex-M4F + Cortex-M0+)
- **Board:** WEACT STM32WB55CG Min/Dev Board
- **Framework:** STM32 HAL + STM32 USB Device Library (HID class)

