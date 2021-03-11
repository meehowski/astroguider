/*
* CFile1.c
*
* Created: 30/09/2012 6:15:35 AM
*  Author: michal
*/

#ifndef SPI_H_
#define SPI_H_

#define SPI_NUM_DEVICES 1

void spi_init();
void spi_select(uint8_t chipid);
uint8_t spi_transmit(uint8_t data);
void spi_test();

#endif /* SPI_H_ */
