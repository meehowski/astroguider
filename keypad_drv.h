/*
* IncFile1.h
*
* Created: 29/09/2012 4:19:47 PM
*  Author: michal
*/


#ifndef KEYPAD_H_
#define KEYPAD_H_

// keyboard interface
#define KEY_FILTER_BUFFER 4

#define kButtonDMask  0x00ff /* digital key mask */
#define kButtonAMask  0xff00 /* analog key mask */

#define kButtonNoKey  0x00
#define kButtonRED    0x01
#define kButtonBLACK  0x02
#define kButtonBLUE   0x04
#define kButtonLeft   0x0100
#define kButtonPgLeft 0x0200
#define kButtonRight  0x0400
#define kButtonPgRight  0x0800
#define kButtonUp   0x1000
#define kButtonPgUp   0x2000
#define kButtonDown   0x4000
#define kButtonPgDown 0x8000

uint16_t keypad_ascan(void);
uint16_t keypad_dscan(void);
void keypad_init(void);
void keypad_test(void);
uint16_t keypad_get(void);
uint8_t keypad_iskey(uint16_t key);

#endif /* KEYPAD_H_ */
