;------------------------------------------------------------------------------
; z80ctrl CP/M bios v0.99; 11/25/2019 by J.B. Langston
; Uses FatFS DMA interface for z80ctrl
;
; Thanks to:
; 
; CPM-IDE BIOS by Phillip Stevens - for buffered disk routines
; https://github.com/RC2014Z80/RC2014/blob/master/ROMs/CPM-IDE/acia/cpm22bios.asm
;
; SIMH CP/M 2.2 BIOS by Peter Schorn - for serial I/O routines
; https://schorn.ch/altair_4.php
;------------------------------------------------------------------------------

;------------------------------------------------------------------------------
; constants
;------------------------------------------------------------------------------

bios    equ	$		; start of bios
ramtop	equ	0ffffh		; top of ram for temporary stack pointer

wbootv	equ	0h		; warm boot vector
iobyte	equ	03h		; io byte
cdisk	equ	04h		; current disk
bdosv	equ	05h		; bdos vector
buff	equ	80h		; default dma buffer

blksiz	equ	4096		; CP/M allocation size
hstsiz	equ	512		; host disk sector size
hstbuf	equ	ramtop-hstsiz+1	; host buffer address
hstspt	equ	32		; host disk sectors/trk
hstblk	equ	hstsiz/128	; CP/M sects/host buff

cpmbls	equ	4096		; CP/M allocation block size
cpmdir	equ	512		; CP/M directory blocks
cpmspt	equ	hstblk * hstspt	; CP/M sectors/track

secmsk	equ	hstblk-1	; sector mask

wrall	equ	0		; write to allocated
wrdir	equ	1		; write to directory
wrual	equ	2		; write to unallocated

; SIO controller ports
constat	equ	16		; sio port 1 status port
condata	equ	17		; sio port 1 data port
punstat	equ	18		; sio port 2 status port
pundata	equ	19		; sio port 2 data port

; masks for SIO controller (constat, punstat)
mout	equ	02h		; output allowed mask


;------------------------------------------------------------------------------
; boot procedures
;------------------------------------------------------------------------------

include	"filedma.asm"

; cold boot
boot:
	ld	sp,ramtop		; set up temporary stack
	call	f_init			; initialize file DMA mailbox
	f_setw	f_inaddr, d1fn
	f_setw	f_fpaddr, d1fp          ; open drive A image
	f_setw	f_maxlen, hstsiz
	f_setb	f_mode, FA_READ | FA_WRITE
	f_call	F_OPEN
	jp      nz, error
	f_setw	f_outaddr, hstbuf	; set up disk buffer
	f_setw	f_inaddr, hstbuf

	xor	a			; set current drive to A
	ld	(cdisk),a
	inc	a			; initialize iobyte
	ld	(iobyte),a
	jp	initcpm

; warm boot
wboot:
	ld	sp, ramtop
	;f_setw	f_maxlen, hstsiz
	;f_setw	f_outaddr, hstbuf	; point to buffer
	;call	f_cli_first		; get name of cpm loader
	;ld	(f_inaddr), de
	f_setw	f_inaddr, ccpfn		; open CP/M image for reading
	f_setw	f_fpaddr, ccpfp
	f_setb	f_mode, FA_READ
	f_call	F_OPEN
	jp	nz, error
	f_setw	f_outaddr, CBASE	; reload CCP and BDOS
	f_setw	f_maxlen, bios-CBASE
	f_call	F_READ
	jp	nz, error
	f_call	F_CLOSE
	jp	nz, error
	f_setw	f_fpaddr, d1fp		; point back to disk fp
	f_setw	f_maxlen, hstsiz
	f_setw	f_outaddr, hstbuf	; point to disk buffer
	f_setw	f_inaddr, hstbuf

	jp	initcpm

; common initialization
initcpm:
	ld	a,0c3h
	ld	(wbootv),a	; set up warm boot vector
	ld	hl,wboot
	ld	(wbootv+1),hl
	ld	(bdosv),a	; set up bdos vector
	ld	hl,FBASE
	ld	(bdosv+1),hl
	ld	bc,buff		; set dma address
	call	setdma

	xor	a		; set initial drive
	ld	(hstact),a
	ld	(unacnt),a

	ld	a,(cdisk)
	ld	c,a
	jp	CBASE		; start CCP

