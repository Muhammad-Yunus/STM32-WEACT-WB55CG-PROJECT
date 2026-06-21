/* ---------------------------------------------------------------------------
 *  USB CDC — tiny abstraction over FS-USB-VCP  (no UART involved)
 *
 *  Ring-buffer accumulates inbound bytes.
 *  CDC_ReadLine()     waits (blocking) for \r or \n.
 *  CDC_Printf()       formats into 128 B, sends atomically.
 *  CDC_TransmitBlocking_FS() waits for TX idle before dispatching.
 *
 *  CubeMX will never touch this file.
 * --------------------------------------------------------------------------- */

#ifndef CDC_UART_H
#define CDC_UART_H

#include <stdint.h>
#include <stdarg.h>

/* USB-CDC constants (match usbd_cdc_if.c) */
#define APP_RX_DATA_SIZE   2048U
#define CDC_TX_TIMEOUT_MS  500U

/* Status codes */
typedef enum {
  CDC_OK = 0,
  CDC_ERROR = 1
} CDC_StatusTypeDef;

/* CDC handle — user-visible ring-buffer state */
typedef struct {
  uint8_t             *pRxBuff;
  uint32_t             RxBufferSize;
  volatile uint32_t    RxHead;
  volatile uint32_t    RxTail;
  volatile uint32_t    RxLen;
} CDC_HandleTypeDef;

/* --------------- public API --------------- */
void    CDC_Init(CDC_HandleTypeDef *hcdc);
void    CDC_ProcessPacket(uint8_t *Buf, uint32_t Len);

/* Format-and-send (blocks until TX idle). */
int     CDC_Printf(CDC_HandleTypeDef *hcdc, const char *fmt, ...);

/* Blocking read-line — waits for \r or \n.  Strips the terminator. */
CDC_StatusTypeDef CDC_ReadLine(CDC_HandleTypeDef *hcdc,
                               char *buf, uint32_t bufsize);

/* Direct transmit helpers */
CDC_StatusTypeDef CDC_TransmitBlocking_FS(uint8_t *buf, uint16_t len);
uint8_t           CDC_Transmit_FS(uint8_t *buf, uint16_t len);

/* User-accessible RX buffer (for USBD_ init in CDC_Init) */
extern uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

#endif /* CDC_UART_H */
