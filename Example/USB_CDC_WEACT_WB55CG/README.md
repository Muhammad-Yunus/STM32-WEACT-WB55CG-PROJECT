# USB CDC Echo Demo — WEACT STM32WB55CGU6

Demo proyek ini menunjukkan komunikasi **USB Virtual COM Port (CDC)** pada chip
**STM32WB55CGU6** (WEACT Studio board) tanpa melibatkan UART fisik — seluruh
input dan output berjalan melalui koneksi USB ke PC.

## Hardware

| Item                | Detail                                     |
| ------------------- | ------------------------------------------ |
| MCU                 | STM32WB55CGU6 (ARM Cortex-M4 @ 64 MHz)     |
| Board               | WEACT Studio STM32WB55CGU6                 |
| USB                 | FS USB (Full-Speed, 12 Mbps)               |
| Debugger/Programmer | ST-Link/V2-1 (built-in on WEACT board)     |
| Crystal             | HSE 8 MHz + HSI48 for USB FS clock         |

## Arsitektur

```
┌───────────────────────────────────────────────────┐
│ PC — Terminal (PuTTY / Screen / Arduino Serial)   │
│   "hello\r\n"  ──USB──►                           │
└───────────────────────────────────────────────────┘
                        ▲
                        │ USB CDC (Virtual COM)
                        ▼
┌───────────────────────────────────────────────────┐
│ WEACT STM32WB55CGU6                               │
│                                                   │
│  USB Device Stack (STM32 CubeMX / HAL)            │
│  ┌───────────────────┐                            │
│  │ usbd_cdc_if.c     │  ← CDC_Receive_FS()        │
│  │   CDC_ProcessPacket()  push into ring buffer   │
│  └────────┬──────────┘                            │
│           │                                       │
│  ┌────────▼──────────┐                            │
│  │ cdc_uart.c        │  ← ring-buffer + ReadLine  │
│  │                   │     + Printf + TX blocking │
│  └────────┬──────────┘                            │
│           │                                       │ 
│  ┌────────▼──────────┐                            │
│  │ main.c            │  ← while(1) ReadLine/Printf│
│  │   "Echo> hello"   │                            │
│  └───────────────────┘                            │
└───────────────────────────────────────────────────┘
```

### Layer Files

| File                    | Role                                         |
| ----------------------- | -------------------------------------------- |
| `Core/Src/main.c`       | Application loop — ReadLine → print echo     |
| `Core/Src/cdc_uart.c`   | Ring-buffer, CDC_ReadLine, CDC_Printf        |
| `Core/Inc/cdc_uart.h`   | API declarations, `CDC_HandleTypeDef` struct |
| `USB_Device/App/usbd_cdc_if.c` | USB CDC media-low-layer (CubeMX gen) |
| `USB_Device/App/usbd_desc.c`    | USB descriptors (device + config)     |

## Ring Buffer

- Ukuran: **1024 byte** (power-of-two, menggunakan bitmask untuk performa)
- `CDC_ProcessPacket()`: USB callback menulis ke buffer (drop jika penuh)
- `CDC_ReadLine()`: membaca hingga menemukan `\r` atau `\n`
- Mendukung blok ukuran buffer hingga 2048 byte (konfigurasi USB FS MTU)

## Build & Flash

### Environment
- **STM32CubeIDE** 1.18.1
- **Compiler**: ARM GCC 13.3 (GNU Arm Embedded Toolchain)
- **Build Config**: Debug

### Build
```bash
cd workspace_1.18.1/USB_CDC_WEACT_WB55CG
make -j$(nproc) CONFIGURATION=Debug
```

Atau buka di STM32CubeIDE dan tekan **Ctrl+B**.

### Flash
1. Hubungkan board WEACT STM32WB55CGU6 via USB (ST-Link port)
2. Flash dengan STM32CubeIDE: **Right-click → Debug As**
3. Atau gunakan CLI:
   ```
   openocd -f interface/stlink.cfg -f target/stm32wbx.cfg -c "program Debug/USB_CDC_WEACT_WB55CG.elf verify reset exit"
   ```

