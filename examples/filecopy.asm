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

            org 100h

ramtop:     equ 0ffffh
uarts:      equ 10h
uartd:      equ 11h
lf:         equ 0ah
cr:	        equ 0dh

    ld      hl, ramtop                  ; set up stack
    ld      sp, hl

    call    f_init                      ; initialize file DMA mailbox

    ld      de, filebuf                 ; get command-line buffer
    ld      bc, buflen
    call    f_cli_first
    cp      3                           ; verify that there are at least 2 params
    jp      c, usage

    call    f_cli_next                  ; skip over command name

    ld      hl, sourcefp                ; open source file for reading
    ld      a, FA_READ
    call    f_open
    jp      nz, error

    call    f_cli_next

    ld      hl, destfp                  ; open destination file for writing
    ld      a, FA_WRITE | FA_CREATE_NEW
    call    f_open
    jp      nz, error

    ld      de, filebuf
loop:
    ld      hl, sourcefp                ; fill buffer with bytes from source
    ld      bc, buflen
    call    f_read
    jp      nz, error
    ld      hl, destfp                  ; write bytes from buffer to destination
    ld      bc, (f_retlen)
    call    f_write
    jp      nz, error
    ld      hl, buflen                  ; check whether last read was less than full buffer
    or      a
    sbc     hl, bc
    add     hl, bc
    jp      z, loop                     ; if not, go again
    jp      close

usage:
    ld      hl, usage_text              ; print usage
    call    strout
    jp      close

error:
    call    f_find_error                ; print error message
    ex      de, hl
    call    strout
    ld      hl, crlf
    call    strout

close:
    ld      hl, sourcefp
    call    f_close                     ; close the source file
    ld      hl, destfp
    call    f_close                     ; close the file

    halt


strout:     ld a, (hl)
            and a
            ret z
	        call chrout
            inc hl
            jr strout

chrout:     push af
rdylp:	    in a, (uarts)
	        bit 1,a
	        jr z, rdylp
	        pop af
            out (uartd), a
	        ret

sourcefp    ds 42                       ; file pointers
destfp      ds 42

filebuf     ds 256                      ; temporary buffer
buflen      equ $-filebuf

usage_text  db "usage: filecopy <source> <dest>"
crlf        db cr, lf, 0
colon       db ": ", 0

include     "filedma.asm"