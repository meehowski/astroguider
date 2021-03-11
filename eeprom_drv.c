/*
* IncFile1.c
*
* Created: 29/09/2012 4:19:47 PM
*  Author: michal
*/

#include "project.h"

void eeprom_init(void)
{
  DEBUG_PRINT("EEPROM init ...");
  SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
  EEPROMInit();
  DEBUG_PRINT(" detected size %i bytes, %i blocks", EEPROMSizeGet(), EEPROMBlockCountGet());
  DEBUG_PRINT(" ... done\n");
}

static void print_hex(uint8_t *mem, uint32_t len)
{
  uint32_t i;

  for (i = 0; i < len; i++)
  {
    DEBUG_PRINT("%02x ", mem[i]);

    if ((i & 0xf) == 0xf)
    {
      DEBUG_PRINT("\n");
    }
  }

  if ((i & 0xf) != 0x0)
  {
    DEBUG_PRINT("\n");
  }
}

uint32_t eeprom_write_block(config_t *config, uint32_t offset, uint32_t size)
{
  DEBUG_PRINT("EEPROM write %i bytes at 0x%x:\n", size, offset);
  EEPROMProgram((unsigned long *)config, offset, size);
  print_hex((uint8_t *)config, size);
  return 0;
}

uint32_t eeprom_read_block(config_t *config, uint32_t offset, uint32_t size)
{
  DEBUG_PRINT("EEPROM read %i bytes from 0x%x:\n", size, offset);
  EEPROMRead((unsigned long *)config, offset, size);
  print_hex((uint8_t *)config, size);
  return 0;
}

