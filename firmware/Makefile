# All build variables can be set on command line. Build with a
# command of this form:
#
#    TMS_BASE=0xBE SN76489_PORT=0xFF BOARD_REV=4 MSX_KEY_BASE=0xA9 PORT=/dev/tty.usbserial-00000000 make install
#

# Hardware revision (Important: must be set to correct value)
# BOARD_REV?=6

# Bank address for z80ctrl-style banked RAM; comment out to disable support
# BANK_PORT=0x78

# Base address for RomWBW-style banked RAM; comment out to disable support
# BANK_BASE=0x78

# Uncomment to enable support for SST39F0x0 flash chips on RomWBW boards
# SST_FLASH=1

# Uncomment to enable DS1306 RTC support (used on IOX board)
# DS1306_RTC=1

# Uncomment to enable DS1302 RTC support (used on CPU/RAM/RTC board)
# DS1302_RTC=1

# Base address TMS9918A chip; comment out to disable support
# TMS_BASE=0xBE

# Port assigned to SN76489 sound chip
# SN76489_PORT=0xFF

# Base port on which to expose I/O expander card to Z80
# IOX_BASE=0x00

# Base port for MSX keyboard
# MSX_KEY_BASE=0xA9

# SD Card Adapter - Set set 1 for the AdaFruit adapter, leave commented out for the Polulu adapter.
# SD_CARD_ADAFRUIT = 1

# Current git hash
GITVERSION:= $(shell git log -1 --pretty='%h')

# MCU type and frequency
MCU?=atmega1284p
F_CPU?=20000000L

# Programmer options
PROGRAMMER?=arduino
PORT?=COM3
BAUD?=115200

AVRCC?=avr-gcc
OBJCOPY?=avr-objcopy
AVRDUDE?=avrdude

CLEAN?=rm -rf

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
	filedma.o \
	ihex.o \
	disasm.o \
	util.o \
	xmodem.o \
	bdosemu.o \
	ffwrap.o \
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
ifdef SN76489_PORT
 	FEATURE_DEFINES += -DSN76489_PORT=$(SN76489_PORT)
endif
ifdef BANK_BASE
	FEATURE_DEFINES += -DBANK_BASE=$(BANK_BASE)
endif
ifdef BANK_PORT
	FEATURE_DEFINES += -DBANK_PORT=$(BANK_PORT)
endif
ifdef DS1306_RTC
	FEATURE_DEFINES += -DDS1306_RTC -DUSE_RTC
	OBJS += ds1306.o
endif
ifdef DS1302_RTC
	FEATURE_DEFINES += -DDS1302_RTC -DUSE_RTC
	OBJS += ds1302.o
endif
ifdef IOX_BASE
	FEATURE_DEFINES += -DIOX_BASE=$(IOX_BASE)
endif
ifdef RTC_BASE
	FEATURE_DEFINES += -DRTC_BASE=$(RTC_BASE)
endif
ifdef MSX_KEY_BASE
	FEATURE_DEFINES += -DMSX_KEY_BASE=$(MSX_KEY_BASE)
	OBJS += msxkey.o
endif
ifdef SD_CARD_ADAFRUIT
	FEATURE_DEFINES += -DMISO_INPUT_PULLUP
endif

CFLAGS=-std=c99 -flto -Os $(FEATURE_DEFINES) -DF_CPU=$(F_CPU) -DGITVERSION="\"${GITVERSION}\"" -mmcu=$(MCU) -I.


%.o: %.c
	$(AVRCC) $(CFLAGS) $(LDFLAGS) -c -o $@ $<

$(BIN).hex: $(BIN).elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

$(BIN).elf: $(OBJS)
	$(AVRCC) $(CFLAGS) $(LDFLAGS) -o $@ $^

install: $(BIN).hex
	$(AVRDUDE) -c $(PROGRAMMER) -p $(MCU) -P $(PORT) -b $(BAUD) -U flash:w:$<

clean:
	$(CLEAN) $(BIN).elf $(BIN).hex $(OBJS)
