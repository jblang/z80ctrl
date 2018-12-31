; z80ctrl (https://github.com/jblang/z80ctrl)
; Copyright 2018 J.B. Langston
;
; Permission is hereby granted, free of charge, to any person obtaining a 
; copy of this software and associated documentation files (the "Software"), 
; to deal in the Software without restriction, including without limitation 
; the rights to use, copy, modify, merge, publish, distribute, sublicense, 
; and/or sell copies of the Software, and to permit persons to whom the 
; Software is furnished to do so, subject to the following conditions:
; 
; The above copyright notice and this permission notice shall be included in
; all copies or substantial portions of the Software.
; 
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
; FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
; DEALINGS IN THE SOFTWARE.

; @file lcdclock.asm: LCD clock example for IO expander card

; z80ctrl IO expander base address
ioxbase:        equ 0

; chip addresses
rtcaddr:        equ 0
ioxaddr:        equ 1

; rtc registers
rtcsecond:      equ 0
rtcminute:      equ 1
rtchour:        equ 2
rtcweekday:     equ 3
rtcday:         equ 4
rtcmonth:       equ 5
rtcyear:        equ 6

; rtc hour bit positions
rtc1224:        equ 6
rtcampm:        equ 5

; io expander registers
iodira:         equ $00
gpioa:          equ $12

; lcd control lines
lcden:	        equ 6
lcdrw:	        equ 5
lcdrs:	        equ 4

; lcd registers
lcdinstreg:	equ $00
lcddatareg:	equ $10

; lcd commands
lcdclear:       equ $01
lcdhome:        equ $02
lcdentrymode:   equ $04
lcdenable:      equ $08
lcdshiftcursor: equ $10
lcdshiftdispl:  equ $18
lcdfunction:    equ $20
lcdsetcgaddr:   equ $40
lcdsetddaddr:   equ $80

; lcd entry mode bits
lcdentryinc:    equ $02
lcdentrydec:    equ $00
lcdentdisplay:  equ $01
lcdentcursor:   equ $00

; lcd display enable bits
lcddisplay:     equ $04
lcdcursor:      equ $02
lcdblink:       equ $01

; lcd shift bits
lcdright:       equ $04
lcdleft:        equ $00

; lcd function bits
lcd8bit:        equ $10
lcd4bit:        equ $00
lcd1line:       equ $00
lcd2line:       equ $08
lcd5x8font:     equ $00
lcd5x10font:    equ $04

; lcd reset sequence
lcdreset1:      equ $33
lcdreset2:      equ $32

; lcd busy bit position
lcdbusy:	equ 7

; top of ram for stack pointer
ramtop: equ $ffff

        org $100

        ld	sp, ramtop              ; set up stack pointer
        ld      hl, lcdconfig           ; configure LCD
        call	lcdinit
mainloop:
        call	lcdtime                 ; print the time
        ld      a, ' '
        call    lcdchrout
        call    lcddate                 ; and the date
        ld      a, lcdhome              ; send the cursor home
        call    lcdsendcmd
        jp      mainloop                ; again

; LCD configuration table
lcdconfig:
        db lcdreset1                    ; reset sequence from datasheet
        db lcdreset2
        db lcdfunction | lcd4bit | lcd2line | lcd5x8font
        db lcdenable | lcddisplay | lcdblink
        db lcdclear
lcdconfiglen: equ $ - lcdconfig

; print the time on the LCD
lcdtime:
        ld      d, rtcaddr              ; get hour from rtc
        ld      e, rtchour
        call    ioxread
        bit     rtc1224, a              ; see if 12 hour format is set
        ld      b, 0                    ; if not, skip the AM/PM part
        jp      z, ampmdone
        bit     rtcampm, a              ; check whether AM or PM
        res     rtcampm, a              ; clear AM/PM bit
        ld      b, 'A'                  ; load first letter for AM or PM in B
        jp      z, ampmdone
        ld      b, 'P'
ampmdone:        
        res     rtc1224, a              ; clear 24-hour bit
        call    lcdhexbyte              ; output hours
        ld      a, ':'
        call    lcdchrout
        ld      e, rtcminute            ; output minutes
        call    ioxread
        call    lcdhexbyte
        ld      a, ':'
        call    lcdchrout
        ld      e, rtcsecond            ; output seconds
        call    ioxread
        call    lcdhexbyte
        ld      a, b                    ; check whether AM/PM was set
        or      a
        ret     z                       ; if not, we're done
        call    lcdchrout               ; if so, output the letter we stored in B
        ld      a, 'M'                  ; then output M
        call    lcdchrout
        ret

; print the date on the LCD
lcddate:
        ld      d, rtcaddr              ; output month
        ld      e, rtcmonth
        call    ioxread
        call    lcdhexbyte
        ld      a, '/'
        call    lcdchrout
        ld      e, rtcday               ; output day
        call    ioxread
        call    lcdhexbyte
        ld      a, '/'
        call    lcdchrout
        ld      e, rtcyear              ; output year
        call    ioxread
        call    lcdhexbyte
        ret

; initialize LCD from configuration table
;       HL = configuration table address
lcdinit:
        ld      b, lcdconfiglen
lcdinit2:
        ld      a, (hl)                 ; get configuration command from table
        call    lcdsendcmd              ; send it to the LCD
        inc	hl
        djnz    lcdinit2                ; next...
        ret

; Display hex value in A on LCD
;       A = number to display
lcdhexbyte:
        push    af                      ; shift down the top nybble
        rra
        rra
        rra
        rra
        call    lcdhexnibble            ; print it
        pop     af                      ; restore the full byte
lcdhexnibble:
        and     $0f                     ; mask off the top nybble
        add     $90                     ; convert number to ASCII
        daa
        adc     $40
        daa
        call    lcdchrout               ; print it
        ret

; send a null-terminated string to the LCD
;       HL = address of string
lcdstrout:
        ld 	a, (hl)                 ; get char from string
        and	a                       ; check if it's null
        ret	z                       ; if so, we're done
        call	lcdchrout               ; otherwise, print it
        inc	hl                      ; next...
        jr	lcdstrout

; send a character to the LCD
;       A = character to send
lcdchrout: 
        push    bc
        ld	c, lcddatareg           ; write to lcd data register
        call    lcdwrite
        pop     bc
        ret

; send a command to the LCD
;       A = command to send
lcdsendcmd:
        push    bc
        ld      c, lcdinstreg           ; write to lcd instruction register
        call    lcdwrite
        pop     bc
        ret

; write byte to LCD
;       C = register to write (instruction/data)
;       A = value to write
lcdwrite:
        push    de
        push    bc
        push    af
        ld      d, ioxaddr              ; Make GPIO A bits output
        ld      e, iodira
        xor     a
        call    ioxwrite
        ld      e, gpioa
        pop     af
        push    af                      ; get upper nybble
        srl     a
        srl     a
        srl     a
        srl     a
        or	c
        call	ioxwrite
        set     lcden, a                ; clock in upper nybble
        call    ioxwrite
        res     lcden, a
        call    ioxwrite
        pop     af                      ; get lower nybble
        push    af
        and     $f                    	; clock in lower nybble
        or	c
        call	ioxwrite
        set     lcden, a
        call    ioxwrite
        res     lcden, a
        call    ioxwrite
lcdwait:
        ld      c, lcdinstreg           ; wait for write to finish
        call	lcdread
        bit	lcdbusy, a
        jp	nz, lcdwait
        pop     af
        pop     bc
        pop     de
        ret

; read from LCD
;       C = register to read (instruction/data)
;       value returned in A
lcdread:
        push    bc
        push    de
        ld      d, ioxaddr
        ld      e, iodira               ; make upper nybble input, lower nybble output
        ld      a, $0f
        call    ioxwrite
        ld      e, gpioa
        ld      a, c
        set     lcdrw, a                ; clock in upper nybble
        call	ioxwrite
        set     lcden, a
        call    ioxwrite
        call    ioxread
        res     lcden, a
        call    ioxwrite
        add     a, a                    ; move to upper nybble and save in b
        add     a, a
        add     a, a
        add     a, a
        ld      b, a
        ld      a, c                    ; clock in lower nybble
        set     lcdrw, a
        call	ioxwrite
        set     lcden, a
        call    ioxwrite
        call    ioxread
        res     lcden, a
        call    ioxwrite
        and     $0f                     ; combine with upper nybble
        or      b
        pop     de
        pop     bc
        ret

; read from the IO expander
;       D = chip address
;       E = register address
;       A = register value
ioxwrite:
        push    bc
        ld      c, ioxbase              ; set chip address
        out     (c), d
        inc     c                       ; set register address              
        out     (c), e
        inc     c                       ; write to register
        out     (c), a
        pop     bc
        ret

; read from the IO expander
;       D = chip address
;       E = register address
;       register value returned in A
ioxread:
        push    bc
        ld      c, ioxbase              ; set chip address
        out     (c), d
        inc     c                       ; set register address
        out     (c), e
        inc     c                       ; read from register
        in      a, (c)
        pop     bc
        ret