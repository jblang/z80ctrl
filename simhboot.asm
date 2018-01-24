; modified cold boot routine to be put into ROM

; The sectors of a track are read in the following order:
; first even sectors, then odd sectors in ascending order
; 0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,
; 1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31
; T0[S8 - S31], T1, T2, T3, T4, T5[S0 - S23]
;    28       + 32+ 32+ 32+ 32+ 28 = 184 sectors = 23552 byte (5C00)
; loads sectors to 0 .. 5bff and jumps to 0 when done

        .Z80
        aseg

cold            equ     05c00h          ; starting address of loader after relocation
rom             equ     0ff00h

;       Address         Mode    Function
;       -------         ----    --------
;       selout          Out     Selects and enables controller and drive
;       statin          In      Indicates status of drive and controller
;       dskcon          Out     Controls disk function
;       secpos          In      Indicates current sector position of disk
;       dskwrit         Out     Write data
;       dskread         In      Read data

selout  equ     8               ; port to select and enable controller and drive (OUT)
;       +---+---+---+---+---+---+---+---+
;       | C | X | X | X |   Device      |
;       +---+---+---+---+---+---+---+---+
;
;       C       = If this bit is 1, the disk controller selected by 'device' is
;                 cleared. If the bit is zero, 'device' is selected as the
;                 device being controlled by subsequent I/O operations.
;       X       = not used
;       Device  = value zero thru 15, selects drive to be controlled.

statin  equ     8               ; port indicating status of drive and controller (IN)
;       +---+---+---+---+---+---+---+---+
;       | R | Z | I | X | X | H | M | W |
;       +---+---+---+---+---+---+---+---+
;
;       W - When 0, write circuit ready to write another byte.
;       M - When 0, head movement is allowed
;       H - When 0, indicates head is loaded for read/write
;       X - not used (will be 0)
;       I - When 0, indicates interrupts enabled (not used this simulator)
;       Z - When 0, indicates head is on track 0
;       R - When 0, indicates that read circuit has new byte to read

dskcon  equ     9               ; port to control disc function (OUT)
;       +---+---+---+---+---+---+---+---+
;       | W | C | D | E | U | H | O | I |
;       +---+---+---+---+---+---+---+---+
;
;       I - When 1, steps head IN one track
;       O - When 1, steps head OUT one track
;       H - When 1, loads head to drive surface
;       U - When 1, unloads head
;       E - Enables interrupts (ignored by this simulator)
;       D - Disables interrupts (ignored by this simulator)
;       C - When 1 lowers head current (ignored by this simulator)
;       W - When 1, starts Write Enable sequence:
;           W bit on device 'statin' (see above) will go 1 and data will be read from
;           port 'dskread' until 137 bytes have been read by the controller from
;           that port. The W bit will go off then, and the sector data will be written
;           to disk. Before you do this, you must have stepped the track to the desired
;           number, and waited until the right sector number is presented on
;           device 'secpos', then set this bit.

secpos  equ     9               ; port to indicate current sector position of disk (IN)
;       As the sectors pass by the read head, they are counted and the
;       number of the current one is available in this register.
;
;       +---+---+---+---+---+---+---+---+
;       | X | X |  Sector Number    | T |
;       +---+---+---+---+---+---+---+---+
;
;       X               = Not used
;       Sector number   = binary of the sector number currently under the head, 0-31.
;       T               = Sector True, is a 1 when the sector is positioned to read or write.

dskread equ     10              ; port to read data (IN)

spt             equ     32              ; sectors per track
secsiz          equ     128             ; physical sector size
bootorg         equ     0000            ; jump to this address after loading
simhport        equ     0feh            ; SIMH port
simhreset       equ     14              ; SIMH reset command
stoptimer       equ     22              ; SIMH stop timer interrupt
setbankselect   equ     12              ; SIMH command to set memory bank
hasbankedmemory equ     18              ; SIMH command to check for banked memory
unitnooffset1   equ     (selds0+1-cold)+(dest-rom)
unitnooffset2   equ     (gotoit+1-cold)+(dest-rom)

seldsk          equ     selds0+1        ; address to select disk
resdsk          equ     gotoit+1        ; address to reset disk

        org     100h
        .phase  rom
        di
        ld      b,128                   ; sending SIMHRESET 128 times guarantees reset
        ld      a,simhreset             ; reset command
