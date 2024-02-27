# Project settings
PROJECT = vu_meter
MCU = attiny85
F_CPU = 8000000
SRC = main.c
OBJ = $(SRC:.c=.o)
HEX = $(PROJECT).hex

# AVR toolchain programs
CC = avr-gcc
OBJCOPY = avr-objcopy
AVRDUDE = avrdude

# Compiler and linker flags
CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os
LDFLAGS =

# Programmer settings
PROGRAMMER = stk500v1
PORT = usb

# Compile the project
all: $(HEX)

$(HEX): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(PROJECT).elf $(OBJ)
	$(OBJCOPY) -O ihex -R .eeprom $(PROJECT).elf $(HEX)
	avr-size --mcu=$(MCU) --format=avr $(PROJECT).elf

# Upload the hex file to the MCU
upload:
	$(AVRDUDE) -c $(PROGRAMMER) -p $(MCU) -P $(PORT) -U flash:w:$(HEX):i

# Clean up build files
clean:
	rm -f $(PROJECT).elf $(HEX) $(OBJ)

# Rebuild the project (clean and then all)
rebuild: clean all

# Replace 'main.c' with the path to your source file if it's different.
# Adjust 'MCU', 'F_CPU', 'PROGRAMMER', and 'PORT' as needed for your setup.
