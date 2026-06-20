# FATFS_SD_SPI_WEACT_WB55CG

Project STM32CubeIDE yang mengimplementasikan komunikasi **SD Card** melalui **SPI** dan filesystem **FATFS** pada board **WeAct STM32WB55CGUx**. Project ini mendemonstrasikan operasi dasar **read/write** file pada SD card dari mikrokontroler.

---

## 📋 Overview

| Item | Detail |
|------|--------|
| **Board** | WeAct STM32WB55CGUx |
| **MCU** | STM32WB55CG (ARM Cortex-M4) |
| **Framework** | STM32CubeIDE + HAL Library |
| **SD Interface** | SPI Mode |
| **Filesystem** | FatFs (R0.12c / R0.14) |
| **Debug Output** | USART1 (printf via UART) |

Project ini terdiri dari tiga komponen utama:

1. **`sd_spi.c/h`** — Low-level SD card SPI driver (CMD0, CMD8, CMD17, CMD24, ACMD41, dll)
2. **`user_diskio.c/h`** — Glue layer yang menghubungkan `sd_spi` driver ke FatFs middleware
3. **`sd_fatfs_demo.c`** — Demo aplikasi yang melakukan mount filesystem dan write/read file

---

## 🔌 Wiring (SD Card → STM32WB55CG)

Hubungkan modul SD card breakout ke board WeAct STM32WB55CGUx menggunakan jalur SPI1 berikut:

| SD Card Pin | STM32 Pin | Keterangan |
|-------------|-----------|------------|
| **SCK** | `PA5` | SPI1 Clock |
| **MOSI** | `PA6` | SPI1 Master Out Slave In (STM32 → SD) |
| **MISO** | `PA7` | SPI1 Master In Slave Out (SD → STM32) |
| **CS** | `PB0` | Chip Select (Active Low) |
| **VCC** | `3.3V` | ⚠️ Jangan gunakan 5V |
| **GND** | `GND` | Common ground |

> **Penting:**
> - Pastikan SD card menggunakan level tegangan **3.3V**. Banyak breakout SD card sudah配有 onboard LDO regulator, tetapi beberapa modul ada yang 5V — gunakan level shifter jika perlu.
> - Disarankan menambahkan **pull-up 10kΩ** pada jalur CS, MOSI, dan MISO.
> - Tambahkan **decoupling capacitor 100nF** dekat VCC pin SD card.

### Pin Configuration di CubeMX

```
PA5  → SPI1_SCK     (AF5, Very High Speed)
PA6  → SPI1_MOSI    (AF5, Very High Speed)
PA7  → SPI1_MISO    (AF5, High Speed)
PB0  → GPIO_Output  (Push-Pull, No pull, High Speed)
```

SPI1 dikonfigurasi sebagai **Master**, MSB First, dengan prescaler yang menghasilkan clock ≤ 400 kHz saat inisialisasi (bisa dinaikkan ke 10-20 MHz setelah init berhasil).

---

## 🏗️ Arsitektur Project

```
┌────────────────────────────────────────────┐
│  Application (main.c, sd_fatfs_demo.c)     │
│  - f_mount, f_open, f_write, f_read        │
└──────────────┬─────────────────────────────┘
               │
┌──────────────▼─────────────────────────────┐
│  FatFs Middleware (ff.c, diskio.c)         │
│  - f_mount, f_open, file operations        │
└──────────────┬─────────────────────────────┘
               │
┌──────────────▼─────────────────────────────┐
│  user_diskio.c (Glue Layer)                │
│  - USER_initialize, USER_read/write/ioctl  │
└──────────────┬─────────────────────────────┘
               │
┌──────────────▼─────────────────────────────┐
│  sd_spi.c (Low-level Driver)               │
│  - SD_Init, SD_ReadSector, SD_WriteSector  │
└──────────────┬─────────────────────────────┘
               │
┌──────────────▼─────────────────────────────┐
│  SPI1 HAL (stm32wbxx_hal_spi.c)            │
│  - HAL_SPI_TransmitReceive                 │
└────────────────────────────────────────────┘
```

