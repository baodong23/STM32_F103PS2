/*
 * keyboard.c
 *
 *  Created on: Nov 6, 2025
 *      Author: Dong Bao
 */

#include "keyboard.h"

// 2D array to house key scan codes of key matrix (as well as each code's parity and stop bit in the 2nd-byte)
const uint32_t KEY_SCAN_CODES[14][6] = {
    { 0x0314, 0x0312, 0x0258, 0x020d, 0x020e, 0x0276 },         // { L_CTRL, L_SHFT, CAPS, TAB, B_T, ESC },
    { 0xe0021f, 0x021a, 0x021c, 0x0215, 0x0216, 0x00 },         // { WIN, Z, A, Q, R_1, 0 },
    { 0x0311, 0x0322, 0x031b, 0x031d, 0x031e, 0x0305 },         // { L_ALT, X, S, W, R_2, F_1 },
    { 0x00, 0x0321, 0x0223, 0x0324, 0x0226, 0x0306 },           // { 0, C, D, E, R_3, F_2 },
    { 0x00, 0x022a, 0x032b, 0x032d, 0x0225, 0x0204 },           // { 0, V, F, R, R_4, F_3 },
    { 0x00, 0x0232, 0x0234, 0x022c, 0x032e, 0x030c },           // { 0, BB, G, T, R_5, F_4 },
    { 0x0229, 0x0231, 0x0333, 0x0335, 0x0336, 0x00 },           // { SPACE, N, H, Y, R_6, 0 },
    { 0x00, 0x033a, 0x023b, 0x033c, 0x023d, 0x0303 },           // { 0, M, J, U, R_7, F_5 },
    { 0x00, 0x0341, 0x0342, 0x0243, 0x023e, 0x020b },           // { 0, COMMA, K, I, R_8, F_6 },
    { 0xe00311, 0x0249, 0x034b, 0x0344, 0x0246, 0x0283 },       // { R_ALT, PER, L, O, R_9, F_7 },
    { 0xe00327, 0x024a, 0x024c, 0x034d, 0x0245, 0x030a },       // { WIFN, B_SLA, S_COL, PP, R_0, F_8 },
    { 0xe0022f, 0x00, 0x0252, 0x0254, 0x034e, 0x0201 },         // { MENUS, 0,  F_T, L_BRAK, SUB, F_9 },
    { 0x00, 0x00, 0x0378, 0x025b, 0x0355, 0x0309 },             // { 0, 0, F_11, R_BRAK, EQU, F_10 },
    { 0xe00314, 0x0359, 0x035a, 0x025d, 0x0366, 0x0207 }        // { R_CTRL, R_SHFT, ENTER, F_SLA, BCKSP, F_12 }
};
static unsigned int  LAST_BYTE = 0x00;   // for keeping track of last byte sent to host (for retransmission request)
static unsigned char EN = 1;         // for enabling/disabling keyscanning
static unsigned char REPEAT_RATE = 50;   // for the rate at which a keycode is repeated (1000 / REPEAT_RATE * 10 hertz or cps)
static unsigned char REPEAT_DELAY = 100; // for delay before a pressed key starts repeating (REPEAT_DELAY * 10 milliseconds)
static unsigned char ELAPSED_TIME = 0;   // for counting intervals of 10ms created by Timer 2 to keep track of when to repeat keycodes

void Keyboard_Init(Keyboard_HandleTypeDef *key, GPIO_TypeDef *column_port, GPIO_TypeDef *row_port, uint8_t column, uint8_t row) {
	key->column_port = column_port;
	key->row_port = row_port;
	key->row_size = row;
	key->column_size = column;
}

void Send_Code(PS2_HandleTypeDef *ps2, uint32_t keycode, uint8_t keyState) {
	if( keyState ){
		// check if extended code
		if( (keycode & 0xff0000) == 0xe00000 ){
			// transmit extension, then keycode
			PS2_Transmit(ps2, EXT);
			delay_us(BREAK); // BREAK period between code and extension/state transmission
			PS2_Transmit(ps2, keycode);
			delay_us(BREAK);
		}else{
			PS2_Transmit(ps2, keycode);
			delay_us(BREAK);
		}
	}else{ // else the keyState is zero, indicating to send the code for releasing a key
		// check if extended code
		if( (keycode & 0xff0000) == 0xe00000 ){
			// transmit extension, then release code, then finally keycode
			PS2_Transmit(ps2, EXT);
			delay_us(BREAK);
			PS2_Transmit(ps2, REL);
			delay_us(BREAK);
			PS2_Transmit(ps2, keycode);
			delay_us(BREAK);
		}else{
			// transmit release code, then keycode
			PS2_Transmit(ps2, REL);
			delay_us(BREAK);
			PS2_Transmit(ps2, keycode);
			delay_us(BREAK);
		}
	}
}

