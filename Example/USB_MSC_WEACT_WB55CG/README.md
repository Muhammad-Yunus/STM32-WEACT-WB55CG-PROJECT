# USB Mass Storage Device - STM32WB55CG + SD Card via SPI

STM32CubeIDE project implementing a USB Mass Storage Class (MSC) device that exposes an SD card connected via SPI as a removable USB storage drive on Windows/Linux/macOS.

## Hardware

- **MCU**: STM32WB55CGU6 (STM32WB Series, ARM Cortex-M4 @ 64 MHz)
- **Board**: WEAct Studio STM32WB55CG Module
- **Package**: UFQFPN48
- **Crystal**: 32 MHz HSE

## Pin Mapping

### SPI1 - SD Card Interface

| Pin    | Peripheral      | Function         |
|--------|----------------|------------------|
| PA5    | SPI1           | SCK              |
| PA6    | SPI1           | MISO             |
| PA7    | SPI1           | MOSI             |
| PB0    | GPIO (Output)  | SD CS (Chip Select) |

### USB OTG FS - USB Device

| Pin    | Peripheral | Function |
|--------|-----------|----------|
| PA11   | USB       | DM       |
| PA12   | USB       | DP       |

### USART1 - Debug/Logging

| Pin    | Peripheral | Baud Rate | Configuration     |
|--------|-----------|-----------|-------------------|
| PA9    | USART1 TX | 115200    | 8N, Async         |
| PA10   | USART1 RX | 115200    | 8N, Async         |

### Other

- **USB VDD**: Auto-enabled via `HAL_PWREx_EnableVddUSB()` in MSP init
- **System Clock**: 64 MHz from HSE -> PLL (HSE 32MHz, PLLM=2, PLLN=8, PLLP=2)
- **USB Clock**: 48 MHz from PLLSAI1 (PLLSAI1N=6, PLLR=2)

## Architecture

```
[SD Card] --SPI1--> [STM32WB55CG] --USB FS MSC--> [PC / Host]
                          ||
                      [USART1]
                          ||
                    [Debug Logs @ 115200]
```

### Data Flow

1. **SD Card** is initialized via SPI protocol (CMD0, CMD8, CMD55/ACMD41, CMD9)
2. Sector read/write operations go through `sd_spi.c` which handles the raw SPI SD command protocol
3. **USB MSC layer** (`usbd_storage_if.c`) bridges host USB SCSI commands to SD card sector operations
4. **Logger** (`logger.h`) provides UART debug output with configurable log levels (INFO / DEBUG)

## Supported SD Card Types

| Type     | Addressing       | Max Capacity     | Status    |
|----------|-----------------|------------------|-----------|
| SDSC     | Block addressing (sector * 512) | 2 GB       | Supported |
| SDHC     | Byte addressing (LBA)           | 4 GB - 32 GB   | Supported |
| SDXC     | Byte addressing (LBA)           | > 32 GB        | Not tested |

## Filesystem Notes - Important

### Tested Filesystems

- **FAT (non-FAT32)** with 2 GB SD card -- **WORKING**
  - SD card is recognized correctly by Windows
  - Full read/write operations through USB MSC

- **FAT32** with > 4 GB SD card -- **NOT WORKING**
  - Volume is **not detected** by Windows
  - The `STORAGE_BLK_NBR` in `usbd_storage_if.c` is hardcoded to `0x10000` (65536 blocks), which only covers ~32 GB at 512 bytes/block. For cards > 4 GB formatted as FAT32, the actual sector count from the CSD register may exceed this or the boot sector write in `SD_WipeBootSectors()` may not produce a filesystem Windows accepts for larger cards.

### Why FAT 2 GB Works But FAT32 > 4 GB Does Not

The `STORAGE_BLK_NBR` constant (`0x10000` = 65536) is a fixed value that doesn't match the actual SD card sector count for larger cards. The real sector count is dynamically read from the card's CSD register via `SD_GetSectorCount()`, but the USB MSC capacity report uses the hardcoded value. For a 2 GB SDSC card this happens to work within range, but for larger SDHC cards the mismatch causes Windows to reject the volume.

