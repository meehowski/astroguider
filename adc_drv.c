/*
* CFile1.c
*
* Created: 30/10/2012 6:09:01 AM
*  Author: michal
*/

#include "project.h"

static unsigned long adc_val[8] = {1, 1, 1, 1, 0, 0, 0, 0};

static void adc_service()
{
  // Clear the ADC interrupt flag.
  ADCIntClear(ADC0_BASE, 0);
  //DEBUG_PRINT ("&");
  // Read ADC Value.
  ADCSequenceDataGet(ADC0_BASE, 0, (long unsigned int *)&adc_val[0]);
}

uint16_t adc_get(uint8_t channel)
{
  return adc_val[channel];
}

void adc_start(int8_t channel)
{
  //DEBUG_PRINT ("In adc_start\n");
  ADCProcessorTrigger(ADC0_BASE, 0);
}

void adc_init()
{
  DEBUG_PRINT("ADC driver init\n");
  // enable the ADCn peripherals:
  SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
  // GPIO port E needs to be enabled so these pins can be used
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
  // Select the analog ADC function for these pins
  GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_7 | GPIO_PIN_6 | GPIO_PIN_5 | GPIO_PIN_4);
  ADCSequenceDisable(ADC0_BASE, 0);
  //ADCSequenceDisable(ADC1_BASE, 0);
  ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
  //ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
  //ADCSequenceConfigure(ADC0_BASE, 2, ADC_TRIGGER_PROCESSOR, 0);
  //ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
  // Configure step 0-3 on sequence 0.  Sample channel 0 (ADC_CTL_CH0) in
  // single-ended mode (default) and configure the interrupt flag
  // (ADC_CTL_IE) to be set when the sample step 3 is done.  Tell the ADC logic
  // that this is the last conversion on sequence 1 (ADC_CTL_END).
  ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_CH0);
  ADCSequenceStepConfigure(ADC0_BASE, 0, 1, ADC_CTL_CH1);
  ADCSequenceStepConfigure(ADC0_BASE, 0, 2, ADC_CTL_CH2);
  ADCSequenceStepConfigure(ADC0_BASE, 0, 3, ADC_CTL_CH3);
  ADCSequenceStepConfigure(ADC0_BASE, 0, 4, ADC_CTL_TS);
  ADCSequenceStepConfigure(ADC0_BASE, 0, 5, ADC_CTL_TS | ADC_CTL_IE | ADC_CTL_END);
  // Since sample sequence 0 is now configured, it must be enabled.
  ADCSequenceEnable(ADC0_BASE, 0);
  //GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_6, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_ANALOG);
  ADCReferenceSet(ADC0_BASE, ADC_REF_EXT_3V);
  // Clear the interrupt status flag.  This is done to make sure the
  // interrupt flag is cleared before we sample.
  ADCIntClear(ADC0_BASE, 0);
  SysCtlADCSpeedSet(SYSCTL_ADCSPEED_125KSPS);
  ADCHardwareOversampleConfigure(ADC0_BASE, 64);
  ADCIntRegister(ADC0_BASE, 0, adc_service);
  ADCIntEnable(ADC0_BASE, 0);
}

void adc_test(void)
{
  char buffer[40];
  //adc_init();
  //lcd_init();
  lcd_acquire();
  lcd_clear();

  while (1)
  {
    adc_start(-1);

    for (int i = 0; i < 6; i++)
    {
      delay_ms(10);
      snprintf(buffer, sizeof(buffer), "%4i ", adc_get(i));
      lcd_write_str((i % 3) * 6, (i / 3), buffer);
    }
  }
}
