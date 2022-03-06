
/*
 *
 * by Lewis Loflin www.bristolwatch.com lewis@bvu.net
 * http://www.bristolwatch.com/rpi/i2clcd.htm
 * Using wiringPi by Gordon Henderson
 *
 *
 * Port over lcd_i2c.py to C and added improvements.
 * Supports 16x2 and 20x4 screens.
 * This was to learn now the I2C lcd displays operate.
 * There is no warrenty of any kind use at your own risk.
 *
 */

#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "lcd.h"
 // Define some device parameters
#define I2C_ADDR 0x27 // I2C device address

// Define some device constants
#define LCD_CHR 1 // Mode - Sending data
#define LCD_CMD 0 // Mode - Sending command

#define LCD_BACKLIGHT 0x08 // On
// LCD_BACKLIGHT = 0x00  # Off

#define ENABLE 0b00000100 // Enable bit

int fd; // seen by all subroutines

void lcd_setup() {

    if (wiringPiSetup() == -1)
        exit(1);

    fd = wiringPiI2CSetup(I2C_ADDR);
    lcd_init();
}

void typeFloat(float myFloat) {
    char buffer[20];
    sprintf(buffer, "%4.2f", myFloat);
    typeln(buffer);
}

void typeInt(int i) {
    char array1[20];
    sprintf(array1, "%d", i);
    typeln(array1);
}

void ClrLcd(void) {
    lcd_byte(0x01, LCD_CMD);
    lcd_byte(0x02, LCD_CMD);
}

void lcdLoc(int line) {
    lcd_byte(line, LCD_CMD);
}

void typeChar(char val) {

    lcd_byte(val, LCD_CHR);
}

void typeln(const char* s) {

    while (*s)
        lcd_byte(*(s++), LCD_CHR);
}

void lcd_byte(int bits, int mode) {


    int bits_high;
    int bits_low;
    bits_high = mode | (bits & 0xF0) | LCD_BACKLIGHT;
    bits_low = mode | ((bits << 4) & 0xF0) | LCD_BACKLIGHT;

    wiringPiI2CReadReg8(fd, bits_high);
    lcd_toggle_enable(bits_high);

    wiringPiI2CReadReg8(fd, bits_low);
    lcd_toggle_enable(bits_low);
}

void lcd_toggle_enable(int bits) {
    delayMicroseconds(500);
    wiringPiI2CReadReg8(fd, (bits | ENABLE));
    delayMicroseconds(500);
    wiringPiI2CReadReg8(fd, (bits & ~ENABLE));
    delayMicroseconds(500);
}

void lcd_init() {
    lcd_byte(0x33, LCD_CMD);
    lcd_byte(0x32, LCD_CMD);
    lcd_byte(0x06, LCD_CMD);
    lcd_byte(0x0C, LCD_CMD);
    lcd_byte(0x28, LCD_CMD);
    delayMicroseconds(500);
}

void lcd_stop() {
    close(fd);
}