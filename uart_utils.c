/* Handles user input and printing to the serial console
*/

#include "uart_utils.h"
#include "bitwise.h"
#include <string.h>
#include <stdarg.h> // Required for va_list, va_start
#include <stdio.h>  // Required for vsnprintf
#include <stdlib.h> // Required for strtof
#include <stdbool.h> // For bool, true, false
#include <ctype.h>  // For isspace

// Alustaa USART2:n
void uart_init(void)
{
    // set PA2 & PA3 alternate functions to AF7 (USART2 RX/TX)
    bits_val(GPIOA->AFR[0], 4, 2, 7); // PA2 -> USART2_TX
    bits_val(GPIOA->AFR[0], 4, 3, 7); // PA3 -> USART2_RX
    bits_val(GPIOA->MODER , 2, 2, 2); // PA2 -> alternate function mode
    bits_val(GPIOA->MODER , 2, 3, 2); // PA3 -> alternate function mode

    // configure UART as 8N1 at 115200bps
    USART2->BRR  = baud(115200);
    USART2->CR1 |= USART_CR1_UE | USART_CR1_RE | USART_CR1_TE;
}

// send single character to UART
void uart_send(char c)
{
    // wait for the transmit buffer to become empty
    while (!(USART2->SR & USART_SR_TXE))
        ;

    // begin transmission of character
    USART2->DR = c;
}

// send string to UART
void uart_send_str(char *s)
{
    char c;
    // transmit whole string
    while ((c = *s++) != '\0')
        uart_send(c);
}

// receive single character from UART (blocking)
char uart_recv()
{
    // wait for a single byte to be received
    while (!(USART2->SR & USART_SR_RXNE))
        ;

    // return received byte
    return USART2->DR;
}



// Prints text WITHOUT a newline.
void print(const char *format, ...)
{
    char buffer[128];
    va_list args;
    
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    uart_send_str(buffer);
}

// Prints text WITH a newline.
void println(const char *format, ...)
{
    char buffer[128];
    va_list args;
    
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    uart_send_str(buffer);
    uart_send_str("\r\n"); // Explicitly add CR+LF
}

float read_float_from_uart(float min, float max) {
    char buffer[32];
    uint8_t index = 0;
    char c;
    uint8_t decimal_entered = 0;
    
    // clear buffer
    memset(buffer, 0, sizeof(buffer));
    
    while (1) {
        c = uart_recv();
        
        // Handle Enter
        if (c == '\r' || c == '\n') {
            if (index > 0) {
                buffer[index] = '\0';
                float value = strtof(buffer, NULL); 
                
                // IMPORTANT: Move cursor to next line after user hits enter
                println(""); 
                
                if (value < min) {
                    println("Error: Value too small. Minimum is %.2f. Try again: ", min);
                    index = 0;
                    decimal_entered = 0;
                    continue;
                }
                if (value > max) {
                    println("Error: Value too large. Maximum is %.2f. Try again: ", max);
                    index = 0;
                    decimal_entered = 0;
                    continue;
                }
                
                return value;
            }
            continue;
        }
        
        // Handle Backspace
        if (c == '\b' || c == 127) {
            if (index > 0) {
                index--;
                if (buffer[index] == '.') {
                    decimal_entered = 0;
                }
                buffer[index] = 0; 
                uart_send('\b');
                uart_send(' ');
                uart_send('\b');
            }
            continue;
        }
        
        // Handle Numbers and Decimal point
        if ((c >= '0' && c <= '9') || (c == '.' && !decimal_entered) || (c == '-' && index == 0)) {
             if (index < sizeof(buffer) - 1) {
                 if (c == '.') decimal_entered = 1;
                 buffer[index++] = c;
                 uart_send(c); // Echo character back to user
             }
        }
    }
}