---

## 📁 Struktur File

```
FATFS_SD_SPI_WEACT_WB55CG/
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── sd_spi.h          # SD SPI driver header
│   │   ├── sd_spi_cfg.h      # Konfigurasi pin
│   │   ├── sd_fatfs_demo.h   # Demo function prototype
│   │   └── stm32wbxx_it.h
│   └── Src/
│       ├── main.c            # Entry point, init peripherals, jalankan demo
│       ├── sd_spi.c          # Implementasi SD SPI driver
│       ├── sd_fatfs_demo.c   # Demo read/write file
│       ├── printf_uart.c     # Printf redirect ke UART
│       └── ...
├── FATFS/
│   ├── App/
│   │   ├── app_fatfs.c       # MX_FATFS_Init()
│   │   └── app_fatfs.h
│   └── Target/
│       ├── user_diskio.c     # Glue layer: FATFS ↔ sd_spi
│       ├── user_diskio.h
│       └── ffconf.h          # Konfigurasi FatFs
├── Drivers/
│   ├── CMSIS/                # ARM CMSIS core
│   └── STM32WBxx_HAL_Driver/ # HAL library
├── Middlewares/
│   └── Third_Party/FatFs/    # FatFs source
├── FATFS_SD_SPI_WEACT_WB55CG.ioc  # CubeMX config
├── STM32WB55CGUX_FLASH.ld         # Linker script (Flash)
└── README.md
```

---

## ⚙️ Konfigurasi

### SPI Configuration (CubeMX)

- **Mode**: Master
- **Direction**: Full-Duplex
- **Data Size**: 8 bits
- **Clock Polarity (CPOL)**: Low
- **Clock Phase (CPHA)**: 1 Edge
- **NSS**: Software (manual GPIO control)
- **Baud Rate Prescaler**: Sesuaikan agar ≤ 400 kHz saat init, bisa dinaikkan setelahnya

### FATFS Configuration (`ffconf.h`)

Pastikan konfigurasi berikut **enabled** untuk mendukung read/write:

```c
#define _FS_READONLY   0
#define _USE_WRITE     1
#define _USE_IOCTL     1
```

---

## 🚀 Cara Menjalankan

1. **Buka project** di STM32CubeIDE (versi 1.18.1 atau yang kompatibel)
2. **Hubungkan** board WeAct STM32WB55CGUx via ST-Link
3. **Periksa wiring** SD card sesuai tabel di atas
4. **Build** project (Ctrl+B)
5. **Flash** ke board (Run → Debug, atau Run → Run)
6. **Buka serial monitor** (115200 baud, 8N1) — bisa menggunakan PuTTY, Tera Term, atau STM32CubeIDE built-in
7. Amati output demo pada terminal

---

## 📤 Output Demo

Setelah program di-flash dan dijalankan, output berikut akan muncul di serial monitor:

```
==== READ/WRITE DEMO ====
SD + SPI + FATFS
Write bytes: 21
Read bytes: 21
Content:
HELLO STM32 SD SPI

==== DONE ====
```

### Penjelasan Output:

- **`==== READ/WRITE DEMO ====`** — Header demo dimulai
- **`SD + SPI + FATFS`** — Indikasi stack yang digunakan
- **`Write bytes: 21`** — Jumlah byte yang berhasil ditulis ke `test.txt` (21 bytes = panjang string "HELLO STM32 SD SPI\r\n")
- **`Read bytes: 21`** — Jumlah byte yang berhasil dibaca kembali
- **`Content:`** — Konten file yang dibaca
- **`HELLO STM32 SD SPI`** — Isi file `test.txt` yang ditulis dan dibaca kembali
- **`==== DONE ====`** — Demo selesai

---

## 🔧 Penjelasan Kode

### 1. SD SPI Driver (`sd_spi.c`)

Driver ini mengimplementasikan protokol SD card dalam SPI mode:

