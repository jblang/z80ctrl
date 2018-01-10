        org 100h

lf:     equ 0ah
cr:	equ 0dh
ramtop: equ 0ffffh
uarts:  equ 10h
uartd:  equ 11h

        ld sp, ramtop
        ld hl, msg
        call strout
        ret

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
