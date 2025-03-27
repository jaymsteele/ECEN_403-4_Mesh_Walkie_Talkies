#ifndef LCD_H
#define LCD_H

#include <ti/drivers/I2C.h>  // Include I2C driver

// Define the I2C address for the LCD (commonly 0x27 or 0x3F depending on the module)
#define LCD_I2C_ADDRESS 0x27

// LCD Commands
#define LCD_CLEAR_DISPLAY 0x01
#define LCD_RETURN_HOME 0x02
#define LCD_ENTRY_MODE_SET 0x04
#define LCD_DISPLAY_CONTROL 0x08
#define LCD_FUNCTION_SET 0x20
#define LCD_SET_DDRAM_ADDR 0x80

// LCD Options
#define LCD_ENTRY_LEFT 0x02
#define LCD_ENTRY_SHIFT_DECREMENT 0x00
#define LCD_DISPLAY_ON 0x04
#define LCD_CURSOR_OFF 0x00
#define LCD_BLINK_OFF 0x00
#define LCD_2LINE 0x08
#define LCD_5x8DOTS 0x00
#define LCD_4BITMODE 0x00

// LCD Control Macros
#define LCD_BACKLIGHT 0x08
#define LCD_ENABLE_BIT 0x04
#define LCD_COMMAND 0x00
#define LCD_DATA 0x01

// Cursor positioning base command
#define LCD_SET_CURSOR 0x80

// Function Prototypes
void LCD_init(I2C_Handle i2c);
void LCD_clear(I2C_Handle i2c);
void LCD_print(I2C_Handle i2c, char *str);
void LCD_setCursor(I2C_Handle i2c, uint8_t col, uint8_t row);
void I2C_writeCommand(I2C_Handle i2c, uint8_t command);
void LCD_send(I2C_Handle i2c, uint8_t value, uint8_t mode);
void LCD_strobe(I2C_Handle i2c, uint8_t data);

#endif / LCD_H */
