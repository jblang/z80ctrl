## z80ctrl with AVR128DB64

This is a experimental design for a new z80ctrl using the [AVR128DB64](https://www.microchip.com/en-us/product/avr128db64). 

**Important: Right now, I'm just experimenting with pin assignments and thinking through how I might lay out such a board. It is absolutely not ready for manufacturing.**

This part has several advantages over the current design using an ATmega1284P and I/O expander:

- Is a 64-pin part with more than enough I/O to interface with the RC2014 bus without any I/O expander. 
- Has configurable logic that could be used to assert the wait signal on I/O requests without having to use an external nand gate.
- Has multi-voltage I/O that could be used to interface the SD card directly without any level shifters.

The big disadvantage is that it's a fine-pitched surface mount part so it would be extremely difficult to hand solder, but I've been wanting to try out JLCPCB or PCBWay's board assembly service anyway.