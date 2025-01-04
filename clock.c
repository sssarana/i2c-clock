#define F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay.h>

#define SDA_PIN 0
#define SCL_PIN 4
#define BUTTON_PIN 2

uint8_t segm[12] = {0b00111111, 0b0000110, 0b01011011, 0b01001111, 0b01100110, 0b01101101, 0b01111101, 0b00000111, 0b01111111, 0b01101111, 0, 0b00000010};
uint8_t razr[5] = {0, 2, 6, 8, 4};
uint8_t e = 2, d = 2, s = 2, t = 2;
uint8_t seconds = 0;

void SDA_LOW() {
    DDRB |= (1 << SDA_PIN);
    PORTB &= ~(1 << SDA_PIN);
}

void SDA_HIGH() {
    DDRB &= ~(1 << SDA_PIN);
    PORTB |= (1 << SDA_PIN);
}

void SCL_LOW() {
    DDRB |= (1 << SCL_PIN);
    PORTB &= ~(1 << SCL_PIN);
}

void SCL_HIGH() {
    DDRB &= ~(1 << SCL_PIN);
    PORTB |= (1 << SCL_PIN);
}

void start() {
    SDA_HIGH();
    SCL_HIGH();
    _delay_us(4);
    SDA_LOW();
    _delay_us(4);
    SCL_LOW();
}

void stop() {
    SDA_LOW();
    _delay_us(4);
    SCL_HIGH();
    _delay_us(4);
    SDA_HIGH();
}

void wr_i2c(uint8_t val) {
    for (uint8_t i = 0; i < 8; i++) {
        if (val & (1 << (7 - i))) {
            SDA_HIGH();
        } else {
            SDA_LOW();
        }
        _delay_us(1);
        SCL_HIGH();
        _delay_us(4);
        SCL_LOW();
        _delay_us(1);
    }
    SDA_HIGH();
    _delay_us(1);
    SCL_HIGH();
    _delay_us(4);
    SCL_LOW();
}

void _print(uint8_t addr, uint8_t val) {
    start();
    wr_i2c(0b11100000);
    wr_i2c(razr[addr]);
    wr_i2c(segm[val]);
    stop();
}

void display_time() {
    _print(0, t);
    _print(1, s);
    _print(2, d);
    _print(3, e);
    _print(4, 10);
}

void increment_time() {
    e++;
    if (e > 9) {
        d++;
        e = 0;
    }
    if (d > 5) {
        s++;
        d = 0;
    }
    if (s > 9) {
        t++;
        s = 0;
    }
    if (t > 2 || (t == 2 && s > 3)) {
        t = 0;
        s = 0;
    }
}

int main(void) {
    SDA_HIGH();
    SCL_HIGH();

    DDRB &= ~(1 << BUTTON_PIN);
    PORTB |= (1 << BUTTON_PIN);

    start();
    wr_i2c(0b11100000);
    wr_i2c(0b00100001);
    stop();

    start();
    wr_i2c(0b11100000);
    wr_i2c(0b10000001);
    stop();

    uint8_t button_pressed = 0;
    uint16_t button_hold_time = 0;

    while (1) {
        display_time();
        _delay_ms(1000);
        seconds++;

        if (seconds >= 60) {
            seconds = 0;
            increment_time();
        }

        if (!(PINB & (1 << BUTTON_PIN))) {
            if (button_hold_time == 0) {
                _delay_ms(25);
                if (!(PINB & (1 << BUTTON_PIN))) {
                    button_pressed = 1;
                }
            }

            if (button_pressed) {
                button_hold_time += 10;

                if (button_hold_time >= 1000) {
                    increment_time();
                    _delay_ms(50);
                } else if (button_hold_time % 100 == 0) {
                    increment_time();
                }
            }
        } else {
            button_pressed = 0;
            button_hold_time = 0;
        }
    }

    return 0;
}
