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

ramtop      equ 0ffffh
pathlen     equ 16

    ld      hl, ramtop                  ; set up stack
    ld      sp, hl

    call    f_init                      ; initialize file DMA mailbox

    ld      hl, sourcefp                ; open source file for reading
    ld      de, sourcename
    ld      a, FA_READ
    call    f_open
    jp      nz, done

    ld      hl, destfp                  ; open dest file for writing
    ld      de, destname
    ld      a, FA_WRITE | FA_CREATE_NEW
    call    f_open
    jp      nz, done

    ld      de, filebuf
loop:
    ld      hl, sourcefp                ; fill buffer with bytes from source
    ld      bc, buflen
    call    f_read
    jp      nz, done
    ld      hl, destfp                  ; write bytes from buffer to destination
    ld      bc, (f_retlen)
    call    f_write
    jp      nz, done
    ld      hl, buflen                  ; check whether last read was less than full buffer
    or      a
    sbc     hl, bc
    add     hl, bc
    jp      z, loop                     ; if not, go again

done:
    ld      hl, sourcefp
    call    f_close                     ; close the source file
    ld      hl, destfp
    call    f_close                     ; close the file

    halt

sourcefp    ds 42           ; file pointers
destfp      ds 42

sourcename  db "/TEST.TXT", 0
destname    db "/COPY.TXT", 0

filebuf     ds 256          ; temporary space for file
buflen      equ $-filebuf

include     "filedma.asm"