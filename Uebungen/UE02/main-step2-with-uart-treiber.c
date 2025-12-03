#define F_CPU 16000000UL

#include <Arduino.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "dht.h"
#include "lcd.h"
#include "uart.h"

#define UART_BAUD_RATE 9600
#define DHT_TIMEOUT 2000
#define STX 0x02
#define ETX 0x03

volatile bool tick = false;
int8_t currentTemp;
int8_t currentHumidity;
int8_t errorStatus;

void init_uart(void) {
    //UBRR0 = 103;
    //UCSR0B |= (1<<TXEN0);
    //UCSR0C |= (1<<UCSZ01) | (1<<UCSZ00);

    uart_init(UART_BAUD_SELECT(UART_BAUD_RATE, F_CPU));
}

void send_values_uart(int8_t temp, int8_t hum)
{
    char buffer[16];
    sprintf(buffer, "%d;%d", temp, hum);
    uart_putc(STX);
    uart_puts(buffer);
    uart_putc(ETX);
}

void send_error(void) {
    uart_puts("Error");
}

void init_timer(void) {
    TCCR1A = 0;
    TCCR1B = 0;
    OCR1A  = 15624;
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
        lcd_gotoxy(0, 1);
        lcd_puts("Kein Wert");
    }
}

void print_values(const int8_t temp, const int8_t hum) {
    char tempBuffer[9];
    char humBuffer[9];
    sprintf(tempBuffer, "%d", temp);
    sprintf(humBuffer, "%d", hum);

    lcd_clrscr();
    lcd_puts("Temp: ");
    lcd_puts(tempBuffer);
    lcd_puts("C");
    lcd_gotoxy(0, 1);
    lcd_puts("Humid: ");
    lcd_puts(humBuffer);
    lcd_puts("%");
}

int main(void) {
    cli();
    init_lcd();
    init_timer();
    init_uart();
    sei();

    while (1) {
        if (tick) {
            tick = false;
            read_dht11();
            if (errorStatus == 0) {
                print_values(currentTemp, currentHumidity);
                send_values_uart(currentTemp, currentHumidity);
            } else {
                send_error();
            }
        }
    }
}

ISR(TIMER1_COMPA_vect)
{
    tick = true;
}



