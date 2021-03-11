#include <string.h>
#include <stdio.h>
#include <avr/io.h>
#include "config.h"

#define uchar unsigned char
#define uint  unsigned int

#define RS_CLR  PORTF &= ~(1 << PF2)
#define RS_SET  PORTF |= (1 << PF2)

#define RW_CLR  PORTF &= ~(1 << PF1)
#define RW_SET  PORTF |= (1 << PF1)

#define EN_CLR  PORTF &= ~(1 << PF0)
#define EN_SET  PORTF |= (1 << PF0)

#define PSB_CLR PORTE &= ~(1 << PE2)
#define PSB_SET PORTE |= (1 << PE2)

#define RST_CLR PORTF &= ~(1 << PF3)
#define RST_SET PORTF |= (1 << PF3)

#define LOW   0
#define HIGH    1

#define CLEAR_SCREEN  0x01
#define AC_INIT       0x02
#define CURSE_ADD   0x06
#define FUN_MODE    0x30
#define DISPLAY_ON    0x0c
#define DISPLAY_OFF   0x08
#define CURSE_DIR   0x14
#define SET_CG_AC   0x40
#define SET_DD_AC   0x80

#define Data_IO         PORTC
#define Data_DDR        DDRC

void system_init_lcd12864()
{
  Data_IO = 0xFF;
  Data_DDR = 0xFF;
  PORTF = 0xFF;
  DDRF = 0xFF;
  PORTE = 0xFF;
  DDRE = 0xFF;
  PSB_SET;
  RST_SET;
}


void LCD12864_write_com(unsigned char com)
{
  RS_CLR;
  RW_CLR;
  EN_SET;
  Data_IO = com;
  delay_ms(5);
  EN_CLR;
}

void LCD12864_write_data(unsigned char data)
{
  RS_SET;
  RW_CLR;
  EN_SET;
  Data_IO = data;
  delay_ms(5);
  EN_CLR;
}

void LCD12864_clear(void)
{
  LCD12864_write_com(0x01);
  delay_ms(5);
}

void DisplayCgrom(uchar addr, uchar *hz)
{
  LCD12864_write_com(addr);
  delay_ms(5);

  while (*hz != '\0')
  {
    LCD12864_write_data(*hz);
    hz++;
    delay_ms(5);
  }
}

void Display(void)
{
  DisplayCgrom(0x80, "�����͵��ӻ�ӭ��");
  DisplayCgrom(0x88, "��:jingyehanxing");
  DisplayCgrom(0x90, "www.avrgcc.com  ");
  DisplayCgrom(0x98, "�绰057487476870");
}

void LCD12864_init(void)
{
  DEBUG_PRINT("LCD12864 driver init\n");
  LCD12864_write_com(FUN_MODE);
  delay_ms(5);
  LCD12864_write_com(FUN_MODE);
  delay_ms(5);
  LCD12864_write_com(DISPLAY_ON);
  delay_ms(5);
  LCD12864_write_com(CLEAR_SCREEN);
  delay_ms(5);
}

int main_lcd12864(void)
{
  system_init_lcd12864();
  delay_ms(100);
  LCD12864_init();
  LCD12864_clear();

  while (1)
  {
    Display();
  }
}