;------------------------------------------------------------------------------
; serial io
;------------------------------------------------------------------------------

; console status
const:	in	a,(constat)	; get console status
	rra			; I bit into carry
	ld	a,0		; prepare no character available
	ret	nc		; I bit clear means no character, done
	dec	a		; character available, result is 0ffh
	ret			; done

; console input
conin:	in	a,(constat)	; get console status
	rra			; I bit into carry
	jp	nc,conin	; jump back if no character available
	in	a,(condata)	; read character
	and	7fh		; clear bit 8
	ret

; console output
conout:	in	a,(constat)	; get console status
	and	mout		; mask output bit
	jp	z,conout	; jump back if not ready for output
	ld	a,c		; prepare character for output
	out	(condata),a	; do it
	ret

; list/punch output
list:				; list aliased to punch
punch:	in	a,(punstat)	; get punch status
	and	mout		; mask output bit
	jp	z,punch		; jump back if not ready for output
	ld	a,c		; prepare character for output
	out	(pundata),a	; do it
	ret

; list status
prstat:
	xor	a
	ret

; reader input
reader:	in	a,(punstat)	; get reader status
	rra			; I bit into carry
	jp	nc,reader	; jump back if no character available
	in	a,(pundata)	; read character
	ret

;------------------------------------------------------------------------------
; bios disk interface
;------------------------------------------------------------------------------

; select disk
seldsk:
	ld hl,dpbase
	ret

; home disk
home:	ld      a,(hstwrt)      ;check for pending write
	or      a
	jr      nz,homed
	ld      (hstact),a      ;clear host active flag				
homed:
	ld	bc,0
	; fall into settrk

; set track number
settrk:	ld	(sektrk),bc
	ret

; set sector number
setsec:	ld	a,c		; set sector
	ld	(seksec),a
	ret

; translate sector
sectrn:
	ld h,b
	ld l,c
	ret

; set dma address
setdma:	ld	(dmaadr),bc
	ret

;------------------------------------------------------------------------------
; buffered disk read and write
;------------------------------------------------------------------------------

; buffered read
read:
    xor     a
    ld      (unacnt),a		;unacnt = 0
    inc     a
    ld      (readop),a      ;read operation
    ld      (rsflag),a      ;must read data
    ld      a,wrual
    ld      (wrtype),a      ;treat as unalloc
    jp      rwoper          ;to perform the read

; buffered write
write:
    xor     a               ;0 to accumulator
    ld      (readop),a      ;not a read operation
    ld      a,c             ;write type in c
    ld      (wrtype),a
    and     wrual           ;write unallocated?
    jr      Z,chkuna        ;check for unalloc

;           write to unallocated, set parameters
    ld      a,cpmbls/128    ;next unalloc recs
    ld      (unacnt),a
    ld      a,(sekdsk)      ;disk to seek
    ld      (unadsk),a      ;unadsk = sekdsk
    ld      hl,(sektrk)
    ld      (unatrk),hl     ;unatrk = sectrk
    ld      a,(seksec)
    ld      (unasec),a      ;unasec = seksec

chkuna:
;           check for write to unallocated sector
    ld      a,(unacnt)      ;any unalloc remain?
    or      a
    jr      Z,alloc         ;skip if not

;           more unallocated records remain
    dec     a               ;unacnt = unacnt-1
    ld      (unacnt),a
    ld      a,(sekdsk)      ;same disk?
    ld      hl,unadsk
    cp      (hl)            ;sekdsk = unadsk?
    jr      NZ,alloc        ;skip if not

;           disks are the same
    ld      hl,unatrk
    call    sektrkcmp       ;sektrk = unatrk?
    jr      NZ,alloc        ;skip if not

;           tracks are the same
    ld      a,(seksec)      ;same sector?
    ld      hl,unasec
    cp      (hl)            ;seksec = unasec?
    jr      NZ,alloc        ;skip if not

;           match, move to next sector for future ref
    inc     (hl)            ;unasec = unasec+1
    ld      a,(hl)          ;end of track?
    cp      cpmspt          ;count CP/M sectors
    jr      C,noovf         ;skip if no overflow

