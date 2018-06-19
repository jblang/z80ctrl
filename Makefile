# Hardware revision (Important: must be set to correct value)
#BOARD_REV=3

# Base address for RomWBW-style paging; comment out to disable support
#PAGE_BASE=0x28

# Uncomment to enable support for SST39F0x0 flash chips on RomWBW boards
#SST_FLASH=1

# Base address TMS9918A chip; comment out to disable support
#TMS_BASE=0x98

# MCU type and frequency
MCU=atmega1284p
F_CPU=20000000L

# Programmer options
PROGRAMMER=arduino
PORT=/dev/ttyS9
BAUD=115200

CC=avr-gcc
OBJCOPY=avr-objcopy
AVRDUDE=avrdude

BIN=z80ctrl
FF_OBJS=ff.o diskio.o mmc_avr_spi.o
OBJS=	cli.o \
	uart.o \
	spi.o \
	iox.o \
	bus.o \
	z80.o \
	iorq.o \
	sioemu.o \
	diskemu.o \
	ihex.o \
	disasm.o \
	util.o \
	$(FF_OBJS)

ifdef BOARD_REV
	FEATURE_DEFINES += -DBOARD_REV=$(BOARD_REV)
endif
ifdef SST_FLASH
	FEATURE_DEFINES +=-DSST_FLASH
	OBJS += flash.o
endif
ifdef TMS_BASE
 	FEATURE_DEFINES += -DTMS_BASE=$(TMS_BASE)
	OBJS += tms.o
endif
ifdef PAGE_BASE
	FEATURE_DEFINES += -DPAGE_BASE=$(PAGE_BASE)
endif

CFLAGS=-std=c99 -Os $(FEATURE_DEFINES) -DF_CPU=$(F_CPU) -mmcu=$(MCU) -I.

$(BIN).hex: $(BIN).elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

$(BIN).elf: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

install: $(BIN).hex
	$(AVRDUDE) -c $(PROGRAMMER) -p $(MCU) -P $(PORT) -b $(BAUD) -U flash:w:$<

clean:
	rm -f $(BIN).elf $(BIN).hex $(OBJS)
