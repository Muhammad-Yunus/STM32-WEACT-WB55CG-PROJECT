# STM32WB55 BLE → USB VCP Bridge

A demonstration project that bridges **Bluetooth Low Energy (BLE)** data to a **USB Virtual COM Port (CDC ACM)** on the **WeAct STM32WB55CGUx** (1 MB Flash) development board.

A mobile app writes data to a custom BLE GATT characteristic on the STM32WB55. The MCU receives the data and immediately forwards it to the PC through USB CDC, where it appears on a serial terminal such as PuTTY or Tera Term.

```
Phone (ST BLE Toolbox)
   │  BLE Write
   ▼
STM32WB55 ── Custom GATT Characteristic
   │  Custom_STM_Event_Handler
   ▼
CDC_Transmit_FS
   │  USB CDC (VCP)
   ▼
PC (PuTTY / Tera Term)
```

## Summary

### Tech Stack

| Layer        | Technology |
|--------------|------------|
| MCU          | STM32WB55CGUx (Cortex-M4 application CPU + Cortex-M0+ wireless CPU) |
| Toolchain    | STM32CubeIDE, STM32CubeMX |
| Framework    | STM32CubeWB (HAL + BLE stack) |
| Wireless FW  | FUS v2.2.2 + BLE Stack full firmware |
| USB          | USB Device CDC class (Virtual COM Port) |
| BLE role     | GATT server, 1 custom service, 1 writable characteristic |
| Mobile app   | ST BLE Toolbox (Android / iOS) |
| Programmer   | ST-Link + STM32CubeProgrammer |
| Terminal     | PuTTY, Tera Term, or any serial monitor |

### Project Structure

```
BLE_USB_VPC_WEACT_WB55CG/
├── Core/                       # Application code
│   ├── Inc/
│   ├── Src/
│   │   └── main.c              # Entry point, clock/peripheral init
│   └── Startup/
├── Drivers/                    # STM32WBxx HAL + CMSIS
├── Middlewares/
│   ├── ST/STM32_USB_Device_Library/   # USB CDC class driver
│   └── ST/STM32_WPAN/                # BLE stack + services
├── STM32_WPAN/
│   └── App/
│       └── custom_stm.c        # BLE GATT service — wire BLE writes to USB CDC here
├── USB_Device/
│   ├── App/
│   │   └── usbd_cdc_if.c       # USB CDC interface — CDC_Transmit_FS() lives here
│   └── Target/
├── Utilities/                  # Low-power manager, sequencer
├── Debug/                      # Build output
└── README.md
```

### Key Files

- **`Core/Src/main.c`** — System init, clock tree, peripheral bring-up, optional HSEM workaround for USB enumeration.
- **`STM32_WPAN/App/custom_stm.c`** — Custom BLE GATT service. The `Custom_STM_Event_Handler` callback receives writes from the phone.
- **`USB_Device/App/usbd_cdc_if.c`** — USB CDC interface. `CDC_Transmit_FS()` pushes bytes to the host.

---

# Build & Run — Step by Step

## Prerequisites

### Hardware
- WeAct STM32WB55CGUx board (1 MB Flash)
- USB cable (for power and VCP)
- ST-Link V2/V3 programmer

### Software
- STM32CubeIDE
- STM32CubeMX
- STM32CubeProgrammer
- ST BLE Toolbox (Android / iOS)
- PuTTY or Tera Term

---

## Step 1 — Flash the Wireless Coprocessor Firmware

The STM32WB55 has a separate Cortex-M0+ core that runs the BLE stack. Its firmware must be flashed **before** building any application, using STM32CubeProgrammer.

### 1.1 Flash the FUS (Firmware Upgrade Services)

| Field    | Value                |
|----------|----------------------|
| File     | `stm32wb5x_FUS_fw.bin` |
| Address  | `0x080EE000`         |
| Version  | FUS v2.2.2           |

### 1.2 Flash the BLE Stack

| Field    | Value                          |
|----------|--------------------------------|
| File     | `stm32wb5x_BLE_Stack_full_fw.bin` |
| Address  | `0x080D0000`                   |

### 1.3 Verify

In STM32CubeProgrammer:
- Read **FUS Information**
- Verify **FUS Version** is reported
- Verify **BLE Stack Version** is reported
- Click **Start Wireless Stack**

All four should report success. Disconnect the ST-Link from CubeProgrammer before continuing.

