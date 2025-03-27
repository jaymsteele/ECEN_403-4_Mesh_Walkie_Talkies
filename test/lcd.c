//LCD.c:
// lcd.c - Source file for LCD functions
#include "lcd.h"
#include <unistd.h>   // For usleep
#include <stdint.h>   // For standard types
#include <stddef.h>   // For NULL

/*
 *  ======== LCD_init ========
 *  Initialize the LCD with necessary commands and settings.
 */
void LCD_init(I2C_Handle i2c) {
    // Ensure the LCD is in 4-bit mode
    LCD_send(i2c, 0x03, LCD_COMMAND);
    usleep(5000);  // Wait 5ms

    LCD_send(i2c, 0x03, LCD_COMMAND);
    usleep(5000);  // Wait 5ms

    LCD_send(i2c, 0x03, LCD_COMMAND);
    usleep(5000);  // Wait 5ms

    LCD_send(i2c, 0x02, LCD_COMMAND);  // Finally, set to 4-bit mode
    usleep(5000);  // Wait 5ms

    // Set function, control, and entry mode for the LCD
    LCD_send(i2c, LCD_FUNCTION_SET | LCD_2LINE | LCD_5x8DOTS | LCD_4BITMODE, LCD_COMMAND);
    usleep(2000);  // Wait 2ms

    LCD_send(i2c, LCD_DISPLAY_CONTROL | LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF, LCD_COMMAND);
    usleep(2000);  // Wait 2ms

    LCD_send(i2c, LCD_CLEAR_DISPLAY, LCD_COMMAND);  // Clear the display
    usleep(2000);  // Wait 2ms

    LCD_send(i2c, LCD_ENTRY_MODE_SET | LCD_ENTRY_LEFT | LCD_ENTRY_SHIFT_DECREMENT, LCD_COMMAND);
    usleep(2000);  // Wait 2ms
}

/*
 *  ======== LCD_clear ========
 *  Clears the LCD display.
 */
void LCD_clear(I2C_Handle i2c) {
    LCD_send(i2c, LCD_CLEAR_DISPLAY, LCD_COMMAND);
    usleep(2000);  // Wait 2ms for the clear command to process
}

/*
 *  ======== LCD_setCursor ========
 *  Set the cursor to a specific position on the LCD.
 */
void LCD_setCursor(I2C_Handle i2c, uint8_t col, uint8_t row) {
    uint8_t rowOffsets[] = {0x00, 0x40, 0x14, 0x54};  // DDRAM offsets for 16x2 or 20x4 LCDs

    if (row > 1) {  // Limit the row value for a 16x2 LCD
        row = 1;
    }

    // Set the DDRAM address based on row and column
    LCD_send(i2c, LCD_SET_CURSOR | (col + rowOffsets[row]), LCD_COMMAND);
}

/*
 *  ======== LCD_print ========
 *  Print a string to the LCD.
 */
void LCD_print(I2C_Handle i2c, char *str) {
    while (*str) {
        LCD_send(i2c, *str++, LCD_DATA);  // Send each character to the LCD
    }
}

/*
 *  ======== I2C_writeCommand ========
 *  Send a command to the LCD via I2C.
 */
void I2C_writeCommand(I2C_Handle i2c, uint8_t command) {
    uint8_t buffer[1];
    buffer[0] = command | LCD_BACKLIGHT;  // Combine command with backlight state

    I2C_Transaction i2cTransaction = {0};  // Initialize the I2C transaction structure
    i2cTransaction.targetAddress = LCD_I2C_ADDRESS;  // Set the target I2C address
    i2cTransaction.writeBuf = buffer;  // Set pointer to the buffer
    i2cTransaction.writeCount = 1;  // Set number of bytes to write
    i2cTransaction.readBuf = NULL;
    i2cTransaction.readCount = 0;

    I2C_transfer(i2c, &i2cTransaction);  // Perform the I2C transfer
}

/*
 *  ======== LCD_send ========
 *  Send a byte of data or command to the LCD.
 */
void LCD_send(I2C_Handle i2c, uint8_t value, uint8_t mode) {
    uint8_t highNibble = value & 0xF0;  // Extract the high nibble
    uint8_t lowNibble = (value << 4) & 0xF0;  // Shift the low nibble

    // Write high nibble
    I2C_writeCommand(i2c, highNibble | mode);
    LCD_strobe(i2c, highNibble | mode);

    // Write low nibble
    I2C_writeCommand(i2c, lowNibble | mode);
    LCD_strobe(i2c, lowNibble | mode);
}

/*
 *  ======== LCD_strobe ========
 *  Pulse the enable bit to latch the data.
 */
void LCD_strobe(I2C_Handle i2c, uint8_t data) {
    I2C_writeCommand(i2c, data | LCD_ENABLE_BIT);  // Enable pulse
    usleep(1000);  // Wait for enable to settle
    I2C_writeCommand(i2c, data & ~LCD_ENABLE_BIT);  // Disable pulse
    usleep(1000);  // Wait for command to settle
}