## Cara Penggunaan

### 1. Hubungkan USB
Colok board WEACT ke PC via **USB OTG / USB FS**. Windows akan mendeteksi
dan menginstall driver CDC (jika driver ST-Link belum terinstall, board tetap
dikenali sebagai USB CDC).

### 2. Buka Terminal
Gunakan salah satu:

| Platform | Tool           | Command                                      |
| -------- | -------------- | -------------------------------------------- |
| Windows  | PuTTY          | Port: COMx (mis. COM3), Baud: 115200         |
| Linux    | screen         | `screen /dev/ttyACM0 115200`                  |
| macOS    | screen         | `screen /dev/cu.usbmodemXXXX 115200`         |
| Cross    | minicom        | `minicom -D /dev/ttyACM0 -b 115200`          |

**Baud Rate: 115200** — meskipun tidak ada UART fisik, descriptor USB CDC
diset ke 115200 bps.

### 3. Test Echo (Ping-Pong)
Di terminal, ketik sesuatu dan tekan **Enter**:
```
hello
Echo> hello

test CDC
Echo> test CDC
```

Setiap baris yang dikirim akan di-"pong" balik oleh firmware dengan prefix
`Echo> `.

### 4. Test Input Panjang
Coba kirim teks panjang (lebih dari 1024 byte):
```bash
python3 -c "print('A' * 2000)" | /dev/ttyACM0   # Linux/macOS
```
Buffer akan drop byte berlebih — perilaku ini sengaja untuk testing.

## Struktur Kode Utama

### main.c
Loop utama sangat sederhana:
```c
while (1) {
    if (CDC_ReadLine(&hcdc, line_buf, sizeof(line_buf)) == CDC_OK) {
        CDC_Printf(&hcdc, "Echo> %s\r\n", line_buf);
    }
    HAL_Delay(5);
}
```

### cdc_uart.c
API utama yang menyediakan:

| Fungsi                     | Deskripsi                                     |
|---------------------------|----------------------------------------------|
| `CDC_Init(hcdc)`          | Init ring buffer + prime USB RX callback     |
| `CDC_ProcessPacket(buf,n)`| Push bytes dari USB callback ke ring buffer  |
| `CDC_ReadLine(hcdc,buf,n)`| Block until \r/\n found (strip terminator)   |
| `CDC_Printf(hcdc,fmt,...)`| Formatted output, blocks until TX idle       |
| `CDC_TransmitBlocking_FS()`| Wait for TX idle before sending             |

## Troubleshooting

### Problem: Board tidak terdeteksi sebagai USB Device
- Pastikan kabel USB terhubung ke port **USB FS** (bukan ST-Link port saja)
- Pada WEACT board, port USB FS biasanya ditandai dengan ikon USB
- Periksa di Device Manager (Windows) atau `dmesg` (Linux)

### Problem: Tidak ada output echo
- Pastikan baud rate terminal = 115200
- Coba disconnect/reconnect USB cable
- Reset board dengan menekan tombol RESET

### Problem: Terminal menampilkan karakter acak/gibberish
- Cek baud rate (115200)
- Pastikan HSE crystal 8 MHz terpasang di board
- Coba build ulang dengan konfigurasi Release

### Problem: Build gagal — "storage size of hcdc isn't known"
- Pastikan `cdc_uart.h` menyertakan definisi `CDC_HandleTypeDef` lengkap
- Versi terbaru sudah memperbaiki masalah ini

## Ukuran Firmware

| Section | Size      |
| ------- | --------- |
| `.text` | ~38 KB    |
| `.data` | ~336 B    |
| `.bss`  | ~11 KB    |

## License

This project uses STM32CubeMX-generated code (STMicroelectronics license).
Custom additions (`cdc_uart.c`, `cdc_uart.h`, `main.c` USER CODE) are MIT licensed.

## Board Reference

- [WEACT Studio STM32WB55CGU6](https://github.com/WeActStudio/WeActStudio.STM32WB55CoreBoard)
- STM32WB55 Reference Manual: RM0481
