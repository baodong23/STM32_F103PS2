/*
 * ps2.h
 *
 *  Created on: Oct 30, 2025
 *      Author: Dong Bao
 */

#ifndef INC_PS2_H_
#define INC_PS2_H_

#include "stm32f1xx_hal.h"

extern void delay_us(uint16_t time);

extern unsigned int LAST_BYTE;

typedef enum {
	PS2_RESET,
	PS2_SET
}PS2_Status;

typedef struct {
	GPIO_TypeDef *ps2_port;
	GPIO_InitTypeDef ps2_gpio_init;
	uint16_t ps2_clk_pin;
	uint16_t ps2_data_pin;
}PS2_HandleTypeDef;

#define PS2_Reset_Clock()				ps2->ps2_port->BSRR = (uint32_t)(ps2->ps2_clk_pin << 16)	//HAL_GPIO_WritePin(ps2->ps2_port, ps2_clk_pin, PS2_RESET)
#define PS2_Set_Clock()					ps2->ps2_port->BSRR = (uint32_t)(ps2->ps2_clk_pin)         //HAL_GPIO_WritePin(ps2->ps2_port, ps2_clk_pin, PS2_SET)
#define PS2_Check_Clock(__handle__) \
		(uint32_t)(((__handle__->ps2_port->IDR) & (__handle__->ps2_clk_pin)) ? 1 : 0)

#define PS2_Set_Data_Clock()		    ps2->ps2_port->BSRR = (uint32_t)(ps2->ps2_clk_pin | ps2->ps2_data_pin)
#define PS2_Reset_Data_Clock()			ps2->ps2_port->BSRR = (uint32_t)(ps2->ps2_clk_pin | ps2->ps2_data_pin) << 16

#define PS2_Write_Data(__handle__, __state__) \
		(__handle__->ps2_port->BSRR) = (uint32_t)((__state__ & 0x01) ? (__handle__->ps2_data_pin) : (__handle__->ps2_data_pin << 16))
#define PS2_Read_Data(__handle__) \
		(uint32_t)(((__handle__->ps2_port->IDR) & (__handle__->ps2_data_pin)) ? 1 : 0)

//#define PS2_Read_Data()					HAL_GPIO_ReadPin(ps2_port, ps2_data_pin)

void PS2_Init(PS2_HandleTypeDef *ps2, GPIO_TypeDef *gpio_port, uint16_t pin_clk, uint16_t pin_data);
void PS2_Transmit(PS2_HandleTypeDef *ps2, uint32_t keycode);
int PS2_Receive(PS2_HandleTypeDef *ps2);
#endif /* INC_PS2_H_ */
