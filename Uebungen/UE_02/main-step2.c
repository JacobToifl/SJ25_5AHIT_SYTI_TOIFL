#include <Arduino.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "dht.h"
#include "lcd.h"

#define F_CPU 16000000UL
#define DHT_TIMEOUT 2000
#define STX 0x02
#define ETX 0x03

volatile bool tick = false;
int8_t currentTemp;
int8_t currentHumidity;
int8_t errorStatus;

void init_uart(void) {
    UBRR0 = 103;
    UCSR0B |= (1<<TXEN0);
    UCSR0C |= (1<<UCSZ01) | (1<<UCSZ00);
}

void send(int8_t temp, int8_t hum) {

    char buffer[5];
    snprintf(buffer, sizeof(buffer), "%d%d", temp, hum);

    while (!(UCSR0A & (1<<UDRE0))) {}
    UDR0 = 0x02;
    while (!(UCSR0A & (1<<UDRE0))) {}
    UDR0 = (uint8_t)temp;
    while (!(UCSR0A & (1<<UDRE0))) {}
    UDR0 = (uint8_t)hum;
    while (!(UCSR0A & (1<<UDRE0))) {}
    UDR0 = 0x03;
}

void send_error(void) {
    while (!(UCSR0A & (1 << UDRE0))) {}
    UDR0 = 0x02;
    while (!(UCSR0A & (1 << UDRE0))) {}
    UDR0 = '1';
    while (!(UCSR0A & (1 << UDRE0))) {}
    UDR0 = 0x03;
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
                send(currentTemp, currentHumidity);
            }
            else{
                send_error();
            }
        }
    }
}

ISR(TIMER1_COMPA_vect) {
    tick = true;
}

//#define LCD_PORT         PORTB        /**< port for the LCD lines   */
//#define LCD_DATA0_PORT   LCD_PORT     /**< port for 4bit data bit 0 */
//#define LCD_DATA1_PORT   LCD_PORT     /**< port for 4bit data bit 1 */
//#define LCD_DATA2_PORT   LCD_PORT     /**< port for 4bit data bit 2 */
//#define LCD_DATA3_PORT   LCD_PORT     /**< port for 4bit data bit 3 */
//#define LCD_DATA0_PIN    PB0        /**< pin for 4bit data bit 0  */
//#define LCD_DATA1_PIN    PB1          /**< pin for 4bit data bit 1  */
//#define LCD_DATA2_PIN    PB2          /**< pin for 4bit data bit 2  */
//#define LCD_DATA3_PIN    PB3          /**< pin for 4bit data bit 3  */
//#define LCD_RS_PORT      PORTC   /**< port for RS line         */
//#define LCD_RS_PIN       PC0          /**< pin  for RS line         */
//#define LCD_RW_PORT      PORTC     /**< port for RW line         */
//#define LCD_RW_PIN       PC1          /**< pin  for RW line         */
//#define LCD_E_PORT       PORTC     /**< port for Enable line     */
//#define LCD_E_PIN        PC2          /**< pin  for Enable line     */

