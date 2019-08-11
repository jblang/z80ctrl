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
buflen:     equ 32*1024

    jp      start

include     "filedma.asm"

start:
    ld      hl, ramtop                  ; set up stack
    ld      sp, hl

    call    f_init                      ; initialize file DMA mailbox

    f_setw  f_outaddr, filebuf          ; set up file buffer
    f_setw  f_maxlen, buflen

    call    f_cli_first                 ; get command-line arguments
    cp      3                           ; verify that there are at least 2 params
    jp      c, usage                    ; if not, show usage

    call    f_cli_next                  ; get pointer to first filename
    ld      (f_inaddr), de

    f_setw  f_fpaddr, sourcefp          ; open source file for reading
    f_setb  f_mode, FA_READ
    f_call  F_OPEN
    jp      nz, error

    call    f_cli_next                  ; get pointer to second filename
    ld      (f_inaddr), de

    f_setw  f_fpaddr, destfp            ; open destination file for writing
    f_setb  f_mode, FA_WRITE | FA_CREATE_NEW
    f_call  F_OPEN
    jp      nz, error

    f_setw  f_inaddr, filebuf           ; set input address back to beginning of buffer

loop:
    f_setw  f_fpaddr, sourcefp          ; fill buffer with bytes from source
    f_call  F_READ
    jp      nz, error

    f_setw  f_fpaddr, destfp            ; write bytes from buffer to destination
    f_cpyw  f_maxlen, f_retlen          ; copy number of bytes returned by read
    f_call  F_WRITE
    jp      nz, error

    ld      hl, buflen                  ; check whether last read was less than full buffer
    ld      de, (f_maxlen)
    or      a
    sbc     hl, de
    add     hl, de
    jp      z, loop                     ; if not, go again
    jp      close

usage:
    ld      hl, usage_text              ; print usage
    call    strout
    jp      close

error:
    call    f_error                     ; print error message
    ex      de, hl
    call    strout
    ld      hl, crlf
    call    strout

close:
    f_setw  f_fpaddr, sourcefp
    f_call  F_CLOSE                     ; close the source file
    f_setw  f_fpaddr, destfp
    f_call  F_CLOSE                     ; close the file

    halt

; output null-terminated string
;   HL = pointer to string
strout:     ld a, (hl)                  ; get next character
            and a                       ; check if it's null
            ret z                       ; if so, we're done
	        call chrout                 ; otherwise output character
            inc hl                      ; point to next character
            jr strout                   ; repeat

; output character
chrout:     push af                     ; save character
rdylp:	    in a, (uarts)               ; check if uart is ready to receive
	        bit 1,a
	        jr z, rdylp                 ; if not, wait until it is
	        pop af                      ; restore character
            out (uartd), a              ; output it
	        ret

usage_text  db "usage: filecopy <source> <dest>"
crlf        db cr, lf, 0
colon       db ": ", 0

sourcefp    ds fplen                    ; file pointers for fatfs
destfp      ds fplen

filebuf     equ $
