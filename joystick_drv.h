/*
* IncFile1.h
*
* Created: 30/10/2012 9:10:24 AM
*  Author: michal
*/


#ifndef JOYSTICK_H_
#define JOYSTICK_H_

#define NUM_JOYSTICS              2
#define JOYSTICK_END_MARGIN       64 /* adc dead space at each end where return value is JOYSTICK_MAX */
#define JOYSTICK_MIDDLE_MARGIN    64 /* adc dead space at middle where return value is 0 */
#define JOYSTICK_MAX            100 /* max signed value of joystick value return */

int8_t joystick_get(uint8_t channel);
void joystick_test(void);
void joystick_init(void);

#endif /* JOYSTICK_H_ */
