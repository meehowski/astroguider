/*
* IncFile1.h
*
* Created: 29/09/2012 4:19:47 PM
*  Author: michal
*/


#ifndef EEPROM_H_
#define EEPROM_H_

#define EEPROM_STORE_LOCATION     0

void eeprom_init(void);
uint32_t eeprom_write_block(config_t *config, uint32_t offset, uint32_t size);
uint32_t eeprom_read_block(config_t *config, uint32_t offset, uint32_t size);

#endif /* EEPROM_H_ */

