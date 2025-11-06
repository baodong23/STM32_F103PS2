/*
 * keycode.h
 *
 *  Created on: Nov 6, 2025
 *      Author: Dong Bao
 */

#ifndef INC_KEYBOARD_H_
#define INC_KEYBOARD_H_

#include "stm32f1xx_hal.h"
#include "ps2.h"

extern void delay_us(uint16_t time);

#define CLOCK 24    // the clock speed in MHz driving XTAL1 & XTAL2
#define BREAK 336   // the period between keycode/byte transmissions (in particular for extended/release codes, or multiple argument byte transmissions in a row)
#define EXT 0x02E0  // extension keycode with stop/parity
#define REL 0x03F0  // release keycode with stop/parity
#define ACK 0x03FA  // acknowledge command with stop/parity
#define RE  0x02FE  // resend command with stop/parity
#define NA  0x0300  // NA/error command with stop/parity (unused, experimental)

typedef struct {
	GPIO_TypeDef *row_port;
	GPIO_TypeDef *column_port;
	uint8_t row_size;
	uint8_t column_size;
}Keyboard_HandleTypeDef;

void Send_Code(PS2_HandleTypeDef *ps2, uint32_t keycode, uint8_t keyState);
void Follow_Command(PS2_HandleTypeDef *ps2, uint32_t command);
void Keyboard_Init(Keyboard_HandleTypeDef *key, GPIO_TypeDef *column_port, GPIO_TypeDef *row_port, uint8_t column, uint8_t row);
void Key_Check(PS2_HandleTypeDef *ps2, Keyboard_HandleTypeDef *key, uint16_t *column_pin, uint16_t *row_pin);

#endif /* INC_KEYBOARD_H_ */
