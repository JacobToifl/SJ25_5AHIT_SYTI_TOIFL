#define F_CPU 16000000UL

#include <Arduino.h>
#include <avr/interrupt.h>
#include "dht.h"
#include "lcd.h"
#include "uart.h"

#define UART_BAUD_RATE 9600
#define STX 0x02
#define ETX 0x03

#define OCR_1SEC 15624
#define OCR_4SEC 62499

volatile bool tick = false;
float currentTemp;
float currentHumidity;
int8_t errorStatus;

void init_uart(void) {
    uart_init(UART_BAUD_SELECT(UART_BAUD_RATE, F_CPU));
}

void send_values_uart(int8_t temp, int8_t hum) {
    char buffer[32];
    // Format: <STX>Temp:22;Hum:55<ETX>
    sprintf(buffer, "Temp:%d;Hum:%d", temp, hum);
    uart_putc(STX);
    uart_puts(buffer);
    uart_putc(ETX);
}

void send_error(void) {
    uart_puts("DHT Error\r\n");
}

void init_timer(void) {
    TCCR1A = 0;
    TCCR1B = 0;
    OCR1A  = OCR_1SEC;
    TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10);
    TIMSK1 |= (1 << OCIE1A);
}

void init_lcd(void) {
    lcd_init(LCD_DISP_ON);
    lcd_clrscr();
}

void read_dht11(void) {
    errorStatus = dht_gettemperaturehumidity(&currentTemp, &currentHumidity);
    if (errorStatus != 0) {
        lcd_clrscr();
        lcd_puts("DHT11 Fehler!");
    }
}

void print_values(const int8_t temp, const int8_t hum) {
    char tempBuffer[16];
    char humBuffer[16];
    sprintf(tempBuffer, "T: %d C", temp);
    sprintf(humBuffer, "H: %d %%", hum);

    lcd_clrscr();
    lcd_puts(tempBuffer);
    lcd_gotoxy(0, 1);
    lcd_puts(humBuffer);
}

void check_uart_input(void) {
    unsigned int c = uart_getc();

    if (!(c & UART_NO_DATA)) {
        unsigned char data = (unsigned char)c;
        if (data == '1') {
            OCR1A = OCR_1SEC;
            TCNT1 = 0;
            uart_puts("\r\nIntervall: 1s\r\n");
        }
        else if (data == '4') {
            OCR1A = OCR_4SEC;
            TCNT1 = 0;
            uart_puts("\r\nIntervall: 4s\r\n");
        }
    }
}

int main(void) {
    cli();
    init_lcd();
    init_timer();
    init_uart();
    sei();

    while (1) {
        check_uart_input();
        if (tick) {
            tick = false;
            read_dht11();
            if (errorStatus == 0) {
                print_values((int8_t)currentTemp, (int8_t)currentHumidity);
                send_values_uart((int8_t)currentTemp, (int8_t)currentHumidity);
            } else {
                send_error();
            }
        }
    }
}

ISR(TIMER1_COMPA_vect) {
    tick = true;
}