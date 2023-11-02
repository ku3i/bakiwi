DEVICE     = attiny84
CLOCK      = 8000000
PROGRAMMER = -c arduino -P /dev/ttyUSB0 -b 19200
OBJECTS    = main.o
FUSES      = -U lfuse:w:0xE2:m -U hfuse:w:0xDF:m -U efuse:w:0xFF:m

#        SUT0-----+ +-----CKSEL3
#        SUT1----+| |+----CKSEL2
#       CKOUT---+|| ||+---CKSEL1
#      CKDIV8--+||| |||+--CKSEL0
#              |||| ||||
# lfuse: 0xE2: 1110.0010
#        CKSEL[3..0] = 0010 : calibrated internal oscillator
#        SUT[1..0]   = 10   : slowly rising power
#        CKOUT       = 1    : no clock output
#        CKDIV8      = 1    : no clock division by 8
# hfuse: 0xDF (defaults)
# efuse: 0xFD: xxxx.x110 Brownout detection min:1V7 typ:1V8 max:2V0
# fuses explained: http://www.ladyada.net/learn/avr/fuses.html
# calculator: http://www.engbedded.com/fusecalc/

# attiny85 defaults: lfuse: 0x62, hfuse: 0xDF, efuse 0x07 (0xFF)

######################################################################
######################################################################

# Tune the lines below only if you know what you are doing:

AVRDUDE = avrdude $(PROGRAMMER) -p $(DEVICE)
COMPILE = avr-g++ -Wall -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE)

# symbolic targets:
all : main.hex

.c.o:
	$(COMPILE) -c $< -o $@

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

.c.s:
	$(COMPILE) -S $< -o $@

flash:  all
	$(AVRDUDE) -U flash:w:main.hex:i

test:
	$(AVRDUDE) -v

read:
	$(AVRDUDE) -U eeprom:r:eedump.hex:i

fuse:
	$(AVRDUDE) $(FUSES)

install: flash fuse

# if you use a bootloader, change the command below appropriately:
#load: all
#    bootloadHID main.hex

clean:
	rm -f main.hex main.elf $(OBJECTS)

# file targets:
main.elf: $(OBJECTS)
	$(COMPILE) -o main.elf $(OBJECTS)

main.hex: main.elf
	rm -f main.hex
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex
# If you have an EEPROM section, you must also create a hex file for the
# EEPROM and add it to the "flash" target.

# Targets for code debugging and analysis:
disasm: main.elf
	avr-objdump -d main.elf

cpp:
	$(COMPILE) -E main.c

