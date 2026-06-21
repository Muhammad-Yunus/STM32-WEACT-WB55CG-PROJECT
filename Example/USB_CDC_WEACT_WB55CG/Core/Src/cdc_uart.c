/* ---------------------------------------------------------------------------
 *  USB CDC — tiny ring-buffer wrapper over FS-USB-VCP  (no UART involved)
 *
 *  CDC_Init()        prime USB reception, init ring buffer
 *  CDC_ProcessPacket()  USB callback: push bytes into ring buffer
 *  CDC_ReadLine()      blocking read until \r / \n (stripped)
 *  CDC_Printf()        formatted output (blocking TX)
 *
 *  CubeMX will never touch this file.
 * --------------------------------------------------------------------------- */

/* Includes ------------------------------------------------------------------*/
#include "cdc_uart.h"

#include "usbd_cdc.h"              /* USBD_HandleTypeDef, USBD_CDC_HandleTypeDef */
#include <string.h>
#include <stdarg.h>

/* External symbols (from usbd_cdc_if.c) */
extern USBD_HandleTypeDef hUsbDeviceFS;
extern uint8_t            CDC_Transmit_FS(uint8_t *buf, uint16_t len);

/* ---------------------------------------------------------------------------
 *  Ring buffer — private storage
 * --------------------------------------------------------------------------- */
static uint8_t   s_ring[CDC_TX_TIMEOUT_MS + APP_RX_DATA_SIZE];
static uint16_t  s_head, s_tail, s_len;       /* internal write/read/count   */

/* Link to the user-visible handle (set in CDC_Init). */
static CDC_HandleTypeDef *s_hcdc;

/* ---------------------------------------------------------------------------
 *  Ring-buffer helpers
 * --------------------------------------------------------------------------- */

static inline void rb_push_byte(uint8_t b)
{
  if (s_len >= sizeof(s_ring)) return;
  s_ring[s_head++] = b;
  s_head &= (sizeof(s_ring) - 1);
  s_len++;
}

static inline uint16_t rb_drain_byte(uint8_t *dst, uint16_t max)
{
  uint16_t n = s_len < max ? s_len : max;
  for (uint16_t i = 0; i < n; i++) {
    *dst++ = s_ring[s_tail++];
    s_tail &= (sizeof(s_ring) - 1);
  }
  s_len -= n;
  return n;
}

/* ---------------------------------------------------------------------------
 *  USB CDC API
 * --------------------------------------------------------------------------- */

/* Prime USB reception and initialise ring buffer. */
void CDC_Init(CDC_HandleTypeDef *hcdc)
{
  /* Initialise ring buffer (power-of-two mask — sizeof ≈ APP_RX_DATA_SIZE) */
  memset(s_ring, 0, sizeof(s_ring));
  s_head = s_tail = s_len = 0;

  /* Link user handle to internal state. */
  s_hcdc    = hcdc;
  hcdc->pRxBuff      = s_ring;
  hcdc->RxBufferSize = sizeof(s_ring);
  hcdc->RxHead       = 0;
  hcdc->RxTail       = 0;
  hcdc->RxLen        = 0;

  /* Prime USB FS reception so callback fires. */
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);
  USBD_CDC_ReceivePacket(&hUsbDeviceFS);
}

/* USB CDC callback (called from usbd_cdc_if.c::CDC_Receive_FS). */
void CDC_ProcessPacket(uint8_t *Buf, uint32_t Len)
{
  for (uint32_t i = 0; i < Len; i++)
    rb_push_byte(Buf[i]);       /* silently drops when full */
}

/* ---------------------------------------------------------------------------
 *  CDC_TransmitBlocking_FS  —  wait for TX idle, then dispatch
 * --------------------------------------------------------------------------- */
CDC_StatusTypeDef CDC_TransmitBlocking_FS(uint8_t *buf, uint16_t len)
{
  if (buf == NULL || len == 0) return CDC_ERROR;

  USBD_CDC_HandleTypeDef *h =
      (USBD_CDC_HandleTypeDef *)hUsbDeviceFS.pClassData;

  uint32_t t0 = HAL_GetTick();
  while (h->TxState != 0) {
    if ((HAL_GetTick() - t0) > CDC_TX_TIMEOUT_MS) return CDC_ERROR;
  }
  return (CDC_StatusTypeDef)CDC_Transmit_FS(buf, len);
}

/* ---------------------------------------------------------------------------
 *  CDC_Printf  —  formatted output over USB CDC
 * --------------------------------------------------------------------------- */
int CDC_Printf(CDC_HandleTypeDef *hcdc, const char *fmt, ...)
{
  (void)hcdc;
  if (fmt == NULL) return -1;

  char tmp[128];
  va_list ap;
  va_start(ap, fmt);
  int n = vsnprintf(tmp, sizeof(tmp), fmt, ap);
  va_end(ap);

  if (n <= 0) return 0;
  if ((uint32_t)n >= sizeof(tmp)) n = sizeof(tmp) - 1;

  CDC_TransmitBlocking_FS((uint8_t *)tmp, (uint16_t)n);
  return n;
}

/* ---------------------------------------------------------------------------
 *  CDC_ReadLine  —  BLOCKING read until \r / \n  (stripped)
 *
 *  Drains every available byte, looking for the first \r or \n.
 *  If the first byte is a newline the call fails (stray \n after \r).
 * --------------------------------------------------------------------------- */
CDC_StatusTypeDef CDC_ReadLine(CDC_HandleTypeDef *hcdc,
                               char *buf, uint32_t bufsize)
{
  (void)hcdc;                        /* internal ring buffer, no access needed */
  if (buf == NULL || bufsize == 0) return CDC_ERROR;

  uint16_t pos = 0;
  uint32_t t0  = HAL_GetTick();

  while (pos < bufsize - 1) {

    /* --- Wait for at least one byte --- */
    while (s_len == 0) {
      if ((HAL_GetTick() - t0) > 2000U) return CDC_ERROR;
    }

    /* --- Drain everything available into output buffer --- */
    uint16_t n = rb_drain_byte((uint8_t *)(buf + pos),
                                (uint16_t)(bufsize - pos - 1));
    pos += n;

    /* --- Scan for newline in what we just drained --- */
    for (uint16_t i = pos - n; i < pos; i++) {
      if (buf[i] == '\r' || buf[i] == '\n') {
        if (i == 0) return CDC_ERROR;     /* stray lone-newline */
        buf[i] = '\0';                     /* strip terminator */
        return CDC_OK;
      }
    }
    buf[pos] = '\0';                       /* partial line */
  }

  return CDC_OK;                            /* full, no newline */
}
