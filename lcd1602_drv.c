#include "project.h"

xSemaphoreHandle g_pLCDSemaphore = 0;

/* stellaris launchpad board */

#define DEFAULT_DELAY     50
#define CMD_WRITE_DELAY   5
#define DATA_WRITE_DELAY  1
#define EN_DELAY          1

#define RS(x)   GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_6, ((x)&1) << 6)
#define RW(x)   //GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, ((x)&1) << 3)
#define EN(x)   GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_7, ((x)&1) << 7)
#define DATA(x) GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, ((x)&0xf) << 4)

uint8_t LCDBacklight = 0;
uint8_t LCDBuffer[LCD_WIDTH][LCD_HEIGHT];

void lcd1602_acquire(void)
{
  if (g_pLCDSemaphore != 0)
  {
    xSemaphoreTake(g_pLCDSemaphore, portMAX_DELAY);
  }
}

void lcd1602_release(void)
{
  if (g_pLCDSemaphore != 0)
  {
    xSemaphoreGive(g_pLCDSemaphore);
  }
}

void lcd1602_write_com(unsigned char com)
{
  RW(0);
  RS(0);
  DATA((com) >> 4);
  EN(1);
  delay_ms(EN_DELAY);
  EN(0);
  DATA((com) & 0xf);
  EN(1);
  delay_ms(EN_DELAY);
  EN(0);
  delay_ms(CMD_WRITE_DELAY);
}

void lcd1602_write_data(unsigned char data)
{
  RW(0);
  RS(1);
  DATA((data) >> 4);
  EN(1);
  delay_ms(EN_DELAY);
  EN(0);
  DATA((data) & 0xf);
  EN(1);
  delay_ms(EN_DELAY);
  EN(0);
  delay_ms(DATA_WRITE_DELAY);
}

void lcd1602_clear(void)
{
  lcd1602_write_com(0x01);
  delay_ms(DEFAULT_DELAY);
  memset(&LCDBuffer[0], 0, sizeof(LCDBuffer));
  //DEBUG_PRINT ("\033[2J");
}

void lcd1602_write_cchar(uint8_t start_char, uint8_t len, const unsigned char *data)
{
  // write cgram
  lcd1602_write_com(0x40 + (start_char << 3));

  while (len-- > 0)
  {
    lcd1602_write_data(*data);
    data ++;
  }
}

void lcd1602_write_strn(uint8_t x, uint8_t y, const char *s, uint8_t n)
{
  //DEBUG_PRINT (s); // debug
  //DEBUG_PRINT ("\n"); // debug

  // check content
  if ((x < LCD_WIDTH) && (y < LCD_HEIGHT)
      && (memcmp(s, &LCDBuffer[x][y], MIN(LCD_WIDTH - x, MAX(n, strlen(s)))) == 0))
  {
    return; // same content
  }

  // ddram write
  lcd1602_write_com(0x80 + (y << 6) + x);

  while (*s && (x < LCD_WIDTH) && (y < LCD_HEIGHT))
  {
    lcd1602_write_data(*s);
    LCDBuffer[x++][y] = *s++;
  }

  // fill the rest with spaces
  while (x < LCD_WIDTH)
  {
    lcd1602_write_data(' ');
    LCDBuffer[x++][y] = ' ';
  }
}

void lcd1602_write_str(uint8_t x, uint8_t y, const char *s)
{
  lcd1602_write_strn(x, y, s, 0);
}

void lcd1602_write_char(unsigned char x, unsigned char y, unsigned char data)
{
  // ddram write
  lcd1602_write_com(0x80 + (y << 6) + x);
  lcd1602_write_data(data);
}

