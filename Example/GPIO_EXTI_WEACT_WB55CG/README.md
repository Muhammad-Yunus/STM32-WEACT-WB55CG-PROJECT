# STM32WB55 Core Board - Blink LED & External Interrupt (EXTI)

Proyek ini mendemonstrasikan implementasi sistem *embedded* dasar menggunakan papan pengembang **WeAct STM32WB55CGUx** Core Board V10. Proyek ini mengintegrasikan konfigurasi clock eksternal (LSE & HSE), kendali output GPIO untuk LED onboard, serta penanganan *External Interrupt* (EXTI) menggunakan tombol tekan (*push button*) eksternal.

## 🚀 Fitur Proyek
1. **Dual External Crystal Oscillator**: Mengaktifkan HSE (32 MHz) untuk subsistem utama/RF dan LSE (32.768 kHz) sebagai sumber *clock* presisi tinggi untuk RTC.
2. **Non-blocking LED Blink**: Mengendalikan kedipan (*blink*) LED onboard pada pin **PE4** menggunakan metode *tick-counting* (`HAL_GetTick()`) agar sistem tetap responsif.
3. **Interrupt Driven Delay Controller**: Menggunakan tombol *Active High* pada pin **PB7** melalui jalur **EXTI7** untuk memotong (*override*) dan mengurangi durasi jeda kedipan LED secara instan.

---

## 🛠️ Konfigurasi Periferal (STM32CubeMX)

### 1. System Core & Clock Configuration
* **HSE (High Speed External)**: Diatur ke `Crystal/Ceramic Resonator` dengan frekuensi input **32 MHz**.
* **LSE (Low Speed External)**: Diatur ke `Crystal/Ceramic Resonator` dengan frekuensi input **32.768 kHz**.
* **RTC Clock Source Mux**: Diarahkan untuk menggunakan **LSE** sebagai penyedia *clock* utama RTC.

### 2. GPIO Konfigurasi
| Pin | Nama Periferal | Mode | Konfigurasi Tambahan | Deskripsi |
| :--- | :--- | :--- | :--- | :--- |
| **PE4** | GPIO_Output | Output Push-Pull | No Pull-up/Pull-down | LED Onboard |
| **PB7** | GPIO_EXTI7 | External Interrupt Mode with Rising Edge Trigger | Pull-down | Tombol Eksternal (*Active High*) |

### 3. NVIC Konfigurasi
* **EXTI line[9:5] interrupts**: Berikan tanda centang pada opsi **Enabled** untuk mengaktifkan fungsi *callback* interupsi pada Pin 7.

---

## 🔌 Diagram Skema Koneksi Hardware
* **LED Onboard**: Sudah terhubung secara internal di jalur **PE4** pada papan WeAct STM32WB55.
* **Push Button**: 
  * Salah satu kaki tombol dihubungkan ke sumber tegangan **VCC/3.3V**.
  * Kaki tombol lainnya dihubungkan ke Pin **PB7** (Konfigurasi internal Pull-Down pada mikrokontroler akan memastikan kondisi pin bernilai `LOW` saat tombol dilepas, dan bernilai `HIGH` saat tombol ditekan).

---

## 🛠️ Kebutuhan Perangkat Lunak
* **STM32CubeMX** (Versi terbaru disarankan untuk *generasi code*).
* **STM32CubeIDE** atau toolchain berbasis GCC lainnya (Keil uVision / IAR Workbench).
* **STM32Cube MCU Package for STM32WB Series**.

## 📝 Catatan Penting
Setiap kali melakukan konfigurasi ulang atau memodifikasi ulang tata letak pin pada diagram STM32CubeMX, pastikan kode kustom Anda tetap berada di dalam blok komentar bawaan STM32 seperti `/* USER CODE BEGIN ... */` dan `/* USER CODE END ... */` agar tidak terhapus otomatis oleh sistem pembuat kode (*code generator*).