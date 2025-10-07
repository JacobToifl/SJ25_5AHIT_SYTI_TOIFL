#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include "lcd.h"

uint16_t EEMEM savedNumber;
uint16_t actualNumber = 0;

volatile uint8_t new_pressed  = 0;
volatile uint8_t save_pressed = 0;

void buttons_init(void) {
    DDRD  &= ~((1 << PD2) | (1 << PD3));
    PORTD |=  (1 << PD2) | (1 << PD3);
}

void timer_init(void) {
    TCCR1A = 0;
    TCCR1B = (1 << CS11) | (1 << CS10);
}

void interrupts_init(void) {
    EICRA = (1 << ISC01) | (1 << ISC11);
    EIMSK = (1 << INT0) | (1 << INT1);
}

void print(const char *text, uint16_t v) {
    char buf[6];
    lcd_clrscr();
    lcd_puts(text);
    lcd_gotoxy(0,1);
    itoa(v, buf, 10);
    lcd_puts(buf);
}

int main(void) {
    lcd_init(LCD_DISP_ON);
    buttons_init();
    timer_init();
    interrupts_init();
    sei();

    actualNumber = eeprom_read_word(&savedNumber);
    print("last saved:", actualNumber);

    while (1) {
        if (new_pressed) {
            new_pressed = 0;
            _delay_ms(20);
            if (!(PIND & (1 << PD2))) {
                srand(TCNT1);
                actualNumber = rand() % 1000;
                print("number:", actualNumber);
                while (!(PIND & (1 << PD2))) {;}
                _delay_ms(10);
            }
        }

        if (save_pressed) {
            save_pressed = 0;
            _delay_ms(20);
            if (!(PIND & (1 << PD3))) {
                eeprom_update_word(&savedNumber, actualNumber);
                print("saved:", actualNumber);
                _delay_ms(2000);
                print("number:", actualNumber);
                while (!(PIND & (1 << PD3))) {;}
                _delay_ms(10);
            }
        }
    }
}

ISR(INT0_vect) {  new_pressed = 1; }
ISR(INT1_vect) {  save_pressed = 1; }