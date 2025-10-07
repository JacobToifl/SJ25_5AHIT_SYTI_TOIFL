#include <Arduino.h>
#include "lcd.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16000000UL

volatile uint8_t tick1s = 0;
volatile uint8_t button_pressed = 0;

void adc_init(void) {
    ADMUX  = (1 << REFS0); 
    ADCSRA = (1 << ADEN)  | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t adc_read(uint8_t channel) {
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
    ADCSRA |= (1 << ADSC);                    
    while (ADCSRA & (1 << ADSC));             
    return ADC;
}

void timer1_init(void) {
    TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10); 
    OCR1A  = 15624;                                   
    TIMSK1 = (1 << OCIE1A);
}

void button_init(void) {
    EICRA = (1 << ISC01);   
    EIMSK = (1 << INT0);    
    PORTD |= (1 << PD2);    
}

void led_init(void) { DDRB |= (1 << PB0); }
void led_on(void)   { PORTB |= (1 << PB0); }
void led_off(void)  { PORTB &= ~(1 << PB0); }

void lcd_show_temp(float t) {
    char buf[17];
    snprintf(buf, sizeof(buf), "S1: %2.1f \xDF""C");
    lcd_gotoxy(0,0);
    lcd_puts("                ");
    lcd_gotoxy(0,0);
    lcd_puts(buf);
}
void lcd_show_threshold(float thr) {
    char buf[17];
    snprintf(buf, sizeof(buf), "SW: %2.2f");
    lcd_gotoxy(0,0);
    lcd_puts("                ");
    lcd_gotoxy(0,0);
    lcd_puts(buf);
}

float ntc_celsius_from_adc(uint16_t adc) {
    if (adc == 0) adc = 1; 
    float r = 10000.0 * ((1024.0 / adc) - 1.0);
    float lnR = log(r);
    float invT = 0.001129148 + (0.000234125 + (0.0000000876741 * lnR * lnR)) * lnR;
    float tempK = 1.0 / invT;
    return tempK - 273.15;
}



int main(void) {
    lcd_init(LCD_DISP_ON);
    lcd_clrscr();
    adc_init();
    timer1_init();
    button_init();
    led_init();
    sei();

    float threshold = 22.0;
    uint8_t showSW = 0;

    while (1) {
        if (button_pressed) {
            button_pressed = 0;
            threshold += 0.5;
            if (threshold > 24.0) threshold = 24.0;
            if (threshold < 20.0) threshold = 20.0;

            lcd_show_threshold(threshold);
            showSW = 1; 
        }

        if (tick1s) {
            tick1s = 0;

            uint16_t raw = adc_read(1); 
            float temp = ntc_celsius_from_adc(raw);

            if (temp > threshold) led_on(); else led_off();

            if (showSW) {
                showSW = 0; 
            } else {
                lcd_show_temp(temp);
            }
        }
    }

    ISR(TIMER1_COMPA_vect) { tick1s = 1; }
    ISR(INT0_vect) { button_pressed = 1; }
}