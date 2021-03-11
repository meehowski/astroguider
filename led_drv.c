/*
 * CFile1.c
 *
 * Created: 16/12/2012 4:39:25 PM
 *  Author: michal
 */

#include "project.h"

#define LED_GPIO_PERIPH       SYSCTL_PERIPH_GPIOF
#define LED_GPIO_BASE         GPIO_PORTF_BASE
#define ALL_LEDS              RED_LED | BLUE_LED | GREEN_LED

void led_init(void)
{
  DEBUG_PRINT("LED driver init\n");
  SysCtlPeripheralEnable(LED_GPIO_PERIPH);
  GPIOPinTypeGPIOOutput(LED_GPIO_BASE, ALL_LEDS);
  GPIOPinWrite(LED_GPIO_BASE, ALL_LEDS, 0);
#if 0
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
  GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_2);
  GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_PIN_2);
#endif
}

void led_state(uint8_t id, uint8_t state)
{
  GPIOPinWrite(LED_GPIO_BASE, id, (state == 0) ? 0 : id);
}