- **`SD_Init()`** — Inisialisasi SD card:
  - Mengirim ≥74 clock dummy
  - CMD0 (reset ke idle state)
  - CMD8 (deteksi voltase & SDHC)
  - ACMD41 loop (inisialisasi dan tunggu card ready)
  - Mendukung **SD v1**, **SD v2**, dan **SDHC**

- **`SD_ReadSector(sector, buff)`** — Baca 1 sektor (512 bytes):
  - Menggunakan CMD17 untuk single block read
  - Otomatis mengkonversi LBA → byte address untuk non-SDHC

- **`SD_WriteSector(sector, buff)`** — Tulis 1 sektor (512 bytes):
  - Menggunakan CMD24 untuk single block write
  - Menunggu data response token (0x05) dan busy state

### 2. User Disk I/O (`user_diskio.c`)

File ini adalah **glue layer** antara FatFs middleware dan low-level driver. FatFs memanggil fungsi-fungsi ini untuk semua operasi I/O disk:

| Fungsi | Tujuan |
|--------|--------|
| `USER_initialize` | Mount/mounting disk (calls `SD_Init()`) |
| `USER_status` | Cek status disk |
| `USER_read` | Baca sektor (calls `SD_ReadSector()`) |
| `USER_write` | Tulis sektor (calls `SD_WriteSector()`) |
| `USER_ioctl` | Dapatkan info disk (sector size, count) |

### 3. Demo Application (`sd_fatfs_demo.c`)

Fungsi `SD_FATFS_ReadWriteDemo()` melakukan:

1. **Mount** filesystem (`f_mount`)
2. **Buka** file `test.txt` dengan mode write-create (`f_open`)
3. **Tulis** string `"HELLO STM32 SD SPI\r\n"` (21 bytes)
4. **Tutup** file (`f_close`)
5. **Buka kembali** file dalam mode read (`f_open`)
6. **Baca** isi file ke buffer (`f_read`)
7. **Print** konten ke UART

---

## 🧪 Testing

Untuk menguji bahwa SD card Anda kompatibel:

- ✅ SD card harus diformat dengan **FAT12, FAT16, atau FAT32**
- ✅ Gunakan SD card dengan kapasitas ≤ 32GB untuk FAT16, atau > 32GB untuk FAT32/exFAT
- ⚠️ Jika `Mount failed`, coba format ulang SD card dengan **SD Card Formatter** (https://www.sdcard.org/downloads/formatter/)
- ⚠️ Pastikan SD card terpasang dengan benar dan tidak longgar

---

## 🐛 Troubleshooting

| Masalah | Solusi |
|---------|--------|
| `Mount failed: 3` | SD card belum diformat atau format tidak sesuai. Format dengan FAT32. |
| `Mount failed: 1` | SD card tidak terdeteksi. Periksa wiring CS, VCC, dan ground. |
| Tidak ada output di UART | Periksa baud rate (115200), TX/RX pin, dan koneksi serial. |
| Write berhasil, read gagal | Coba turunkan SPI clock. Beberapa SD card tidak stabil di kecepatan tinggi. |
| Hard fault saat init | Pastikan stack size cukup besar (min 4KB) dan heap tersedia untuk FatFs. |

---

## 📚 Referensi

- [FatFs Documentation](http://elm-chan.org/fsw/ff/00index_e.html) — Generic FAT Filesystem Module
- [STM32WB55 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0434-stm32wb55xx-armbased-32bit-mcus-stmicroelectronics.pdf)
- [SD Card Specification](https://www.sdcard.org/downloads/pls/) — Physical Layer Simplified Specification
- [WeAct STM32WB55CGUx Schematic](https://github.com/WeActStudio/WeActStudio.STM32WB55) — Board reference

---

## 📝 Lisensi

Project ini menggunakan komponen open-source:
- **STM32 HAL Drivers** — BSD License (STMicroelectronics)
- **FatFs** — Custom license (ChaN) — free for commercial/non-commercial use
- **CMSIS** — Apache 2.0 License (ARM)