/*
const uint8_t customChars_levels[8*8] PROGMEM = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,
  0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,
  0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,
  0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,
  0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,
  0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,
  0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
};

const uint8_t customChars_default[8*8] PROGMEM = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // n/a - variable mode char
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // n/a - variable battery char
  0x04,0x0e,0x11,0x00,0x00,0x11,0x0e,0x04, // up/down
  0x0, 0xe, 0x15, 0x15, 0x17, 0x11, 0xe, 0x0, // clock
  0x1, 0x2, 0x4, 0xe, 0x1f, 0x1f, 0x1f, 0xe,  // bomb
  0x0, 0x2, 0x4, 0xe, 0x1f, 0x1f, 0x1f, 0xe,  // bomb
  0x0, 0x0, 0x4, 0xe, 0x1f, 0x1f, 0x1f, 0xe,  // bomb
  0x0, 0x0, 0x0, 0xe, 0x1f, 0x1f, 0x1f, 0xe,  // bomb
};

const uint8_t symbols_mode[9][8] PROGMEM  = {
{0x0, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x0}, // stopped = 0
{0x3, 0xe, 0x1c, 0x1c, 0x1c, 0x1c, 0xe, 0x3}, // moon = 1
{0x0, 0x0, 0x4, 0x15, 0x1b, 0x15, 0x4, 0x0}, // star = 2
{0x0, 0xe, 0x1b, 0x11, 0x11, 0x1b, 0xe, 0x0}, // planet = 3
{0x0, 0xe, 0x1f, 0x1f, 0x1f, 0x1f, 0xe, 0x0}, // sun = 4
{0x4, 0xe, 0x1f, 0xa, 0xa, 0x1f, 0xe, 0x4}, // slew = 5
{0x0, 0xe, 0x15, 0x15, 0x17, 0x11, 0xe, 0x0}, // clock = 6
{0x1b, 0x11, 0x0, 0x4, 0x0, 0x11, 0x1b, 0x0}, // goto target = 7
{0x1, 0x2, 0x4, 0xe, 0x1f, 0x1f, 0x1f, 0xe}, // bomb = 8
};

const uint8_t symbols_battery[7][8] PROGMEM  = {
{0xe, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f}, // battery ok
{0xe, 0x1f, 0x11, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f}, // battery ..
{0xe, 0x1f, 0x11, 0x11, 0x1f, 0x1f, 0x1f, 0x1f}, // battery ..
{0xe, 0x1f, 0x11, 0x11, 0x11, 0x1f, 0x1f, 0x1f}, // battery ..
{0xe, 0x1f, 0x11, 0x11, 0x11, 0x11, 0x1f, 0x1f}, // battery ..
{0xe, 0x1f, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1f}, // battery ..
{0xe, 0x1f, 0x11, 0x19, 0x15, 0x13, 0x11, 0x1f}  // battery critical
};

void lcd1602_setbattery_char(uint8_t cchar, int8_t value) {
  static int8_t current_value = -1;

  if (current_value != value) {

    lcd1602_write_cchar(cchar, 8, &symbols_battery[value][0]);
    current_value = value;
  }
}

void lcd1602_setmode_char(uint8_t cchar, int8_t mode) {
  static int8_t current_mode = -1;

  if (current_mode != mode) {
    lcd1602_write_cchar(cchar, 8, &symbols_mode[mode][0]);
    current_mode = mode;
  }
}
*/

void lcd1602_custom_char(uint8_t cchar, int8_t val, const uint8_t *cchar_array)
{
  static int8_t current_val[8] = { -1, -1, -1, -1, -1, -1, -1, -1};
  cchar = cchar % 8;

  if (current_val[cchar] != val)
  {
    lcd1602_write_cchar(cchar, 8, &cchar_array[val * 8]);
    current_val[cchar] = val;
  }
}

void lcd1602_test()
{
  //lcd1602_init();
  lcd_acquire();
  uint8_t i = 0, j = 0, l = 0, k = 0;
  char buffer[LCD_WIDTH + 1];
  lcd1602_clear();

  while (1)
  {
    sprintf(buffer, "%02i:%02i CHAR=%3i", i, j++, k);

    if (j == 60)
    {
      j = 0;
      i++;
    }

    if (i == 24)
    {
      i = 0;
    }

    lcd1602_write_str(0, 0, buffer);

    for (l = 0; l < LCD_WIDTH; l++)
    {
      buffer[l] = (k + l) % 256;
    }

    buffer[l] = 0;
    lcd1602_write_str(0, 1, buffer);

    if (keypad_iskey(kButtonUp))
    {
      k -= LCD_WIDTH;
    }

    if (keypad_iskey(kButtonDown))
    {
      k += LCD_WIDTH;
    }

    if (keypad_iskey(kButtonLeft))
    {
      k--;
    }

    if (keypad_iskey(kButtonRight))
    {
      k++;
    }

    //delay_ms (100);
  }
}

void lcd1602_init(void)
{
  DEBUG_PRINT("LCD1602 driver init\n");
  g_pLCDSemaphore = xSemaphoreCreateMutex();
  xSemaphoreGive(g_pLCDSemaphore);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
  // Unlock PD7 so we can change it to a GPIO input
  // Once we have enabled (unlocked) the commit register then re-lock it
  // to prevent further changes.  PD7 is muxed with NMI thus a special case.
  //
  HWREG(GPIO_PORTD_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY_DD;
  HWREG(GPIO_PORTD_BASE + GPIO_O_CR) |= 0x80;
  HWREG(GPIO_PORTD_BASE + GPIO_O_LOCK) = 0;
  GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, 0x3 << 6);  // rs=pd6/rw=na/e=pd7
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
  GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, 0xf << 4);  // data, pc4-pc7
  delay_ms(DEFAULT_DELAY);
  lcd1602_write_com(0x33);  // 4bit mode
  lcd1602_write_com(0x32);  // 4bit mode
  delay_ms(DEFAULT_DELAY);
  lcd1602_write_com(0x28);
  delay_ms(DEFAULT_DELAY);
  lcd1602_write_com(0x08);  // disp off
  delay_ms(DEFAULT_DELAY);
  lcd1602_write_com(0x01);  // clr screen
  delay_ms(DEFAULT_DELAY);
  lcd1602_write_com(0x06);  // inc cursor to right when writing, dont shift screen
  delay_ms(DEFAULT_DELAY);
  lcd1602_write_com(0x0C);  // disp on, cursor & blonk off
  delay_ms(DEFAULT_DELAY);
}


