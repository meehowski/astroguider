/*
* IncFile1.h
*
* Created: 29/09/2012 4:18:35 PM
*  Author: michal
*/


#ifndef LCD1602_H_
#define LCD1602_H_

#include "project.h"

#define LCD_WIDTH 16
#define LCD_HEIGHT 2

// LCD interface
#define lcd_write_str     lcd1602_write_str
#define lcd_write_strn     lcd1602_write_strn
#define lcd_write_char      lcd1602_write_char
#define lcd_write_cchar     lcd1602_write_cchar
#define lcd_clear     lcd1602_clear
#define lcd_test      lcd1602_test
#define lcd_custom_char     lcd1602_custom_char
#define lcd_init      lcd1602_init
#define lcd_acquire   lcd1602_acquire
#define lcd_release   lcd1602_release

#define CHR_CUSTOM_0  0
#define CHR_CUSTOM_1  1
#define CHR_CUSTOM_2  2
#define CHR_CUSTOM_3  3
#define CHR_CUSTOM_4  4
#define CHR_CUSTOM_5  5
#define CHR_CUSTOM_6  6
#define CHR_CUSTOM_7  7
#define CHR_BLANK 32
#define CHR_UNDERLINE 95
//#define SQUARE    219
#define CHR_RIGHTARROW  126
#define CHR_LEFTARROW 127
#define CHR_PLUSMINUS 177
#define CHR_UPARROW   179
#define CHR_DNARROW   180
#define CHR_RETURN    181
#define CHR_BETA    195
#define CHR_TARGET    199
#define CHR_ARCMINUTE 208
#define CHR_ARCSECOND 209
#define CHR_DEGREE    210 // 223
#define CHR_TILDA   213
#define CHR_LEFTARROWS  215
#define CHR_RIGHTARROWS 216
#define CHR_BACKSLASH 218
#define CHR_EX      219
#define CHR_BRACKET_R 220
#define CHR_BRACKET_C 221
#define CHR_BRACKET_T 222
#define CHR_THREE_BARS  223
#define CHR_ALPHA   224
#define CHR_TOPSTAR   235
#define CHR_BLOCK   255

extern const uint8_t symbols_mode[9][8];
extern const uint8_t symbols_battery[7][8];
extern uint8_t LCDBacklight;

void lcd1602_init(void);
void lcd1602_clear(void);
void lcd1602_write_cchar(uint8_t start_char, uint8_t len, const unsigned char *data);
//void lcd1602_write_cchar_P(uint8_t start_char, uint8_t len, const unsigned char *data);
void lcd1602_write_str(uint8_t x, uint8_t y, const char *s);
void lcd1602_write_strn(uint8_t x, uint8_t y, const char *s, uint8_t n);
//void lcd1602_write_str_P(uint8_t x, uint8_t y, const char *s);
void lcd1602_write_char(unsigned char x, unsigned char y, unsigned char data);
void lcd1602_test();
void lcd1602_custom_char(uint8_t cchar, int8_t val, const uint8_t *cchar_array);
void lcd_acquire(void);
void lcd_release(void);
//void lcd1602_setbattery_char(uint8_t cchar, int8_t value);
//void lcd1602_setmode_char(uint8_t cchar, int8_t mode);

#endif /* LCD1602_H_ */

