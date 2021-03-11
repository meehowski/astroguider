/*
* IncFile1.h
*
* Created: 29/09/2012 4:31:31 PM
*  Author: michal
*/


#ifndef TIMERS_H_
#define TIMERS_H_

#define CONST_TICKS_PER_SECOND  320
#define NUM_PWM 2 // SW pwm generators

enum
{
  TIMER_MAILBOX_EMPTY = 0,
  TIMER_MAILBOX_GET_RTC_DELTA,
  TIMER_MAILBOX_SET_RTC,
  TIMER_MAILBOX_GET_RTC,
  TIMER_MAILBOX_RESET_RTC,
  TIMER_MAILBOX_DONE
};

uint32_t tick_get(void);
double rtc_delta_get(void);
double rtc_get(void);
void rtc_set(double seconds);
void rtc_service();
void rtc_init();
void rtc_reset();
void rtc_pwm_enable(uint8_t id, uint32_t period, uint8_t duty);
void rtc_pwm_disable(uint8_t id);

#endif /* TIMERS_H_ */
