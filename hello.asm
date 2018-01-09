        org 0
nl:     equ 12
uart:   equ 0

        ld hl, msg
        call strout
        in a,(uart)
        out (uart),a
        halt

strout: ld a,(hl)
        and a
        ret z
        out (uart),a
        inc hl
        jr strout


msg:    db "hello, world", nl, 0