/*
* CFile1.c
*
* Created: 26/09/2012 7:29:42 AM
*  Author: michal
*/

#include "project.h"

#define SPI_SSI_PERIPH        SYSCTL_PERIPH_SSI2
#define SPI_SSI_PORT          SSI2_BASE
#define SPI_PERIPH            SYSCTL_PERIPH_GPIOB
#define SPI_PORT              GPIO_PORTB_BASE
#define SPI_CLK_PIN           GPIO_PB4_SSI2CLK
#define SPI_TX_PIN            GPIO_PB7_SSI2TX

//#define SPI_CS_PERIPH         SYSCTL_PERIPH_GPIOB
//#define SPI_CS_PORT           GPIO_PORTB_BASE
//#define SPI_CS0_PIN           GPIO_PIN_5
//#define SPI_CS1_PIN           GPIO_PIN_6

#define SSI_FSS               GPIO_PIN_5
#define SSI_CLK               GPIO_PIN_4
#define SSI_RX                GPIO_PIN_6
#define SSI_TX                GPIO_PIN_7

#define SPI_CLK_RATE          2000000

uint8_t g_spi_device = -1;

void spi_select(uint8_t chipid)
{
  g_spi_device = chipid;
}

void spi_init()
{
  DEBUG_PRINT("SPI stepper driver init\n");
  // Initialize the 4 pins we will need for SPI communication
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);
  // connect CS to pin B5
  //GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_5);
  //GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, GPIO_PIN_5);
  // Connect SPI to PB4 (clock) and PB7(TX)
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
  GPIOPinConfigure(GPIO_PB4_SSI2CLK);
  GPIOPinConfigure(GPIO_PB5_SSI2FSS);
  GPIOPinConfigure(GPIO_PB6_SSI2RX);
  GPIOPinConfigure(GPIO_PB7_SSI2TX);
  GPIOPinTypeSSI(GPIO_PORTB_BASE, SSI_CLK | SSI_TX | SSI_RX | SSI_FSS);
  //GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, SSI_RX);
  // Configure SSI2
  SSIClockSourceSet(SSI2_BASE, SSI_CLOCK_SYSTEM);
  //SSIConfigSetExpClk(SSI2_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_1,  SSI_MODE_MASTER, SysCtlClockGet()/2, 16);
  //SSIConfigSetExpClk(SSI2_BASE, SPI_CLK_RATE, SSI_FRF_MOTO_MODE_0,  SSI_MODE_MASTER, SPI_CLK_RATE>>1, 8);
  SSIConfigSetExpClk(SSI2_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_3,  SSI_MODE_MASTER, SPI_CLK_RATE, 8 * SPI_NUM_DEVICES);
  // Enable the SSI module.
  SSIEnable(SSI2_BASE);
}

uint8_t spi_transmit(uint8_t spidata)
{
  unsigned long spidata_in = -1;

  if (g_spi_device >= SPI_NUM_DEVICES)
  {
    return (-1);
  }

  //GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, 0);
  SSIDataPut(SSI2_BASE, spidata << (8 * g_spi_device));

  // Wait until SSI is done transferring all the data in the transmit FIFO
  while (SSIBusy(SSI2_BASE))
  {
  }

  // Hit the SSI latch, locking in the data
  //GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, GPIO_PIN_5);
  //delay_ms(1);
  SSIDataGet(SSI2_BASE, &spidata_in);
  spidata_in = spidata_in >> (8 * g_spi_device);
  //DEBUG_PRINT("spi: 0x%x -> 0x%x\n", spidata, spidata_in);
  return (spidata_in);
}

void spi_test()
{
  unsigned int i = 0, rc;
  unsigned char reg;
  lcd_init();
  spi_init();
  DEBUG_PRINT("SPI test:");

  for (i = 0; i < 20; i++)
  {
    reg = 12;//PA_ENABLE;
    reg |= 0x80; /* set MSB to 1 for read access */
    spi_select(0);
    rc = spi_transmit(reg);
    DEBUG_PRINT("%i(0x%x) ", i, rc);
    delay_ms(50);
  }

  DEBUG_PRINT("\n");
}
