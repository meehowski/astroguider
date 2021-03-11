/*
* CFile1.c
*
* Created: 19/09/2012 12:58:29 PM
*  Author: michal
*/

#include "project.h"

typedef struct
{
  uint32_t period;
  uint32_t duration;
  uint32_t counter;
  uint8_t enabled;
} pwm_t;

const uint8_t pwm_out[NUM_PWM] = { GREEN_LED, BLUE_LED };
pwm_t pwm[NUM_PWM];

volatile static uint32_t tick_counter = 0;
volatile static uint32_t delta_counter = 0, clock_counter = 0, mailbox_arg = 0;
volatile static uint8_t mailbox = TIMER_MAILBOX_EMPTY;

void rtc_service()
{
  //led_state((tick_counter & 0x180) == 0 ? GREEN_LED : BLUE_LED, (tick_counter & 0x7f) == 0);
  //GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_PIN_2*(tick_counter & 0x1));
  // scan keypad
  keypad_ascan();
  keypad_dscan();
  // simple tick counter
  tick_counter++;

  // clock
  // process mailbox requests
  if (mailbox != TIMER_MAILBOX_EMPTY)
  {
    switch (mailbox)
    {
      case TIMER_MAILBOX_GET_RTC_DELTA:
        mailbox_arg = delta_counter;
        mailbox = TIMER_MAILBOX_DONE;
        delta_counter = 0;
        break;

      case TIMER_MAILBOX_GET_RTC:
        mailbox_arg = clock_counter;
        mailbox = TIMER_MAILBOX_DONE;
        break;

      case TIMER_MAILBOX_SET_RTC:
        clock_counter = mailbox_arg;
        mailbox = TIMER_MAILBOX_DONE;
        break;

      case TIMER_MAILBOX_RESET_RTC:
        tick_counter = delta_counter = clock_counter = 0;
        mailbox = TIMER_MAILBOX_DONE;
        break;

      default:
        /* not supposed to be here */
        mailbox = TIMER_MAILBOX_EMPTY;
        break;
    }
  }

  // sw pwm generators
  for (uint8_t i = 0; i < NUM_PWM; i++)
  {
    pwm_t *my_pwm = &pwm[i];

    if (my_pwm->enabled == false)
    {
      continue;
    }

    led_state(pwm_out[i], (my_pwm->counter < my_pwm->duration) ? 1 : 0);

    if (my_pwm->counter++ >= my_pwm->period)
    {
      my_pwm->counter = 0;
    }
  }

  // advance clocks, adjust if needed
  delta_counter++;
  clock_counter++;
  clock_counter = modulus_i(clock_counter, CONST_TICKS_PER_SECOND * CONST_SEC_PER_DAY);
}

void rtc_init(void)
{
  DEBUG_PRINT("RTC service init\n");
  memset(&pwm[0], 0, sizeof(pwm));
  ROM_TimerLoadSet(TIMER2_BASE, TIMER_A, ROM_SysCtlClockGet() / CONST_TICKS_PER_SECOND);
}

// return tick count UNSAFE/UNRELIABLE
uint32_t tick_get(void)
{
  return (tick_counter);
}

// enable pwm
void rtc_pwm_enable(uint8_t id, uint32_t period, uint8_t duty)
{
  if ((id >= NUM_PWM) || (duty > 100))
  {
    return;
  }

  pwm[id].enabled = false;
  pwm[id].period = period;
  pwm[id].duration = (period * duty) / 100;
  pwm[id].enabled = true;
}

// disable pwm
void rtc_pwm_disable(uint8_t id)
{
  if (id >= NUM_PWM)
  {
    return;
  }

  pwm[id].enabled = false;
}

// get clock seconds via mailbox
double rtc_delta_get(void)
{
  // wait for mailbox
  while (mailbox != TIMER_MAILBOX_EMPTY) {}

  mailbox = TIMER_MAILBOX_GET_RTC_DELTA;

  // wait for interrupt service
  while (mailbox != TIMER_MAILBOX_DONE) {}

  mailbox = TIMER_MAILBOX_EMPTY;
  //DEBUG_PRINT("rtc_delta %i\n", mailbox_arg);
  return (((double) mailbox_arg) / CONST_TICKS_PER_SECOND);
}

// get clock seconds via mailbox
double rtc_get(void)
{
  // wait for mailbox
  while (mailbox != TIMER_MAILBOX_EMPTY) {}

  mailbox = TIMER_MAILBOX_GET_RTC;

  // wait for interrupt service
  while (mailbox != TIMER_MAILBOX_DONE) {}

  mailbox = TIMER_MAILBOX_EMPTY;
  return (((double) mailbox_arg) / CONST_TICKS_PER_SECOND);
}

// set clock seconds via mailbox
void rtc_set(double seconds)
{
  mailbox_arg = seconds * CONST_TICKS_PER_SECOND;

  // wait for mailbox
  while (mailbox != TIMER_MAILBOX_EMPTY) {}

  mailbox = TIMER_MAILBOX_SET_RTC;

  // wait for interrupt service
  while (mailbox != TIMER_MAILBOX_DONE) {}

  mailbox = TIMER_MAILBOX_EMPTY;
}

// reset rtc
void rtc_reset()
{
  mailbox_arg = 0;

  // wait for mailbox
  while (mailbox != TIMER_MAILBOX_EMPTY) {}

  mailbox = TIMER_MAILBOX_RESET_RTC;

  // wait for interrupt service
  while (mailbox != TIMER_MAILBOX_DONE) {}

  mailbox = TIMER_MAILBOX_EMPTY;
}

/*

// increment clock via mailbox
void sidereal_add(double seconds) {
  if (seconds == 0) {
    return;
  }

  // wait for mailbox
  while (clock_mailbox != TIMER_MAILBOX_EMPTY) {}

  clock_shadow = (seconds * CONST_TICK_PER_SECOND * CONST_SIDEREAL_SEC) / CONST_RA_SEC;
  //clock_shadow = seconds * CONST_TICK_PER_SECOND;
  clock_mailbox = TIMER_MAILBOX_INC_SIDEREAL;

  // wait for interrupt service
  while (clock_mailbox != TIMER_MAILBOX_DONE) {}

  clock_mailbox = TIMER_MAILBOX_EMPTY;
}
*/
