        org 0
tx:     equ 0

        ld hl, msg
        call strout
end:    jr end

strout: ld a,(hl)
        and a
        ret z
        out (tx),a
        inc hl
        jr strout


msg:    db "hello, world", 0