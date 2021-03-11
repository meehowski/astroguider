/*
* IncFile1.h
*
* Created: 22/09/2012 2:50:08 PM
*  Author: michal
*/

#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> // tolower
#include <stdint.h>
#include <math.h>
#include <inttypes.h>
#include <errno.h>
#include <float.h>
#include <stdarg.h> // va_...

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <string.h>
#include <queue.h>
#include <semphr.h>

#include <inc/hw_ints.h>
#include <inc/hw_memmap.h>
#include <inc/hw_types.h>
#include <inc/hw_gpio.h>
#include <driverlib/debug.h>
#include <driverlib/fpu.h>
#include <driverlib/gpio.h>
#include <driverlib/interrupt.h>
#include <driverlib/pin_map.h>
#include <driverlib/rom.h>
#include <driverlib/sysctl.h>
#include <driverlib/timer.h>
#include <driverlib/adc.h>
#include <driverlib/rom.h>
#include <driverlib/uart.h>
#include <driverlib/ssi.h>
#include <driverlib/eeprom.h>
#include <utils/uartstdio.h>
#include <drivers/buttons.h>

#include "isqrt_lib.h"
#include "astro_lib.h"
#include "pid_lib.h"

#include "debug.h"
#include "lcd1602_drv.h"
#include "adc_drv.h"
#include "joystick_drv.h"
#include "keypad_drv.h"
#include "dspin_drv.h"
#include "led_drv.h"
#include "spi_drv.h"
#include "timer_drv.h"
#include "rtc_svc.h"
#include "shell_svc.h"
#include "stepper_hw_drv.h"
#include "stepper_sw_drv.h"
#include "object_db_lib.h"
#include "menu_lib.h"
#include "guider_svc.h"
#include "eeprom_drv.h"
#include "platform_drv.h"
#include "keypad_svc.h"

#define CONST_PROJ_VERSION        0x00010000
#define CONST_PROJ_BUILD          0x00010000
#define CONST_PROJ_CFG_VERSION    0x00010006

#define delay_ms(x)     { portTickType ulWakeTime; ulWakeTime = xTaskGetTickCount(); vTaskDelayUntil(&ulWakeTime, (x)); }
#define DEBUG_PRINT_FLOAT(fv) { double i, f; f=modf((fv), &i); DEBUG_PRINT("%c%i.%08i", ((fv)<0?'-':'+'), (int32_t)abs(i), (int32_t)(abs(f*100000000.0))); }

#define ABS(a) ((a) > 0 ? (a) : -(a))
#define SIGN(a) ((a) >= 0 ? +1 : -1)
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define PRINT(...)     { xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY); UARTprintf(__VA_ARGS__); xSemaphoreGive(g_pUARTSemaphore); }
#define DEBUG_PRINT    PRINT

extern xSemaphoreHandle g_pUARTSemaphore;

#endif /* CONFIG_H_ */

