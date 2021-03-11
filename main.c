#include "project.h"

// The mutex that protects concurrent access of UART from multiple tasks.
xSemaphoreHandle g_pUARTSemaphore = 0;

// The error routine that is called if the driver library encounters an error.
#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
}

#endif

// This hook is called by FreeRTOS when an stack overflow error is detected.
void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName)
{
  // This function can not return, so loop forever.  Interrupts are disabled
  // on entry to this function, so no processor interrupts will interrupt
  // this loop.
  while (1)
  {
  }
}

// Initialize FreeRTOS and start the initial set of tasks.
int main(void)
{
  ROM_FPUEnable();
  ROM_FPUStackingEnable();
  ROM_IntMasterEnable();
  // Set the clocking to run at 66.67 MHz from the PLL.
  ROM_SysCtlClockSet(SYSCTL_SYSDIV_3 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
                     SYSCTL_OSC_MAIN);
  // Initialize the UART and configure it for 115,200, 8-N-1 operation.
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);
  ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  UARTStdioInit(0);
  // Create a mutex to guard the UART.
  g_pUARTSemaphore = xSemaphoreCreateMutex();
// Print demo introduction.
  //PRINT ("\033[2JStellaris EK-LM4F120 FreeRTOS (sysclk=%u)\n\n", ROM_SysCtlClockGet());
  PRINT("\nStellaris EK-LM4F120 FreeRTOS (sysclk=%u)\n\n", ROM_SysCtlClockGet());
  PRINT(" A                _______________\n");
  PRINT("  S           ==c(___(o(______(_()\n");
  PRINT("   T                  \\=\\\n");
  PRINT(" G  R                  )=\\\n");
  PRINT("  U  O                //|\\\\\n");
  PRINT("   I                 //|| \\\\\n");
  PRINT("    D               // ||  \\\\\n");
  PRINT("     E             //  ||   \\\\\n");
  PRINT("      R           //         \\\\\n");
  PRINT("\n");
#if 0
#if 0
  PRINT("            \\\\\\\\\\\\////// \n");
  PRINT("             \\\\((()))//  \n");
  PRINT("  WOE-      /   \\\\//   \\  \n");
  PRINT("   -BOT   _|     \\/     |_ \n");
  PRINT("         ((| \\___  ___/ |)) \n");
  PRINT(" KILLS    \\ > -o-\\/-o- < / \n");
  PRINT("   ALL     |     ..     | \n");
  PRINT("  HUMANS    )   ____   (  \n");
  PRINT(" GOOD    _,'\\          /`._\n");
  PRINT("      ,-'    `-.____.-'    `. \n");
  PRINT("     /         |    |        \\ \n");
  PRINT("    |_|_|_|_|__|____|___|_|_|_| \n");
  PRINT("\n");
#else
  PRINT("       -+-             .___. \n");
  PRINT("     .--+--.         _/__ /|\n");
  PRINT("     ||[o o]        |____|||\n");
  PRINT("     || ___|         |O O ||\n");
  PRINT("   __`-----'_      __|++++|/__\n");
  PRINT("  |\\ ________\\    /_________ /|\n");
  PRINT("  ||   WOE-  ||   |  nukes   ||\n");
  PRINT("  |||  BOT   ||   || humans |||\n");
  PRINT("  \\||  v1.0  ||   || good   ||/\n");
  PRINT("   VV========VV   VV========VV\n");
  PRINT("   ||       |      |   |   ||\n");
  PRINT("   ||       |      |   |   ||\n");
  PRINT("   \\|___|___|      |___|___|/\n");
  PRINT("     \\___\\___\\     /___/___/\n");
  PRINT("\n");
#endif
#endif
#if 0

  // Create the LED task.
  if (LED_task_init() != 0)
  {
    while (1)
    {
    }
  }

#endif
#if 0

  if (keypad_svc_init() != 0)
  {
    while (1)
    {
    }
  }

#endif
  timer_init();
  guider_svc_init();

  //platform_init();

  //stepper_speed(0, 100.0f);

  // Create the SHELL service.
  if (shell_svc_init() != 0)
  {
    while (1)
    {
    }
  }

  DEBUG_PRINT("Starting scheduler\n");
  //ROM_IntMasterDisable();
  // Start the scheduler.  This should not return.
  vTaskStartScheduler();

  // In case the scheduler returns for some reason, print an error and loop
  // forever.

  while (1)
  {
  }
}
