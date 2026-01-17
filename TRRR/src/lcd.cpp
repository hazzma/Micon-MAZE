#include "lcd.h"

void LCD_enable_pulse(void) {
    LCD_CTRL_PORT |= (1 << LCD_EN);
    _delay_us(1);
    LCD_CTRL_PORT &= ~(1 << LCD_EN);
    _delay_us(100);
}

void LCD_send_nibble(unsigned char data) {
    // Data D4-D7 ada di PD4-PD7
    LCD_DATA_PORT = (LCD_DATA_PORT & 0x0F) | (data & 0xF0);
    LCD_enable_pulse();
}

void LCD_putcmd(unsigned char command, unsigned char cmdtype) {
    LCD_CTRL_PORT &= ~(1 << LCD_RS); // Mode Command
    LCD_send_nibble(command);
    if (cmdtype) LCD_send_nibble(command << 4);
    _delay_ms(2);
}

void LCD_putch(unsigned char data) {
    LCD_CTRL_PORT |= (1 << LCD_RS); // Mode Data
    LCD_send_nibble(data);
    LCD_send_nibble(data << 4);
    _delay_us(50);
}

void initlcd(void) {
    LCD_CTRL_DDR |= (1 << LCD_RS) | (1 << LCD_EN);
    LCD_DATA_DDR |= 0xF0; // PD4-PD7 Output

    _delay_ms(50);
    LCD_CTRL_PORT &= ~(1 << LCD_RS);
    LCD_send_nibble(0x30); _delay_ms(5);
    LCD_send_nibble(0x30); _delay_us(150);
    LCD_send_nibble(0x30);
    LCD_send_nibble(0x20); // 4-bit mode

    LCD_putcmd(0x28, LCD_2CYCLE);
    LCD_putcmd(0x0C, LCD_2CYCLE); // Display ON, Cursor OFF
    LCD_putcmd(0x01, LCD_2CYCLE); // Clear
    LCD_putcmd(0x06, LCD_2CYCLE); // Increment
}

void lcd_puts(char *s) {
    while(*s) {
        LCD_putch(*s);
        s++;
    }
}

void lcd_gotoxy(unsigned char x, unsigned char y) {
    unsigned char addr = 0x80 + x;
    if (y == 1) addr = 0xC0 + x;
    LCD_putcmd(addr, LCD_2CYCLE);
}

void lcd_clear(void) {
    LCD_putcmd(0x01, LCD_2CYCLE);
}