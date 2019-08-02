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

dmaport     equ 0bh
ramtop      equ 0ffffh

; FatFS commands
F_OPEN                  equ 00h
F_CLOSE                 equ 01h
F_READ                  equ 02h
F_WRITE                 equ 03h
F_LSEEK                 equ 04h
F_TRUNCATE              equ 05h
F_SYNC                  equ 06h
F_FORWARD               equ 07h
F_EXPAND                equ 08h
F_GETS                  equ 09h
F_PUTC                  equ 0ah
F_PUTS                  equ 0bh
F_PRINTF                equ 0ch
F_TELL                  equ 0dh
F_EOF                   equ 0eh
F_SIZE                  equ 0fh
F_ERROR                 equ 10h
F_OPENDIR               equ 11h
F_CLOSEDIR              equ 12h
F_READDIR               equ 13h
F_FINDFIRST             equ 14h
F_FINDNEXT              equ 15h
F_STAT                  equ 16h
F_UNLINK                equ 17h
F_RENAME                equ 18h
F_CHMOD                 equ 19h
F_UTIME                 equ 1ah
F_MKDIR                 equ 1bh
F_CHDIR                 equ 1ch
F_CHDRIVE               equ 1dh
F_GETCWD                equ 1eh
F_MOUNT                 equ 1fh
F_MKFS                  equ 20h
F_FDISK                 equ 21h
F_GETFREE               equ 22h
F_GETLABEL              equ 23h
F_SETLABEL              equ 24h
F_SETCP                 equ 25h
F_DISK_STATUS           equ 26h
F_DISK_INITIALIZE       equ 27h
F_DISK_READ             equ 28h
F_DISK_WRITE            equ 29h
F_DISK_IOCTL            equ 2ah
F_GET_FATTIME           equ 2bh



; FatFS file open flags
FA_READ				    equ 01h
FA_WRITE			    equ 02h
FA_OPEN_EXISTING	    equ 00h
FA_CREATE_NEW		    equ 04h
FA_CREATE_ALWAYS	    equ 08h
FA_OPEN_ALWAYS		    equ 10h
FA_OPEN_APPEND		    equ 30h

; FatFS result codes
FR_OK				    equ 0h
FR_DISK_ERR			    equ 1h
FR_INT_ERR				equ 2h
FR_NOT_READY			equ 3h
FR_NO_FILE				equ 4h
FR_NO_PATH				equ 5h
FR_INVALID_NAME		    equ 6h
FR_DENIED				equ 7h
FR_EXIST				equ 8h
FR_INVALID_OBJECT		equ 9h
FR_WRITE_PROTECTED		equ 0ah
FR_INVALID_DRIVE		equ 0bh
FR_NOT_ENABLED			equ 0ch
FR_NO_FILESYSTEM		equ 0dh
FR_MKFS_ABORTED		    equ 0eh
FR_TIMEOUT				equ 0fh
FR_LOCKED				equ 10h
FR_NOT_ENOUGH_CORE		equ 11h
FR_TOO_MANY_OPEN_FILES	equ 12h
FR_INVALID_PARAMETER	equ 13h


    ; set up stack
    ld      hl, ramtop
    ld      sp, hl

    ; set up mailbox address
    ld      hl, mailbox
    call    setmailbox

    ; open file for reading
    ld      a, FA_READ
    ld      (mailbox.mode), a
    ld      a, F_OPEN
    ld      hl, filename
    ld      bc, fnlen
    call    dodma
    jp      nz, end

    ; read up to 8000h bytes from file
    ld      a, F_READ
    ld      de, filedata
    ld      bc, 8000h
    call    dodma
    jp      nz, end

    ; close the file
    ld      a, F_CLOSE
    call    dodma
    
end
    halt

; initiate a DMA transfer
;       A = DMA command
;       HL = address of input buffer
;       DE = address of output buffer
;       BC = length of buffer
;       returns result code in A; NZ set if error
dodma
    ld      (mailbox.cmd), a
    ld      (mailbox.inaddr), hl
    ld      (mailbox.outaddr), de
    ld      (mailbox.length), bc
    xor     a
    out     (dmaport), a
    ld      a, (mailbox.fr)
    cp      FR_OK
    ret

; set up the mailbox address to use for DMA communication
setmailbox
    ld      c, dmaport
    in      a, (c)           ; clear DMA mailbox address
    out     (c), l           ; set new DMA mailbox address
    out     (c), h
    ret

mailbox
.cmd        db 0
.fr         db 0
.inaddr     dw 0
.outaddr    dw 0
.length     dw 0
.ofs        dd 0
.mode       db 0
.mask       db 0
.fpdp       ds 36

filename    db "/TEST.TXT", 0
fnlen       equ $-filename
filedata    db 0