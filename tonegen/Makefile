# Project Name
PROJECT = tone_generator

# Source Files
SRC = main.c

# MCU, Clock Speed
MCU = attiny85
F_CPU = 8000000UL

# Output
OBJ = $(SRC:.c=.o)
HEX = $(PROJECT).hex

# Compiler and Flags
CC = avr-gcc
CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os
LDFLAGS = 

# Programmer
PROGRAMMER = usbasp
PORT = usb

# Compile
all: $(HEX)

$(HEX): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(PROJECT).elf $(OBJ)
	avr-objcopy -j .text -j .data -O ihex $(PROJECT).elf $(HEX)
	avr-size --format=avr --mcu=$(MCU) $(PROJECT).elf

# Upload to MCU
upload:
	avrdude -c $(PROGRAMMER) -p $(MCU) -P $(PORT) -U flash:w:$(HEX)

# Clean
clean:
	rm -f $(PROJECT).elf $(HEX) $(OBJ)

# Rebuild
rebuild: clean all

# Replace 'main.c' with the path to your source file, and adjust the MCU and F_CPU
# values according to your ATtiny85's configuration and clock speed.
# You may need to install avr-gcc, avr-libc, and avrdude if not already installed.
