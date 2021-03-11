/*
* CFile1.c
*
* Created: 30/10/2012 9:09:54 AM
*  Author: michal
*/

#include "project.h"

uint16_t joy_calib[NUM_JOYSTICS];

void joystick_init(void)
{
  uint32_t i;
  DEBUG_PRINT("Joystick driver init\n");

  for (i = 0; i < 10; i++)
  {
    adc_start(-1);
    delay_ms(10);
  }

  for (i = 0; i < NUM_JOYSTICS; i++)
  {
    joy_calib[i] = adc_get(i);
    DEBUG_PRINT("Joystick %i calibration %i\n",
                i, joy_calib[i]);
  }
}

int8_t joystick_get(uint8_t channel)
{
  if (channel >= NUM_JOYSTICS)
  {
    return (0);
  }

  int16_t adcval = (adc_get(channel)) - joy_calib[channel];
  int32_t joyval;
  int8_t sign = 0;

  if (ABS(adcval) < JOYSTICK_MIDDLE_MARGIN)
  {
    return (0);
  }

  if (adcval < 0)
  {
    adcval += JOYSTICK_MIDDLE_MARGIN;
    sign = -1;
  }
  else
  {
    adcval -= JOYSTICK_MIDDLE_MARGIN;
    sign = +1;
  }

#define DIVISOR (joy_calib[channel] - JOYSTICK_END_MARGIN - JOYSTICK_MIDDLE_MARGIN)
  /* convert to log3 scale */
  joyval = JOYSTICK_MAX;
  joyval *= adcval;
  joyval *= adcval;
  joyval /= DIVISOR;
  joyval *= adcval;
  joyval /= DIVISOR;
  joyval /= DIVISOR;
  //joyval *= sign;

  if (ABS(joyval) > JOYSTICK_MAX)
  {
    return (sign * JOYSTICK_MAX);
  }

  //if (channel == 0)
  //{
  //  DEBUG_PRINT("JOY0: %i\n", joyval);
  //}
  return ((int8_t) joyval);
}

void joystick_test(void)
{
  char buffer[40];
  //lcd_init();
  //adc_init();
  //joystick_init();
  lcd_acquire();
  lcd_clear();

  while (1)
  {
    adc_start(-1);

    for (int i = 0; i < 4; i++)
    {
      snprintf(buffer, sizeof(buffer), "%1i: %4i ", i, joystick_get(i));
      lcd_write_str((i & 1) * 8, (i >> 1), buffer);
    }
  }
}
