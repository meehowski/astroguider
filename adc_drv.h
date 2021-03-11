/*
* IncFile1.h
*
* Created: 30/10/2012 6:09:29 AM
*  Author: michal
*/

#ifndef ADC_H_
#define ADC_H_

#include "project.h"

#define NUM_ADC 8

void adc_init();
void adc_start(int8_t channel);
uint16_t adc_get(uint8_t channel);
void adc_test(void);

#endif /* INCFILE1_H_ */
