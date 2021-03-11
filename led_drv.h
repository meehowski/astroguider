/*
 * IncFile1.h
 *
 * Created: 16/12/2012 4:40:37 PM
 *  Author: michal
 */


#ifndef OUTPUT_H_
#define OUTPUT_H_

#include "project.h"

#define RED_LED            GPIO_PIN_1
#define BLUE_LED           GPIO_PIN_2
#define GREEN_LED          GPIO_PIN_3

void led_init(void);
void led_state(uint8_t id, uint8_t state);

#endif /* OUTPUT_H_ */
