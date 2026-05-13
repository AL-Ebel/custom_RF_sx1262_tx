/*
 * nau88l11.h
 *
 *  Created on: May 9, 2026
 *      Author: HP
 */

#ifndef INC_NAU88L11_H_
#define INC_NAU88L11_H_

#include "main.h"


extern int16_t nau_audio_rx[256];

HAL_StatusTypeDef NAU88L11_Ping(void);
HAL_StatusTypeDef NAU88L11_InitMicPath(void);
HAL_StatusTypeDef NAU88L11_StartRxDMA(void);
HAL_StatusTypeDef NAU88L11_MinimalTest(void);
HAL_StatusTypeDef nau_write(uint8_t reg,
                                  uint16_t value);

#endif /* INC_NAU88L11_H_ */
