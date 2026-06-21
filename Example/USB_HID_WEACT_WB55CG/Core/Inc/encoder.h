/*
 * encoder.h
 *
 *  Created on: Jun 21, 2026
 *      Author: Asus
 */

#ifndef INC_ENCODER_H_
#define INC_ENCODER_H_

#include "main.h"
#include <stdbool.h>

typedef enum
{
    ENC_NONE = 0,
    ENC_CW,
    ENC_CCW

} EncoderDirection;

void Encoder_Init(void);

EncoderDirection Encoder_GetDirection(void);
bool Encoder_ButtonPressed(void);

void Encoder_EXTI_Callback(uint16_t GPIO_Pin);


#endif /* INC_ENCODER_H_ */