;           overflow to next track
    ld      (hl),0          ;unasec = 0
    ld      hl,(unatrk)
    inc     hl
    ld      (unatrk),hl     ;unatrk = unatrk+1

noovf:
;           match found, mark as unnecessary read
    xor     a               ;0 to accumulator
    ld      (rsflag),a      ;rsflag = 0
    jr      rwoper          ;to perform the write

alloc:
;           not an unallocated record, requires pre-read
    xor     a               ;0 to accum
    ld      (unacnt),a      ;unacnt = 0
    inc     a               ;1 to accum
    ld      (rsflag),a      ;rsflag = 1

; common read/write routine
rwoper:
;           enter here to perform the read/write
    xor     a               ;zero to accum
    ld      (erflag),a      ;no errors (yet)
    ld      a,(seksec)      ;compute host sector
                            ;assuming 4 CP/M sectors per host sector
    srl     a               ;shift right
    srl     a               ;shift right
    ld      (sekhst),a      ;host sector to seek

;           active host sector?
    ld      hl,hstact       ;host active flag
    ld      a,(hl)
    ld      (hl),1          ;always becomes 1
    or      a               ;was it already?
    jr      Z,filhst        ;fill host if not

;           host buffer active, same as seek buffer?
    ld      a,(sekdsk)
    ld      hl,hstdsk       ;same disk?
    cp      (hl)            ;sekdsk = hstdsk?
    jr      NZ,nomatch

;           same disk, same track?
    ld      hl,hsttrk
    call    sektrkcmp       ;sektrk = hsttrk?
    jr      NZ,nomatch

;           same disk, same track, same buffer?
    ld      a,(sekhst)
    ld      hl,hstsec       ;sekhst = hstsec?
    cp      (hl)
    jr      Z,match         ;skip if match

nomatch:
;           proper disk, but not correct sector
    ld      a,(hstwrt)      ;host written?
    or      a
    call    NZ,writehst     ;clear host buff

filhst:
;           may have to fill the host buffer
    ld      a,(sekdsk)
    ld      (hstdsk),a
    ld      hl,(sektrk)
    ld      (hsttrk),hl
    ld      a,(sekhst)
    ld      (hstsec),a
    ld      a,(rsflag)      ;need to read?
    or      a
    call    NZ,readhst      ;yes, if 1
    xor     a               ;0 to accum
    ld      (hstwrt),a      ;no pending write

match:
;           copy data to or from buffer
    ld      a,(seksec)      ;mask buffer number
    and     secmsk          ;least significant bits, shifted off in sekhst calculation
    ld      h,0             ;double count    
    ld      l,a             ;ready to shift

    xor     a               ;shift left 7, for 128 bytes x seksec LSBs
    srl     h
    rr      l
    rra
    ld      h,l
    ld      l,a

;           HL has relative host buffer address
    ld      de,hstbuf
    add     hl,de           ;HL = host address
    ex      de,hl           ;now in DE
    ld      hl,(dmaadr)     ;get/put CP/M data
    ld      bc,128          ;length of move
    ex      de,hl           ;source in HL, destination in DE
    ld      a,(readop)      ;which way?
    or      a
    jr      NZ,rwmove       ;skip if read

;           write operation, mark and switch direction
    ld      a,1
    ld      (hstwrt),a      ;hstwrt = 1
    ex      de,hl           ;source/dest swap

rwmove:
    ldir

;           data has been moved to/from host buffer
    ld      a,(wrtype)      ;write type
    and     wrdir           ;to directory?
    ld      a,(erflag)      ;in case of errors
    ret     Z               ;no further processing

;        clear host buffer for directory write
    or      a               ;errors?
    ret     NZ              ;skip if so
    xor     a               ;0 to accum
    ld      (hstwrt),a      ;buffer written
    call    writehst
    ld      a,(erflag)
    ret

; 16-bit compare
sektrkcmp:
;           HL = unatrk or hsttrk, compare with sektrk
    ex      de,hl
    ld      hl,sektrk
    ld      a,(de)          ;low byte compare
    cp      (hl)            ;same?
    ret     NZ              ;return if not
;           low bytes equal, test high 1s
    inc     de
    inc     hl
    ld      a,(de)
    cp      (hl)            ;sets flags
    ret


;------------------------------------------------------------------------------
; host dma interface
;------------------------------------------------------------------------------