> The wireless binaries can be downloaded from the [STM32CubeWB repository](https://github.com/STMicroelectronics/STM32CubeWB/tree/master/Projects/STM32WB_Copro_Wireless_Binaries/STM32WB5x).

---

## Step 2 — Create the STM32CubeMX Project

### 2.1 Clock & System

| Peripheral | Setting |
|------------|---------|
| **RCC**    | Enable HSE Crystal/Ceramic Resonator (`HSE = 32 MHz` for the WeAct board) |
|            | Enable LSE Crystal/Ceramic Resonator |
| **RTC**    | Enable Internal Wakeup + RTC Interrupt |
| **SYS**    | Debug = Serial Wire |
| **HSEM**   | Enable HSEM peripheral |
| **IPCC**   | Enable IPCC RX and TX Interrupts |

### 2.2 USB

- **Connectivity → USB_FS** — enable
- **Middleware → USB Device** — enable, class = **CDC (Virtual COM Port)**

### 2.3 RF

- **Connectivity → RF** — enable, activate **RF1**

### 2.4 BLE (STM32_WPAN)

- **Middleware → STM32_WPAN** — enable
  - **BLE Application**
  - **Server Mode**
- Set:
  - `Custom P2P Server` = Disabled
  - `Custom Template` = Enabled

Enabling the Custom Template adds the BLE Advertising and GATT configuration tabs.

---

## Step 3 — Configure BLE Advertising

Open **BLE Advertising** and enable `AD_TYPE_COMPLETE_LOCAL_NAME`. Set the device name, e.g.:

```
myBLE
```

This is the name that will appear when scanning from a phone.

---

## Step 4 — Configure the GATT Service

### 4.1 Service

| Field             | Value      |
|-------------------|------------|
| Number of Services| `1`        |
| Service Long Name | `myService`|
| Service Short Name| `myService`|

### 4.2 Characteristic

| Field                       | Value     |
|-----------------------------|-----------|
| Number of Characteristics   | `1`       |
| Long Name                   | `writeChar` |
| Short Name                  | `writeChar` |

For multi-byte payloads, either increase **Value Length** or set **Characteristic Length = Variable** (recommended for variable-length packets).

### 4.3 Characteristic Properties

- Enable **WRITE** only.
- Disable the other properties.

### 4.4 GATT Events

- Enable `GATT_NOTIFY_ATTRIBUTE_WRITE`
- Disable the remaining events

Generate the project.

---

## Step 5 — Build, Flash, and Verify USB CDC

1. Build the project in STM32CubeIDE.
2. Flash it to the board.
3. Open Windows **Device Manager**.

You should see a new **USB Serial Device (COMx)** — this is *not* the ST-Link VCP.

### Troubleshooting: "Unknown USB Device"

Some WeAct boards need a manual HSEM lock before USB init. In `Core/Src/main.c`, inside `PeriphCommonClock_Config()`, immediately after:

```c
RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
```

add:

```c
LL_HSEM_1StepLock(HSEM, 5);
```

Rebuild and reflash — the CDC device should enumerate correctly.

---

## Step 6 — Open the Serial Terminal

1. Launch PuTTY (or Tera Term).
2. Connect to the COM port from Step 5.
3. Leave the terminal open.

At this stage, no data is being transmitted yet — the BLE callback hasn't been wired up.

---

## Step 7 — Connect from a Phone

1. Install **ST BLE Toolbox** on Android or iOS.
2. Scan — your device (e.g. `myBLE`) should appear.
3. Connect, then go to **Services**.
4. Three services will be listed. Open the custom one (`myService`).
5. You should see the `writeChar` characteristic with a **Write** button.

Pressing **Write** at this point does nothing — that is expected. The next step wires it up.

---

## Step 8 — Forward BLE Writes to USB CDC

Open `STM32_WPAN/App/custom_stm.c`.

### 8.1 Add the USB CDC header

In the `USER CODE BEGIN Includes` block at the top of the file, add:

```c
#include "usbd_cdc_if.h"
```

### 8.2 Forward the payload

In `Custom_STM_Event_Handler`, find the user-code section:

```c
/* USER CODE BEGIN CUSTOM_STM_Service_1_Char_1_ACI_GATT_ATTRIBUTE_MODIFIED_VSEVT_CODE */
```

and insert:

```c
CDC_Transmit_FS(
    attribute_modified->Attr_Data,
    attribute_modified->Attr_Data_Length
);
```

The result should look like:

```c
/* USER CODE BEGIN CUSTOM_STM_Service_1_Char_1_ACI_GATT_ATTRIBUTE_MODIFIED_VSEVT_CODE */

CDC_Transmit_FS(
    attribute_modified->Attr_Data,
    attribute_modified->Attr_Data_Length
);

/* USER CODE END CUSTOM_STM_Service_1_Char_1_ACI_GATT_ATTRIBUTE_MODIFIED_VSEVT_CODE */
```

Rebuild and reflash.

### 8.3 Test end-to-end

1. Reconnect the USB CDC cable.
2. Reopen PuTTY on the COM port.
3. Reconnect from ST BLE Toolbox.
4. In the `writeChar` panel, type a message (e.g. `Hello STM32`) and press **Write**.

The text appears immediately in PuTTY.

---

## Troubleshooting

### Only one character arrives per Write

The characteristic is set to a fixed 1-byte length. In CubeMX, either:
- increase **Value Length**, or
- set **Characteristic Length = Variable**

then regenerate the project.

### Phone cannot see the device

- Confirm the FUS and BLE stack are flashed and the **Start Wireless Stack** command succeeded.
- Confirm `RF1` is activated in the RF configuration.
- Confirm the device name is set under **BLE Advertising**.

### USB CDC never enumerates

- Apply the `LL_HSEM_1StepLock(HSEM, 5);` workaround in `PeriphCommonClock_Config()`.
- Check that `USB_FS` is enabled and the CDC middleware class is set to **CDC (Virtual COM Port)**.

---

## Expected Result

A full end-to-end pipe from phone to terminal:

```
Phone
  → BLE Write
    → STM32WB55
      → Custom GATT Characteristic
        → Custom_STM_Event_Handler
          → CDC_Transmit_FS
            → USB VCP
              → PuTTY
```

Type `Hello STM32` in the app, and `Hello STM32` appears in the terminal.

---

## References

- [STM32CubeWB Coprocessor Wireless Binaries](https://github.com/STMicroelectronics/STM32CubeWB/tree/master/Projects/STM32WB_Copro_Wireless_Binaries/STM32WB5x)
- [STM32WB YouTube Playlist](https://www.youtube.com/playlist?list=PLnMKNibPkDnG9JRe2fbOOpVpWY7E4WbJ-)
- [Main tutorial video](https://youtu.be/-xYoI84zJew)

---

## Acknowledgements

Built while following the STM32WB BLE tutorial series by STMicroelectronics, with additional notes and modifications for the **WeAct STM32WB55CGUx (1 MB Flash)** development board.
