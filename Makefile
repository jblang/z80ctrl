BIN=z80ctrl
FF_OBJS=diskio.o ff.o ffsystem.o mmc_avr_spi.o #ffunicode.o
OBJS=z80ctrl.o uart.o spi.o iox.o bus.o memory.o ihex.o z80.o cli.o diskemu.o disasm.o $(FF_OBJS)

CC=avr-gcc
OBJCOPY=avr-objcopy
AVRDUDE=avrdude
MCU=atmega1284p
F_CPU=20000000L
PROGRAMMER=arduino
PORT=/dev/ttyS9
BAUD=115200
CFLAGS=-Os -DF_CPU=$(F_CPU) -mmcu=$(MCU) -I.


$(BIN).hex: $(BIN).elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

$(BIN).elf: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

install: $(BIN).hex
	$(AVRDUDE) -c $(PROGRAMMER) -p $(MCU) -P $(PORT) -b $(BAUD) -U flash:w:$<

clean:
	rm -f $(BIN).elf $(BIN).hex $(OBJS)