; read from host disk image
readhst:
	call	seek
	f_call	F_READ
	jp	nz, error
	ret

; write to host disk image
writehst:
	;call	seek
	;f_call	F_WRITE
	;jp	nz, error
	ret

; seek to correct offset in host disk image
seek:
	; calculate 32-bit offset from track and sector
	ld	a,(hstsec)      ; de = sector
	ld	e,a
	xor	a
	ld	d,a
	ld	(f_ofs),a	; clear lsb of offset
	ld	hl,(hsttrk)     ; hl = track
	add	hl,hl		; ahl = track * 2
	adc	a
	add	hl,hl		; ahl = track * 4
	adc	a
	add	hl,hl		; ahl = track * 8
	adc	a
	add	hl,hl		; ahl = track * 16
	adc	a
	add	hl,hl		; ahl = track * 32
	adc	a
	add	hl,de		; ahl = track * 32 + sector
	adc	d
	add	hl,hl		; ahl = (track * 32 + sector) * 2
	adc	a
	ld	(f_ofs+1),hl	; offset = (track * 32 + sector) * 512
	ld 	(f_ofs+3),a

	; seek using DMA interface
	f_call	F_LSEEK		; seek to offset
	jp 	nz,error
	ret

; disk error handler
error:
	halt


;------------------------------------------------------------------------------
; bios data
;------------------------------------------------------------------------------

; DMA file pointers and filenames
d1fn:	defb	"SYS.CPM", 0
d1fp:	defs	fplen
ccpfn:  defb    "CPM.OUT", 0
ccpfp:  defs    fplen

dpbase:
;	disk parameter header for disk 00
	defw	0000h, 0000h
	defw	0000h, 0000h
	defw	dirbf, dpblk
	defw	0000h, alv00
;	disk parameter header for disk 01
	defw	0000h, 0000h
	defw	0000h, 0000h
	defw	dirbf, dpblk
	defw	0000h, alv01
;	disk parameter header for disk 02
	defw	0000h, 0000h
	defw	0000h, 0000h
	defw	dirbf, dpblk
	defw	0000h, alv02
;	disk parameter header for disk 03
	defw	0000h, 0000h
	defw	0000h, 0000h
	defw	dirbf, dpblk
	defw	0000h, alv03
;
;   disk parameter block for all disks.
;
dpblk:
	defw	cpmspt  	; SPT - sectors per track
	defb	5       	; BSH - block shift factor from BLS
	defb	31      	; BLM - block mask from BLS
	defb	1       	; EXM - Extent mask
	defw	blksiz-1	; DSM - Storage size (blocks - 1)
	defw	cpmdir-1	; DRM - Number of directory entries - 1
	defb	0f0h     	; AL0 - 1 bit set per directory block (ALLOC0)
	defb	0     		; AL1 - 1 bit set per directory block (ALLOC0)
	defw	0       	; CKS - DIR check vector size (DRM+1)/4 (0=fixed disk) (ALLOC1)
	defw	0       	; OFF - Reserved tracks offset

sekdsk:	defs	1	; seek disk number
sektrk:	defs	2	; seek track number
seksec:	defs	2	; seek sector number

hstdsk:	defs	1	; host disk number
hsttrk:	defs	2	; host track number
hstsec:	defs	1	; host sector number

sekhst:	defs	1	; seek shr secshf
hstact:	defs	1	; host active flag
hstwrt:	defs	1	; host written flag

unacnt:	defs	1	; unalloc rec cnt
unadsk:	defs	1	; last unalloc disk
unatrk:	defs	2	; last unalloc track
unasec:	defs	1	; last unalloc sector

erflag:	defs	1	; error reporting
rsflag:	defs	1	; read sector flag
readop:	defs	1	; 1 if read operation
wrtype:	defs	1	; write operation type
dmaadr:	defs	2	; last dma address

alv00:	defs	((blksiz-1)/8)+1	; allocation vector 0
alv01:	defs	((blksiz-1)/8)+1	; allocation vector 1
alv02:	defs	((blksiz-1)/8)+1	; allocation vector 2
alv03:	defs	((blksiz-1)/8)+1	; allocation vector 3

dirbf:	defs	128			; scratch directory area