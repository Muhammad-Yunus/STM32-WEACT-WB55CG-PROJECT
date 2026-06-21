/*
 * usb_mouse.h
 *
 *  Created on: Jun 21, 2026
 *      Author: Asus
 */

#ifndef INC_USB_MOUSE_H_
#define INC_USB_MOUSE_H_

#include "stdint.h"

void USB_Mouse_Init(void);

void USB_Mouse_Move(int8_t x, int8_t y);

void USB_Mouse_Wheel(int8_t wheel);

void USB_Mouse_LeftDown(void);

void USB_Mouse_LeftUp(void);

void USB_Mouse_LeftClick(void);

#endif /* INC_USB_MOUSE_H_ */