reset:  out     (simhport),a            ; reset SIMH interface
        dec     b
        jp      nz,reset                ; again
        ld      a,stoptimer             ; stop timer interrupts command
        out     (simhport),a            ; send it
        ld      a,hasbankedmemory       ; check for banked memory support
        out     (simhport),a            ; send command
        in      a,(simhport)            ; receive result
        or      a                       ; check for <> 0
        jp      z,move1                 ; no banked memory support detected
        ld      a,setbankselect         ; next command is select bank
        out     (simhport),a            ; send it
        xor     a                       ; bank for boot is 0
        out     (simhport),a            ; reset bank to 0
move1:  ld      hl,cold
        ld      de,dest
        ld      c,altbuf - cold
move2:  ld      a,(de)
        ld      (hl),a
        inc     de
        inc     hl
        dec     c
        jp      nz,move2
        jp      cold
dest    equ     $
        .dephase

        .phase  cold
        ld      sp,stack
selds0: ld      a,0             ; the address of <0> is "unitNoOffset1"
        out     (selout),a      ; select it
        ld      a,4             ; load head command
        out     (dskcon),a      ; load head to drive surface
        jp      cktk0
back1:  in      a,(statin)
        and     2               ; head movement mask
        jp      nz,back1        ; loop until head movement is allowed
        ld      a,2             ; step out command
        out     (dskcon),a      ; step head out one track
cktk0:  in      a,(statin)
        and     40h             ; head on track zero mask
        jp      nz,back1        ; loop until head is on track zero
        ld      de,bootorg      ; destination load address
        ld      b,8             ; first sector to read on track zero
nextsc: push    bc              ; save current sector to read, <C> is undefined
        push    de              ; save current destination load address
        ld      de,8086h        ; <D> := 80h, <E> := 86h
        ld      hl,altbuf       ; address of sector buffer
findsc: in      a,(secpos)
        rra
        jp      c,findsc        ; loop until sector is positioned to read or write
        and     (spt-1)         ; <A> now contains the sector under the head
        cp      b               ; compare with desired sector
        jp      nz,findsc       ; loop until done
readsc: in      a,(statin)      ; get disk status
        or      a               ; set sign of byte
        jp      m,readsc        ; loop until disk has new byte to read
        in      a,(dskread)     ; read byte of sector
        ld      (hl),a          ; store into buffer
        inc     hl              ; point to next position in buffer
        dec     e               ; decrement byte counter
        jp      nz,readsc       ; repeat if byte counter not yet zero
donesc: pop     de              ; restore current destination load address, <DE> is destination
        ld      hl,altbuf+3     ; ignore first three byte of buffer, <HL> is source
        ld      b,secsiz        ; 128 bytes in a sector
ldir:   ld      a,(hl)          ; get byte from source
        ld      (de),a          ; put byte to destination
        inc     hl              ; point to next source address
        inc     de              ; point to next destination address
        dec     b               ; decrement number of bytes to move
        jp      nz,ldir         ; not zero, move again
        pop     bc              ; <B> is current sector, <C> is undefined
        ld      hl,cold         ; when <DE> reaches this address we are done
        ld      a,d
        cp      h
        jp      nz,decide
        ld      a,e
        cp      l
decide: jp      nc,gotoit       ; jump if everything loaded
        inc     b               ; compute next sector number
        inc     b
        ld      a,b
        cp      spt             ; compare new sector number with sectors per track
        jp      c,nextsc        ; continue if less
        ld      b,1             ; otherwise prepare for odd numbered sectors
        jp      z,nextsc        ; if old sector number was equal to sectors per track
stepin: in      a,(statin)
        and     2               ; head movement mask
        jp      nz,stepin       ; loop until head movement is allowed
        ld      a,1             ; step in one track command
        out     (dskcon),a      ; step in one track
        ld      b,0             ; start with even sectors
        jp      nextsc
gotoit: ld      a,80h           ; the address of <80H> is "unitNoOffset2"
        out     (selout),a
        ei
        jp      bootorg

altbuf: ds      137
        ds      16

stack:
        .dephase

        end