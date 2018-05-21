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

; @file hello.asm: hello world example program

         org 100h

lf:     equ 0ah
cr:	equ 0dh
ramtop: equ 0ffffh
uarts:  equ 10h
uartd:  equ 11h

        ld sp, ramtop
        ld hl, msg
        call strout
        halt

strout: ld a, (hl)
        and a
        ret z
	call chrout
        inc hl
        jr strout

chrout: push af
rdylp:	in a, (uarts)
	bit 1,a
	jr z, rdylp
	pop af
        out (uartd), a
	ret

msg:    db "hello, world", cr, lf, 0
