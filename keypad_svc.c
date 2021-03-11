//*****************************************************************************
// switch_task.c - A simple switch task to process the buttons.
//*****************************************************************************

#include "project.h"

// The stack size for the display task.
#define SWITCHTASKSTACKSIZE        128         // Stack size in words

extern xQueueHandle g_pKeyQueue;

// This task reads the buttons' state and passes this information to LEDTask.
static void SwitchTask(void *pvParameters)
{
  portTickType ulLastTime;
  unsigned long ulSwitchDelay = 25;
  unsigned short ucCurButtonState, ucPrevButtonState;
  unsigned short ucMessage = 0;
  ucCurButtonState = ucPrevButtonState = 0;
  // Initialize the buttons
  adc_init();
  joystick_init();
  keypad_init(); // ButtonsInit();
  // Get the current tick count.
  ulLastTime = xTaskGetTickCount();

  // Loop forever.
  while (1)
  {
    // Poll the debounced state of the buttons.
    keypad_ascan();
    keypad_dscan();
    ucCurButtonState = keypad_get(); //ButtonsPoll(0, 0);

    //
    // Check if previous debounced state is equal to the current state.
    //
    if (ucCurButtonState != ucPrevButtonState)
    {
      ucPrevButtonState = ucCurButtonState;

      // Check to make sure the change in state is due to button press
      // and not due to button release.
      if ((ucCurButtonState) != 0)
      {
        if ((ucCurButtonState & kButtonRED) != 0)
        {
          ucMessage = kButtonRED;
          DEBUG_PRINT("RED Button is pressed.\n");
        }
        else if ((ucCurButtonState & kButtonBLUE) != 0)
        {
          ucMessage = kButtonBLUE;
          DEBUG_PRINT("BLUE Button is pressed.\n");
        }
        else if ((ucCurButtonState & kButtonBLACK) != 0)
        {
          ucMessage = kButtonBLACK;
          DEBUG_PRINT("BLACK Button is pressed.\n");
        }

        // Pass the value of the button pressed
        if (xQueueSend(g_pKeyQueue, &ucMessage, portMAX_DELAY) !=
            pdPASS)
        {
          // Error. The queue should never be full. If so print the
          // error message on UART and wait for ever.
          DEBUG_PRINT("\nQueue full. This should never happen.\n");
          //while (1)
          //{
          //}
        }

#if 0
        DEBUG_PRINT("\nKeypress 0x%x\n", ucMessage);
#endif
      }
    }

    // Wait for the required amount of time to check back.
    vTaskDelayUntil(&ulLastTime, ulSwitchDelay / portTICK_RATE_MS);
  }
}

// Initializes the switch task.
unsigned long keypad_svc_init(void)
{
  DEBUG_PRINT("KEYPAD service init\n");
  // Unlock the GPIO LOCK register for Right button to work.
  //HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY_DD;
  //HWREG(GPIO_PORTF_BASE + GPIO_O_CR) = 0xFF;
  DEBUG_PRINT("Switch task initializing ... ");

  // Create the switch task.
  if (xTaskCreate(SwitchTask, "SWITCH",
                  SWITCHTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
                  PRIORITY_SWITCH_TASK, NULL) != pdTRUE)
  {
    return (1);
  }

  DEBUG_PRINT("done\n");
  // Success.
  return (0);
}
