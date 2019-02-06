# CP/M BIOS for z80ctrl

This is a CP/M BIOS with z80ctrl DMA interface support. It will fall back to the standard Altair disk interface if the DMA interface is not present.  This allows the BIOS to be assembled and installed to a disk image under SIMH, which is much faster than compiling it from z80ctrl.  Once the BIOS has been assembled, the disk image can be copied to the z80ctrl SD card for further testing.

`CBIOSX.MAC` is the main source. It is derived from the SIMH BIOS by Peter Schorn with the SIMH hard drive interface stripped out and z80ctrl DMA interface added in its place.  The DMA interface uses a track buffering scheme inspired by the CPM 2.2b BIOS for Altair 8800 by Mike Douglas.

To generate a disk image, you need the SIMH AltairZ80 emulator and its CP/M 2 software distribution. Copy altairz80.exe and cpm2.dsk to this directory and start altairz80.exe. Here is how to update the disk image with the sources found here:

```
Altair 8800 (Z80) simulator V3.9-0 build 1625 (scp created Feb 10 2013 at 09:37:45 with gcc 4.2.4)

sim> do biosdev

64K CP/M Version 2.2 (SIMH ALTAIR 8800, BIOS V1.27, 2 HD, 02-May-2009)

A>r cbiosx.mac

READ V-2.25 (19-May-14) SIMH Interface V004
Read from "CBIOSX.MAC" and write to "CBIOSX.MAC".
Previous file of same name deleted.
35.625kB written.

A>r cfgccp.lib

READ V-2.25 (19-May-14) SIMH Interface V004
Read from "CFGCCP.LIB" and write to "CFGCCP.LIB".
Previous file of same name deleted.
0.875kB written.

A>do syscpm2
SuperSUB V1.1

A>; CREATE A BOOTABLE IMAGE ON DISK A: OF CP/M WITH CCP
A>; BASED ON ORIGINAL DIGITAL RESEARCH SOURCES FOR CCP AND BDOS
A>; REQUIRED SOURCES ARE CFGCCP.LIB, MOVER.MAC, CCP.MAC, BDOS.MAC, CBIOSX.MAC
A>; REQUIRED PROGRAMS: M80.COM, L80.COM, DDT.COM, BOOT.COM, BOOTGEN.COM
A>XSUB

... assembler messages elided ...

A>; NOW PERFORM A COLD BOOT TO GET RID OF XSUB
A>; THIS RESTORES THE ORIGINAL BIOS JUMP VECTOR WHICH IS REQUIRED BY BOOTGEN
A>BOOT

64K CP/M Version 2.2 (SIMH ALTAIR 8800, BIOS V1.27, 2 HD, 02-May-2009)

A>; WRITE BOOT FILE TO RESERVED TRACKS, MUST BE LAST COMMAND
A>BOOTGEN CPMBOOT.COM A:

BOOTGEN V-1.08 (15-Oct-06)
CP/M 2 detected.
90 sectors written.

64K CP/M Version 2.2 for z80ctrl

A>
```

You can tell that the compilation has been successful because the SIMH signon message is replaced with a z80ctrl signon.  The disk image can be used as normal under SIMH, and then copied to the z80ctrl SD card to test the DMA interface.

Each time you make changes to the BIOS, you will need to rerun `r CBIOSX.MAC` to load it into the disk image and `do syscpm2` to assemble it and update the disk image.  You can also type `do mccl cbiosx` to get a listing of the assembled source.