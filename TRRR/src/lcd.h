#ifndef LCD_H
#define LCD_H

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

// Definisi PIN
#define LCD_DATA_PORT PORTD
#define LCD_DATA_DDR  DDRD
#define LCD_CTRL_PORT PORTB
#define LCD_CTRL_DDR  DDRB
#define LCD_RS        PB0
#define LCD_EN        PB4

#define LCD_1CYCLE  0
#define LCD_2CYCLE  1

// Prototypes
void initlcd(void);
void lcd_puts(char *s);
void lcd_gotoxy(unsigned char x, unsigned char y);
void lcd_clear(void);

#endif