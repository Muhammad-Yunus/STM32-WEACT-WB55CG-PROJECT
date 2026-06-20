# IMNP441 + PCM5102A on STM32WB55CG

Push-to-talk audio echo on a WeAct STM32WB55CG board. Hold the button to record
from the INMP441 microphone; release to hear the same clip played back through
the PCM5102A DAC.

## Hardware

- **MCU:** STM32WB55CGUx (WeAct Studio Black Pill, Cortex-M4 @ 64 MHz)
- **Microphone:** INMP441 I2S MEMS mic (slave, LRCK + BCLK + SD from SAI1)
- **DAC:** PCM5102A I2S DAC (slave, line-out / headphone)
- **Push button:** PB0 → EXTI0 (active high, external pull-down)

```
                     +---------------------+
                     |   STM32WB55CGUx     |
                     |                     |
INMP441  --(I2S)-->  | SAI1_B (slave RX)   |  (DMA, ping-pong 128×int32 halves)
                     |                     |
PCM5102A <--(I2S)--  | SAI1_A (master TX)  |  (DMA, 16 kHz / 32-bit stereo)
                     |                     |
        PB0   --->   | EXTI0 (rising+fall) |
                     +---------------------+
```

SAI1 Block A is master (generates BCLK + LRCK for both peripherals); Block B is
synchronous slave fed by the same clocks.

## Project layout

```
workspace_1.18.1/
├── IMNP441_PCM5102A_WEACT_WB55CG.ioc      CubeMX config (don't hand-edit)
├── STM32WB55CGUX_FLASH.ld                 linker script, FLASH build
├── STM32WB55CGUX_RAM.ld                   linker script, RAM build
├── Core/
│   ├── Inc/                               HAL includes + pin defines
│   └── Src/
│       ├── main.c                         application logic
│       ├── stm32wbxx_hal_msp.c            generated, untouched
│       └── stm32wbxx_it.c                 generated, untouched
├── Drivers/                               CMSIS + STM32WBxx HAL
└── Debug/                                 make build output
```

User code lives entirely inside the `USER CODE BEGIN/END` regions of `main.c`,
so re-generating from the `.ioc` will not destroy changes.

## Application logic

`main.c` defines a tiny state machine:

```c
typedef enum { AUDIO_IDLE, AUDIO_RECORDING, AUDIO_PLAYBACK } AudioState_t;

typedef struct {
    AudioState_t state;
    uint32_t     head;     // next write index in record
    uint32_t     length;   // frozen length after recording
    uint32_t     pos;      // next read index during playback
} Audio_t;

typedef struct { int32_t mic[256], tx[256], record[40000]; } AudioBuffer_t;

typedef struct { uint8_t level, prev_level; } Button_t;
```

**Transitions** (driven by main loop, not ISR):

| Event                     | From        | To          | Side effect                                 |
| ------------------------- | ----------- | ----------- | ------------------------------------------- |
| button pressed            | IDLE        | RECORDING   | `head = 0`                                  |
| button released           | RECORDING   | PLAYBACK    | `length = head; pos = 0`                    |
| `pos >= length`           | PLAYBACK    | IDLE        | (none)                                      |

### Button — EXTI0 interrupt

`PB0` is configured in CubeMX as `GPIO_MODE_IT_RISING_FALLING` with a
pull-down. `EXTI0_IRQHandler` (generated) calls
`HAL_GPIO_EXTI_IRQHandler(BTN_Pin)`, which dispatches to:

```c
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == BTN_Pin) {
    button.level = (HAL_GPIO_ReadPin(BTN_GPIO_Port, BTN_Pin)
                    == BTN_ACTIVE_LEVEL) ? 1U : 0U;
  }
}
```

The main loop reads `button.level` each iteration. With `prev_level` cached in
the struct, it detects the rising edge (`pressed`) and falling edge (`released`)
to drive state transitions. No software debounce — the CubeMX GPIO
configuration and the slow main-loop edge detection are sufficient for a
push-to-talk demo.

### Audio path — SAI1 with DMA in circular ping-pong

- `HAL_SAI_Receive_DMA` runs on **Block B** (slave RX) into `audio_buf.mic[256]`
  (two halves of 128 `int32_t`, i.e. 64 stereo frames at 32-bit I2S).