void Follow_Command(PS2_HandleTypeDef *ps2, uint32_t command) {
	command &= 0xff; // truncates command for below switch statement
    uint32_t arg; // for receiving bytes back from the host when necessary
    switch( command ){
        case 0xed: // set LEDs
        	PS2_Transmit(ps2, ACK);      // acknowledge
            PS2_Receive(ps2, &arg);    // read argument byte from host
            PS2_Transmit(ps2, ACK);      // acknowledge
            // check state of CapsLock LED (bit 2), the only "lock-key" present on version 1.0 of this keyboard
            if( (arg & 0x04) ){
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); // set CapsLock LED
            }else{
            	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET); // clear CapsLock LED // 1111 0111
            }// other LEDs would be: bit 0 == ScrollLock, bit 1 == NumberLock, bit 3 == International purposes (unused in US KBs)
            break;
        case 0xee: // echo
        	PS2_Transmit(ps2, 0x03ee);   // respond to host with an echo back
            break;
        case 0xf0: // scan code set
        	PS2_Transmit(ps2, ACK);      // acknowledge
            PS2_Receive(ps2, &arg);    // read argument byte from host
            PS2_Transmit(ps2, ACK);      // acknowledge
            // if the argument received is 0, respond with current scan code set (which is 0x02)
            if( !(arg & 0xff) )
            	PS2_Transmit(ps2, 0x0341); // respond with scan code set (firmware is hardcoded as always Set 2 or code 0x41)
            break;
        case 0xf2: // read ID
            // respond with device id of 0xab83 (host appears to sometimes disregard the second/low byte, although this may only be due to having seperate ports for KB and mouse)
        	PS2_Transmit(ps2, ACK);      // acknowledge
        	PS2_Transmit(ps2, 0x02ab);
            delay_us(BREAK);    // brief delay to ensure reception
            PS2_Transmit(ps2, 0x0283);
            break;
        case 0xf3: // set typematic delay / auto-repeat rate of keycodes
        	PS2_Transmit(ps2, ACK);      // acknowledge
            PS2_Receive(ps2, &arg);    // read the argument from host
            PS2_Transmit(ps2, ACK);      // acknowledge
            // set requested repeat rate (bits 0-4 of arg, 000X-XXXX) // (1000 / REPEAT_RATE * 10) cps
            if( (arg & 0x1f) >= 0x18 && (arg & 0x1f) <= 0x1f )
                REPEAT_RATE = 50; // 2 cps
            else if( (arg & 0x1f) >= 0x10 && (arg & 0x1f) <= 0x17 )
                REPEAT_RATE = 25; // 4 cps
            else if( (arg & 0x1f) >= 0x08 && (arg & 0x1f) <= 0x0f )
                REPEAT_RATE = 12; // 8.3 cps
            else if( (arg & 0x1f) >= 0x04 && (arg & 0x1f) <= 0x07 )
                REPEAT_RATE = 6; // 16.6 cps
            else
                REPEAT_RATE = 3; // 33.3 cps
            // set the requested delay before keys repeat (bits 5-6 of arg, 0XX0-0000) // (REPEAT_DELAY * 10) milliseconds
            if( (arg & 0x60) == 0x20 )      // 0x01
                REPEAT_DELAY = 50; // 500ms
            else if( (arg & 0x60) == 0x40 ) // 0x10
                REPEAT_DELAY = 75; // 750ms
            else if( (arg & 0x60) == 0x60 ) // 0x11
                REPEAT_DELAY = 100; // 1000ms
            else                            // 0x00
                REPEAT_DELAY = 25; // 250ms
            break;
        case 0xf4: // enable (enables key-matrix scanning)
        	PS2_Transmit(ps2, ACK);      // acknowledge
            EN = 1;
            break;
        case 0xf5: // disable (disables key-matrix scanning)
        	PS2_Transmit(ps2, ACK);      // acknowledge
            EN = 0;
            break;
        case 0xfe: // resend last byte
        	PS2_Transmit(ps2, ACK);      // acknowledge
        	PS2_Transmit(ps2, LAST_BYTE);// last byte sent (seems seldom to be called)
            break;
        case 0xff: // reset
        	PS2_Transmit(ps2, ACK);      // acknowledge
            delay_us(BREAK);    // brief delay for "simulating" a reset, but also to ensure reception
            PS2_Transmit(ps2, 0x03aa);   // report BAT successful
            break;
        // NOTE: commands F6-FD apply to scancode set 3 only. This keyboard firmware currently is hardcoded to scancode set 2, thus these commands can be ignored.
        case 0xf6: // set default
        case 0xf7: // set all keys to typematic/autorepeat
        case 0xf8: // set all keys to make/release
        case 0xf9: // set all keys to make only
        case 0xfa: // set all keys to typematic/autorepeat/make/release
        	PS2_Transmit(ps2, ACK);      // acknowledge
            break;
        case 0xfb: // set specific key to typematic/autorepeat only
        case 0xfc: // set specific key to make/release
        case 0xfd: // set specific key to make only
        	PS2_Transmit(ps2, ACK);      // acknowledge
            PS2_Receive(ps2, &arg);    // read argument byte from host
            PS2_Transmit(ps2, ACK);      // acknowledge
            break;
        default: // command unknown or reception error
        	PS2_Transmit(ps2, RE);   // resend
            //P2 |= 0x20;       // DEBUGGING LED
            break;
    }//end_switch
}

