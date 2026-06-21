/*
 * usb_mouse.c
 *
 *  Created on: Jun 21, 2026
 *      Author: Asus
 */


#include "usb_mouse.h"

#include "usb_device.h"

#include "usbd_hid.h"

extern USBD_HandleTypeDef hUsbDeviceFS;

static void Mouse_Send(uint8_t button,
                       int8_t x,
                       int8_t y,
                       int8_t wheel)
{
    uint8_t report[4];

    report[0] = button;
    report[1] = x;
    report[2] = y;
    report[3] = wheel;

    USBD_HID_SendReport(&hUsbDeviceFS,
                        report,
                        sizeof(report));
}

void USB_Mouse_Move(int8_t x, int8_t y)
{
    Mouse_Send(0, x, y, 0);

    Mouse_Send(0,0,0,0);
}

void USB_Mouse_Wheel(int8_t wheel)
{
    Mouse_Send(0,0,0,wheel);

    Mouse_Send(0,0,0,0);
}

void USB_Mouse_LeftDown(void)
{
    Mouse_Send(0x01,0,0,0);
}

void USB_Mouse_LeftUp(void)
{
    Mouse_Send(0x00,0,0,0);
}

void USB_Mouse_LeftClick(void)
{
    USB_Mouse_LeftDown();

    HAL_Delay(10);

    USB_Mouse_LeftUp();
}
