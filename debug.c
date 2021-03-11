/*
* CFile1.c
*
* Created: 04/11/2012 8:20:20 PM
*  Author: michal
*/

#include "project.h"

#define WAIT_FOR_KEY(msec) \
{ \
  for (uint32_t i = 0; (((keypad_get() & kButtonDMask) == kButtonNoKey) && (i < (msec))); i = i + 10) \
  { \
    delay_ms(10); \
  } \
}

void runtime_msg(const char *msg)
{
  char i = 2;
  //lcd_acquire();
  lcd_write_strn(0, 0, msg, LCD_WIDTH);
  lcd_write_strn(0, 1, "", LCD_WIDTH);
  delay_ms(500);

  while (((keypad_get() & kButtonDMask) == kButtonNoKey) && (i != 0))
  {
    lcd_write_strn(0, 1, "Press Any Key", LCD_WIDTH);
    lcd1602_write_char(15, 1, i + '0');
    led_state(RED_LED, 1);
    WAIT_FOR_KEY(1000);
    led_state(RED_LED, 0);
    lcd_write_strn(0, 1, "", LCD_WIDTH);
    WAIT_FOR_KEY(1000);
    i--;
  };

  lcd_clear();

  //lcd_release();
  while ((keypad_get() & kButtonDMask) != kButtonNoKey) {};
}

static void _runtime_msg(const char *msg1, const char *msg2)
{
  lcd_write_strn(0, 0, msg1, LCD_WIDTH);
  lcd_write_strn(0, 1, (const char *) msg2, LCD_WIDTH);
}

void runtime_error(const char *msg1, const char *msg2)
{
  lcd_acquire();
  PRINT("RUNTIME_ERROR: %s %s\n", msg1, msg2);

  do
  {
    led_state(RED_LED, 1);
    _runtime_msg(msg1, msg2);
    WAIT_FOR_KEY(1000);
    led_state(RED_LED, 0);
    _runtime_msg(msg1, "PRESS ANY KEY");
    WAIT_FOR_KEY(500);
  }
  while ((keypad_get() & kButtonDMask) == kButtonNoKey);

  SysCtlReset(); // point of no return
}

void runtime_debug(const char *msg1, const char *msg2)
{
  PRINT("RUNTIME_DEBUG: %s %s\n", msg1, msg2);
  lcd_acquire();
  _runtime_msg(msg1, msg2);
  delay_ms(1000);
  lcd_release();
  led_state(RED_LED, 0);
}