void Key_Check(PS2_HandleTypeDef *ps2, Keyboard_HandleTypeDef *key, uint16_t *column_pin, uint16_t *row_pin) {
	unsigned char keyStamps[key->column_size][key->row_size];
	uint8_t i = 0, j = 0;
	uint32_t buffer = 0;
	start:
		// check if host is attempting to communicate or inhibit communications
		if(!PS2_Check_Clock(ps2)) {
			//P2 |= 0x20; // DEBUGGING LED (one method I sometimes employ in debugging is to have certain LEDs light under certain conditions)
			delay_us(50);
		// check if host is ready to transmit
		}else if(PS2_Check_Clock(ps2) && !PS2_Read_Data(ps2)) {
			PS2_Receive(ps2, &buffer);
			Follow_Command(ps2, buffer);
		// otherwise, if key-matrix scanning is enabled, proceed with scanning for keypresses
		}else if( EN ){
			//P2 |= 0x10; // DEBUGGING LED
			// loops for checking key matrix for pressed keys, first checking Port 1 (bits 1 to 8) columns then Port 3 (bits 1 to 6) columns
			//P3 = 0x00, P1 = 0x01;
			for(i = 0; i < key->column_size; i++) {
				HAL_GPIO_WritePin(key->column_port, column_pin[i], GPIO_PIN_SET);
				// check 0.0 to 0.5 for active/past  input
				for(j = 0; j < key->row_size; j++) {
					// check if clock is being pulled low before each keyscan, as device is expected to abort scanning if host requests transmission
					if(!PS2_Check_Clock(ps2)) {
						goto start;
					}
					// if j-th Port bit is active high, determine delay then transmit code
					if(HAL_GPIO_ReadPin(key->row_port, row_pin[j])) { //P0 & (0x01 << j
						// if the key was not priorly active, immediately transmit code
						if( !keyStamps[i][j] ) {
							Send_Code(ps2, KEY_SCAN_CODES[i][j], 1 );
							keyStamps[i][j] = ELAPSED_TIME; // update key time-stamp
						}
						// if key was active, determine if the REPEAT_DELAY has been met or already was met
						else if( keyStamps[i][j] >= 128 || // high bit indicates REPEAT_DELAY already was met and key is in repeating mode
								(ELAPSED_TIME > keyStamps[i][j] && (ELAPSED_TIME - keyStamps[i][j] >= REPEAT_DELAY)) ||
								(ELAPSED_TIME < keyStamps[i][j] && ((127 - keyStamps[i][j]) + ELAPSED_TIME >= REPEAT_DELAY))
						) {
							keyStamps[i][j] |= 0x80; // set the high-bit of the byte to indicate key is in repeating mode
							// proceed to repeat the keycode at the specified REPEAT_RATE interval
							if( ELAPSED_TIME % REPEAT_RATE == 0 && (keyStamps[i][j] & 0x7f) != ELAPSED_TIME ) {
								keyStamps[i][j] = 0x80 | ELAPSED_TIME; // update key time-stamp in repeating mode
								Send_Code(ps2, KEY_SCAN_CODES[i][j], 1 );
							}
						}
					// else if it was active, transmit released state
					}else if( keyStamps[i][j] ) {
						Send_Code(ps2, KEY_SCAN_CODES[i][j], 0 );
						keyStamps[i][j] = 0; // clear key time-stamp
					}
				}//end_for_rows
				// NOTE: SFR requires a max of 700 nano-seconds to set the Port data for valid output, which without parasitic capacitance is negligable
				delay_us(100); // fixes potential ghost bug! (ie, parasitic capacitance in circuit causing ghost key-presses in bottom row)
			}//end_for_columns
		}//end_if_else
		delay_us(50);
}