- `HAL_SAI_Transmit_DMA` runs on **Block A** (master TX) from `audio_buf.tx[256]`.
- Each DMA half-transfer fires `HAL_SAI_RxHalfCpltCallback` or
  `HAL_SAI_RxCpltCallback`. Both route to `Audio_OnMicHalf` with the appropriate
  half-buffer pointer:

  ```c
  Audio_OnMicHalf(&audio_buf.mic[0],         &audio_buf.tx[0]);         // half
  Audio_OnMicHalf(&audio_buf.mic[HALF_BUF_LEN], &audio_buf.tx[HALF_BUF_LEN]); // full
  ```

- `Audio_OnMicHalf` switches on `audio.state`:

  | State      | Behaviour                                                                                 |
  | ---------- | ----------------------------------------------------------------------------------------- |
  | RECORDING  | Copy L channel of each frame into `record` (up to `RECORD_LEN`), zero out `tx` (silence) |
  | PLAYBACK   | For each frame, write the recorded sample into both L and R slots of `tx`                |
  | IDLE       | Zero out `tx`                                                                             |

### Main loop

```c
while (1) {
  const uint8_t level      = button.level;
  const uint8_t prev_level = button.prev_level;
  const bool    pressed    = (level == 1U) && (prev_level == 0U);
  const bool    released   = (level == 0U) && (prev_level == 1U);

  if      (pressed  && audio.state == AUDIO_IDLE)     { ...start record...; }
  else if (released && audio.state == AUDIO_RECORDING){ ...start playback...; }
  else if (audio.state == AUDIO_PLAYBACK && audio.pos >= audio.length)
                                                    { audio.state = AUDIO_IDLE; }

  button.prev_level = level;
  __WFI();   // sleep until next button edge or SAI half-transfer interrupt
}
```

`__WFI()` puts the CPU to sleep between events, so the application is
interrupt-driven and easy on power.

## Memory budget

| Region       | Bytes       | Notes                                       |
| ------------ | ----------- | ------------------------------------------- |
| `record`     | 160 000     | 40 000 samples × 4 B = 2.5 s @ 16 kHz mono  |
| `mic` + `tx` | 2 048       | ping-pong, 256 int32 each                   |
| Flash (text) | ≈ 19.7 KB   | HAL SAI + DMA + application                 |
| Total BSS    | ≈ 164 KB    | dominated by `record`                       |

If the linker reports RAM overflow, reduce `RECORD_LEN` in `Core/Src/main.c`.
At 16 kHz mono, 40 000 samples ≈ 2.5 s of recording.

## Build & flash

Open the project in STM32CubeIDE 2.x (`File → Open Projects from File System`,
point at the `.ioc` folder). Build with **Project → Build** (Ctrl+B). Flash
with **Run → Debug** (F11) or **Run** (Ctrl+F11).

Command-line build (matches what the IDE does):

```bash
cd "Debug"
make -j4 all
```

Output: `IMNP441_PCM5102A_WEACT_WB55CG.elf` (and `.bin` / `.hex` siblings).

## CubeMX notes

- `SYSCLK = 64 MHz` (HSE 32 MHz → PLL → SYSCLK)
- `SAI1` clock = `PLLSAI1` (8 / 2 = 4 MHz; divided by SAI for BCLK = ~2 MHz)
- `SAI_AudioFrequency = 16 kHz`, `SAI_PROTOCOL_DATASIZE_32BIT`, stereo
- `DMA1_Channel1` ↔ SAI1_A (TX), `DMA1_Channel2` ↔ SAI1_B (RX)
- `PB0` → `GPIO_MODE_IT_RISING_FALLING`, `GPIO_PULLDOWN`, NVIC priority 0
- `EXTI0_IRQn` enabled in `NVIC`

If you change the audio format (sample rate, bit depth, mono/stereo), update
`MX_SAI1_Init`, `HALF_BUF_LEN`, `HALF_BUF_FRAMES`, and the playback loop in
`Audio_OnMicHalf` together.

## Known limitations / demo scope

- No software debounce on the button — fine for push-to-talk with a tactile
  switch, not fine for noisy environments.
- No overflow handling if recording exceeds `RECORD_LEN`. The DMA keeps
  capturing, but `Audio_OnMicHalf` simply stops writing to `record`. The next
  press will start over from `head = 0`.
- `volatile Audio_t audio` and `volatile Button_t button` rely on 32-bit
  aligned word atomic access (true on Cortex-M4 for these fields). If you add
  a 64-bit or larger field, you must add a critical section.