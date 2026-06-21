/*
 * encoder.c
 *
 *  Created on: Jun 21, 2026
 *      Author: Asus
 */


#include "encoder.h"
#include <stdio.h>
#include <stdbool.h>

static volatile EncoderDirection direction = ENC_NONE;
static volatile bool button_pressed = false;


void Encoder_Init(void)
{
    direction = ENC_NONE;
}

void Encoder_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == ENC_A_Pin)
    {
        if(HAL_GPIO_ReadPin(ENC_B_GPIO_Port, ENC_B_Pin))
            direction = ENC_CCW;
        else
            direction = ENC_CW;
    }
    else if (GPIO_Pin == ENC_SW_Pin)
	{
		button_pressed = true;
	}
}


EncoderDirection Encoder_GetDirection(void)
{
    EncoderDirection d = direction;

    direction = ENC_NONE;

    return d;
}

bool Encoder_ButtonPressed(void)
{
    if (button_pressed)
    {
        button_pressed = false;
        return true;
    }

    return false;
}
