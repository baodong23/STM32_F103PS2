/*
 * ps2.c
 *
 *  Created on: Oct 30, 2025
 *      Author: Dong Bao
 */

#include "ps2.h"

void PS2_Init(PS2_HandleTypeDef *ps2, GPIO_TypeDef *gpio_port, uint16_t pin_clk, uint16_t pin_data) {
	ps2->ps2_port = gpio_port;
	ps2->ps2_clk_pin = pin_clk;
	ps2->ps2_data_pin = pin_data;
}

void PS2_Transmit(PS2_HandleTypeDef *ps2, uint32_t keycode) {
	LAST_BYTE = keycode;
	keycode <<= 1;

	for (int i = 0; i < 11; i++) {
		PS2_Set_Clock();

		PS2_Write_Data(ps2, (uint8_t)keycode);

		PS2_Reset_Clock();

		keycode >>= 1;

		delay_us(16);

		PS2_Set_Clock();

		delay_us(14);
	}

	PS2_Set_Data_Clock();
}

int PS2_Receive(PS2_HandleTypeDef *ps2) {
	while(!(PS2_Check_Clock(ps2) && !PS2_Read_Data(ps2))) {

	}

	uint8_t index = 0;
	int buffer;

	for(index = 0; index < 10; index++) {
		PS2_Reset_Clock();

		delay_us(16);

		PS2_Set_Clock();

		buffer |= (PS2_Read_Data(ps2) << index);

		delay_us(16);
	}

	PS2_Write_Data(ps2, PS2_RESET);

	PS2_Reset_Clock();

	delay_us(16);

	PS2_Set_Data_Clock();

	return buffer;
}