**Fix**: Update `STORAGE_BLK_NBR` to use the dynamic sector count from `SD_GetSectorCount()` in `STORAGE_GetCapacity_FS()` instead of the hardcoded value.

### Recommended Approach

For production use, format the SD card on the target device or use a proper FAT filesystem library (FatFs) mounted on the SD card first, then present it via USB MSC. The current implementation writes a minimal boot sector for FAT32 via `SD_WipeBootSectors()`, but Windows needs the full FAT tables and directory structure to recognize the volume.

## Project Structure

```
USB_MSC_WEACT_WB55CG/
  Core/
    Inc/
      logger.h          # Unified logger (INFO/DEBUG levels via UART)
      main.h            # Pin definitions (SD_CS on PB0)
      sd_spi.h          # SD card SPI driver header
      stm32wbxx_it.h    # Interrupt handlers
    Src/
      main.c            # Application entry, clock/GPIO/SPI/UART init
      sd_spi.c          # Raw SD card SPI driver + FAT32 boot sector writer
      stm32wbxx_hal_msp.c # HAL MSP callbacks (SPI, UART GPIO init)
      stm32wbxx_it.c    # IRQ handlers
  USB_Device/
    App/
      usbd_desc.c       # USB device descriptors (VID:PID = 0x0483:0x5740)
      usbd_storage_if.c # MSC storage interface (bridges USB <-> SD card)
    Target/
      usbd_conf.c       # USB PCD low-level driver
      usbd_conf.h       # USB configuration defines
  Drivers/              # STM32Cube HAL drivers (generated)
  Middlewares/          # Third-party libraries
  Debug/                # Build output
```

## Key Source Files

| File | Purpose |
|------|---------|
| `Core/Src/sd_spi.c` | SD card SPI initialization, CMD protocol, sector R/W, CSD parsing, FAT32 boot sector writer |
| `Core/Inc/sd_spi.h` | Public API: `SD_Init()`, `SD_ReadSector()`, `SD_WriteSector()`, `SD_GetSectorCount()` |
| `USB_Device/App/usbd_storage_if.c` | USB MSC storage callbacks bridging host requests to SD card driver |
| `Core/Src/main.c` | System clock, SPI1, UART1, GPIO initialization; USB and main loop |
| `Core/Inc/logger.h` | Configurable UART logger with `LOG_I()` and `LOG_D()` macros |

## SPI Configuration

- **Mode**: Master, Full Duplex
- **Data Size**: 8 bits
- **Clock Polarity/Phase**: CPOL=Low, CPHA=1 Edge
- **Baud Rate Prescaler**: 128 (actual SPI clock ~500 kHz, within SD spec)
- **First Bit**: MSB
- **NSS**: Software management

## Build

- **IDE**: STM32CubeIDE
- **Firmware Package**: STM32Cube FW_WB V1.24.0
- **Compiler**: GCC (Optimization: O6)
- **Linker Script**: `STM32WB55CGUX_FLASH.ld`

## Debug

Connect a USB-to-TSerial adapter to PA9 (TX) / PA10 (RX) at 115200 baud, 8N1. The logger outputs:

- `[SD]` - SD card init status, CSD version, sector count, voltage detection
- `[MSC]` - USB MSC lifecycle events (init, capacity requests, read/write operations)
- `[FS]` - Filesystem/boot sector write status

Log levels are controlled in `logger.h`:
- `LOG_LEVEL_INFO` - Shows INFO messages only
- `LOG_LEVEL_DEBUG` - Shows INFO + DEBUG messages

## USB Device Info

- **Speed**: Full Speed (12 Mbps)
- **Class**: Mass Storage Class (MSC)
- **VID**: 0x0483 (STMicroelectronics)
- **PID**: 0x5740 (22314 decimal)
- **Product String**: "STM32 Mass Storage"
- **Endpoints**: EP0 (Control), EP1 (IN - MSC), EP2 (OUT - MSC), EP3 (IN - Interrupt)
