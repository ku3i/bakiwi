/*

   +------------------+
   | PLOPS HELLOWORLD |
   +------------------+

   Toggels LED pin every 100ms

   0) check   : avrdude -c arduino -p t85 -P /dev/ttyUSB0 -b 19200 -v
   1) compile : avr-gcc -mmcu=attiny85 -Os -c main.c -o main.o
   2) link    : avr-gcc main.o -o main.elf
   3) to hex  : avr-objcopy -O ihex -j .text -j .data main.elf main.hex
   4) check   : avr-size --mcu=attiny85 -C main.elf
   5) burn    : avrdude -c arduino -p t85 -P /dev/ttyUSB0 -b 19200 -v -U flash:w:main.hex:a

   or use make, flash, fuse, install

*/

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>

int main (void) {
	DDRB |= (1 << PB2); // set led pin to output
	while(1) {
		_delay_ms(500);
		PORTB ^= (1 << PB2); // toggle led
	}

   return 0;
}
