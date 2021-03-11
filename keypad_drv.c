#include "project.h"

#define BUTTONS_GPIO_PERIPH     SYSCTL_PERIPH_GPIOF
#define BUTTONS_GPIO_BASE       GPIO_PORTF_BASE

#define BUTTON_1                GPIO_PIN_4
#define BUTTON_2                GPIO_PIN_0
#define BUTTON_3                GPIO_PIN_5

#define ALL_KEYPAD_BUTTONS             (BUTTON_1 | BUTTON_2 | BUTTON_3)

static uint16_t keyID = kButtonNoKey;

void keypad_init(void)
{
  DEBUG_PRINT("KEYPAD driver init\n");
  // Enable the GPIO port to which the pushbuttons are connected.
  ROM_SysCtlPeripheralEnable(BUTTONS_GPIO_PERIPH);
  // Unlock PF0 so we can change it to a GPIO input
  // Once we have enabled (unlocked) the commit register then re-lock it
  // to prevent further changes.  PF0 is muxed with NMI thus a special case.
  HWREG(BUTTONS_GPIO_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY_DD;
  HWREG(BUTTONS_GPIO_BASE + GPIO_O_CR) |= 0x01;
  HWREG(BUTTONS_GPIO_BASE + GPIO_O_LOCK) = 0;
  // Set each of the button GPIO pins as an input with a pull-up.
  ROM_GPIODirModeSet(BUTTONS_GPIO_BASE, ALL_KEYPAD_BUTTONS, GPIO_DIR_MODE_IN);
  ROM_GPIOPadConfigSet(BUTTONS_GPIO_BASE, ALL_KEYPAD_BUTTONS,
                       GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}

uint16_t keypad_ascan(void)
{
  int16_t joy_val = 0, j1, j2;
  /* analog key scan */
  adc_start(-1);
  keyID &= (~kButtonAMask); /* erase analog keys */
  //return 0; //FIXME
  j1 = joystick_get(0);
  j2 = joystick_get(1);

  /* prevent multi-key detection - pick largest joystick deflection */
  if (abs(j1) > abs(j2))
  {
    j2 = 0;
  }
  else
  {
    j1 = 0;
  }

  /* convert to key value */
  joy_val = j1;

  if (joy_val > 0)
  {
    keyID |= kButtonRight;
  }
  else if (joy_val < 0)
  {
    keyID |= kButtonLeft;
  }

  if (joy_val == JOYSTICK_MAX)
  {
    keyID |= kButtonPgRight;
  }
  else if (joy_val == -JOYSTICK_MAX)
  {
    keyID |= kButtonPgLeft;
  }

  joy_val = j2;

  if (joy_val > 0)
  {
    keyID |= kButtonUp;
  }
  else if (joy_val < -0)
  {
    keyID |= kButtonDown;
  }

  if (joy_val == JOYSTICK_MAX)
  {
    keyID |= kButtonPgUp;
  }
  else if (joy_val == -JOYSTICK_MAX)
  {
    keyID |= kButtonPgDown;
  }

  return (keyID);
}

uint16_t keypad_dscan(void)
{
  /* digital key scan */
  unsigned long ulDelta;
  unsigned long ulData;
  static unsigned char ucSwitchClockA = 0;
  static unsigned char ucSwitchClockB = 0;
  static unsigned char g_ucButtonStates = 0;
  /* internal pullups: 0=closed, 1=open */
  //keyBuffer[keyBufferPointer % KEY_FILTER_BUFFER]
  ulData = ROM_GPIOPinRead(BUTTONS_GPIO_BASE, ALL_KEYPAD_BUTTONS) ^ALL_KEYPAD_BUTTONS;
  // Determine the switches that are at a different state than the debounced
  // state.
  ulDelta = ulData ^ g_ucButtonStates;
  // Increment the clocks by one.
  ucSwitchClockA ^= ucSwitchClockB;
  ucSwitchClockB = ~ucSwitchClockB;
  // Reset the clocks corresponding to switches that have not changed state.
  ucSwitchClockA &= ulDelta;
  ucSwitchClockB &= ulDelta;
  // Get the new debounced switch state.
  g_ucButtonStates &= ucSwitchClockA | ucSwitchClockB;
  g_ucButtonStates |= (~(ucSwitchClockA | ucSwitchClockB)) & ulData;
  // Determine the switches that just changed debounced state.
  ulDelta ^= (ucSwitchClockA | ucSwitchClockB);
  keyID &= (~kButtonDMask); /* erase digital keys */

  if ((g_ucButtonStates & BUTTON_1) != 0)
  {
    keyID |= kButtonRED;
  }

  if ((g_ucButtonStates & BUTTON_2) != 0)
  {
    keyID |= kButtonBLUE;
  }

  if ((g_ucButtonStates & BUTTON_3) != 0)
  {
    keyID |= kButtonBLACK;
  }

  //DEBUG_PRINT ("keypad_dscan %x, key %x\n", g_ucButtonStates, keyID);
  return (keyID);
}

uint16_t keypad_get(void)
{
  return (keyID);
}

uint8_t keypad_iskey(uint16_t key)
{
#if 0

  if ((keyID & key) != 0)
  {
    DEBUG_PRINT("keypad_iskey %x, key %x\n", key, keyID);
  }

#endif
  return ((keyID & key) != 0);
}

void keypad_test(void)
{
  char buffer[32];
  rtc_init();
  lcd_init();
  adc_init();
  joystick_init();
  keypad_init();
  lcd_acquire();
  lcd_clear();

  while (1)
  {
    uint16_t j = keypad_get();
    sprintf(buffer, "K:%04x %08x", j, (int)(tick_get() / CONST_TICKS_PER_SECOND));
    lcd_write_str(0, 0, buffer);
    sprintf(buffer, "%4i%4i%4i%4i", joystick_get(0), joystick_get(1), joystick_get(2), joystick_get(3));
    lcd_write_str(0, 1, buffer);
    delay_ms(50);
  }
}
