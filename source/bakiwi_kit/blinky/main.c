/*

   +-------------------+
   | BAKIWI HELLOWORLD |
   +-------------------+

   Toggels LED pin every 100ms

   0) check   : avrdude -c arduino -p t84 -P /dev/ttyUSB0 -b 19200 -v
   1) compile : avr-gcc -mmcu=attiny84 -Os -c main.c -o main.o
   2) link    : avr-gcc main.o -o main.elf
   3) to hex  : avr-objcopy -O ihex -j .text -j .data main.elf main.hex
   4) check   : avr-size --mcu=attiny84 -C main.elf
   5) burn    : avrdude -c arduino -p t84 -P /dev/ttyUSB0 -b 19200 -v -U flash:w:main.hex:a

   or use make, flash, fuse, install

*/

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>

int main (void) {
    DDRA |= (1 << PA7); // set led left pin to output	
    DDRB |= (1 << PB2); // set led right pin to output

    PORTA |= (1 << PA7); // set led left high (on)

    while(1) {
        _delay_ms(500);
        PORTA ^= (1 << PA7); // toggle led
        PORTB ^= (1 << PB2); // toggle led
    }

   return 0;
}
