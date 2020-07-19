; BDOS stub for z80ctrl BDOS emulation
; Assemble using sjasm
; Rename bdos.out to bdos.bin and place in root directory of z80ctrl SD card

;*****************************************************************
;*****************************************************************
;**                                                             **
;**   B a s i c    D i s k   O p e r a t i n g   S y s t e m    **
;**            I n t e r f a c e   M o d u l e                  **
;**                                                             **
;*****************************************************************
;*****************************************************************

;	Copyright (c) 1978, 1979, 1980
;	Digital Research
;	Box 579, Pacific Grove
;	California


;      20 january 1980

base	equ	0f800h		;base address

	org	base-5		;base load address
	defw	jump
jump:
	jp	cboot		;jump to cold boot


ssize	equ	24		;24 level stack

;	low memory locations
reboot	equ	0000h		;reboot system
ioloc	equ	0003h		;i/o byte location
bdosa	equ	0006h		;address field of jp BDOS

;	bios access constants
bootf	equ	bios+3*0	;cold boot function
wbootf	equ	bios+3*1	;warm boot function
constf	equ	bios+3*2	;console status function
coninf	equ	bios+3*3	;console input function
conoutf	equ	bios+3*4	;console output function
listf	equ	bios+3*5	;list output function
punchf	equ	bios+3*6	;punch output function
readerf	equ	bios+3*7	;reader input function
homef	equ	bios+3*8	;disk home function
seldskf	equ	bios+3*9	;select disk function
settrkf	equ	bios+3*10	;set track function
setsecf	equ	bios+3*11	;set sector function
setdmaf	equ	bios+3*12	;set dma function
readf	equ	bios+3*13	;read disk function
writef	equ	bios+3*14	;write disk function
liststf	equ	bios+3*15	;list status function
sectran	equ	bios+3*16	;sector translate

;	equates for non graphic characters
ctlc	equ	03h		;control c
ctle	equ	05h		;physical eol
ctlh	equ	08h		;backspace
ctlp	equ	10h		;prnt toggle
ctlr	equ	12h		;repeat line
ctls	equ	13h		;stop/start screen
ctlu	equ	15h		;line delete
ctlx	equ	18h		;=ctl-u
ctlz	equ	1ah		;end of file
rubout	equ	7fh		;char delete
tab	equ	09h		;tab char
cr	equ	0dh		;carriage return
lf	equ	0ah		;line feed
ctl	equ	5eh		;up arrow

	db	0,0,0,0,0,0

;	enter here from the user's program with function number in c,
;	and information address in d,e
bdos:	jp	bdose		;past parameter block

;	************************************************
;	*** relative locations 0009 - 000e           ***
;	************************************************
pererr:	dw	persub		;permanent error subroutine
selerr:	dw	selsub		;select error subroutine
roderr:	dw	rodsub		;ro disk error subroutine
roferr:	dw	rofsub		;ro file error subroutine


bdose:	ex	de,hl		;arrive here from user programs
	ld	(info),hl
	ex	de,hl		;info=DE, DE=info
	ld	a,e
	ld	(linfo),a	;linfo = low(info) - don't equ
	ld	hl,0
	ld	(aret),hl	;return value defaults to 0000
				;save user's stack pointer, set to local stack
	add	hl,sp
	ld	(entsp),hl	;entsp = stackptr
	ld	sp,lstack	;local stack setup
	xor	a
	ld	(fcbdsk),a
	ld	(resel),a	;fcbdsk,resel=false
	ld	hl,goback	;return here after all functions
	push	hl		;jmp goback equivalent to ret
	ld	a,c
	cp	nfuncs
	ret	nc		;skip if invalid #
	ld	c,e		;possible output character to C
	ld	hl,functab
	ld	e,a
	ld	d,0		;DE=func, HL=.ciotab
	add	hl,de
	add	hl,de
	ld	e,(hl)
	inc	hl
	ld	d,(hl)		;DE=functab(func)
	ld	hl,(info)	;info in DE for later xchg
	ex	de,hl
	jp	(hl)		;dispatched

;	dispatch table for functions
functab:
	dw	wbootf, func1, func2, func3
	dw	punchf, listf, func6, func7
	dw	func8, func9, func10,func11
diskf	equ	($-functab)/2	;disk funcs
	dw	func12,func13,func14,func15
	dw	func16,func17,func18,func19
	dw	func20,func21,func22,func23
	dw	func24,func25,func26,func27
	dw	func28,func29,func30,func31
	dw	func32,func33,func34,func35
	dw	func36,func37,func38,func39
	dw	func40
nfuncs	equ	($-functab)/2


;	error subroutines
persub:	ld	hl,permsg	;report permanent error
	call	errflg		;to report the error
	cp	ctlc
	jp	z,reboot	;reboot if response is ctlc
	ret			;and ignore the error

selsub:	ld	hl,selmsg	;report select error
	jp	wait_err	;wait console before boot

rodsub:	ld	hl,rodmsg	;report write to read/only disk
	jp	wait_err	;wait console

rofsub:				;report read/only file
	ld	hl,rofmsg	;drop through to wait for console

wait_err:			;wait for response before boot
	call	errflg
	jp	reboot

;	error messages
dskmsg:	db	'Bdos Err On '
dskerr:	db	' : $'		;filled in by errflg
permsg:	db	'Bad Sector$'
selmsg:	db	'Select$'
rofmsg:	db	'File '
rodmsg:	db	'R/O$'


errflg:	push	hl		;report error to console, message address in HL
	call	crlf		;stack mssg address, new line
	ld	a,(curdsk)
	add	a,'A'
	ld	(dskerr),a	;current disk name
	ld	bc,dskmsg
	call	print		;the error message
	pop	bc
	call	print		;error mssage tail
;	jp	conin		;to get the input character
				;(drop through to conin)
;	ret


;	console handlers
conin:	ld	hl,kbchar	;read console character to A
	ld	a,(hl)
	ld	(hl),0
	or	a
	ret	nz
				;no previous keyboard character ready
	jp	coninf		;get character externally
;	ret
conech:	call	conin		;read character with echo
	call	echoc
	ret	c		;echo character?
				;character must be echoed before return
	push	af
	ld	c,a
	call	tabout
	pop	af
	ret			;with character in A

echoc:				;echo character if graphic
	cp	cr		;cr, lf, tab, or backspace
	ret	z		;carriage return?
	cp	lf
	ret	z		;line feed?
	cp	tab
	ret	z		;tab?
	cp	ctlh
	ret	z		;backspace?
	cp	' '
	ret			;carry set if not graphic

conbrk:				;check for character ready
	ld	a,(kbchar)
	or	a
	jp	nz,conb1	;skip if active kbchar
				;no active kbchar, check external break
	call	constf
	and	1
	ret	z		;return if no char ready
				;character ready, read it
	call	coninf		;to A
	cp	ctls
	jp	nz,conb0	;check stop screen function
				;found ctls, read next character
	call	coninf		;to A
	cp	ctlc
	jp	z,reboot	;ctlc implies re-boot
				;not a reboot, act as if nothing has happened
	xor	a
	ret			;with zero in accumulator
conb0:
				;character in accum, save it
	ld	(kbchar),a
conb1:
				;return with true set in accumulator
	ld	a,1
	ret

conout:				;compute character position/write console char from C
				;compcol = true if computing column position
	ld	a,(compcol)
	or	a
	jp	nz,compout
				;write the character, then compute the column
				;write console character from C
	push	bc
	call	conbrk		;check for screen stop function
	pop	bc
	push	bc		;recall/save character
	call	conoutf		;externally, to console
	pop	bc
	push	bc		;recall/save character
				;may be copying to the list device
	ld	a,(listcp)
	or	a
	call	nz,listf	;to printer, if so
	pop	bc		;recall the character
compout:
	ld	a,c		;recall the character
				;and compute column position
	ld	hl,column	;A = char, HL = .column
	cp	rubout
	ret	z		;no column change if nulls
	inc	(hl)		;column = column + 1
	cp	' '
	ret	nc		;return if graphic
				;not graphic, reset column position
	dec	(hl)		;column = column - 1
	ld	a,(hl)
	or	a
	ret	z		;return if at zero
				;not at zero, may be backspace or end line
	ld	a,c		;character back to A
	cp	ctlh
	jp	nz,notbacksp
				;backspace character
	dec	(hl)		;column = column - 1
	ret

notbacksp:			;not a backspace character, eol?
	cp	lf
	ret	nz		;return if not
				;end of line, column = 0
	ld	(hl),0		;column = 0
	ret

ctlout:				;send C character with possible preceding up-arrow
	ld	a,c
	call	echoc		;cy if not graphic (or special case)
	jp	nc,tabout	;skip if graphic, tab, cr, lf, or ctlh
				;send preceding up arrow
	push	af
	ld	c,ctl
	call	conout		;up arrow
	pop	af
	or	40h		;becomes graphic letter
	ld	c,a		;ready to print
				;(drop through to tabout)

tabout:				;expand tabs to console
	ld	a,c
	cp	tab
	jp	nz,conout	;direct to conout if not
				;tab encountered, move to next tab position
tab0:	ld	c,' '
	call	conout		;another blank
	ld	a,(column)
	and	111b		;column mod 8 = 0 ?
	jp	nz,tab0		;back for another if not
	ret

backup:				;back-up one screen position
	call	pctlh
	ld	c,' '
	call	conoutf
;	(drop through to pctlh)
pctlh:				;send ctlh to console without affecting column count
	ld	c,ctlh
	jp	conoutf
;	ret
crlfp:				;print #, cr, lf for ctlx, ctlu, ctlr functions
				;then move to strtcol (starting column)
	ld	c,'#'
	call	conout
	call	crlf		;column = 0, move to position strtcol
crlfp0:	ld	a,(column)
	ld	hl,strtcol
	cp	(hl)
	ret	nc		;stop when column reaches strtcol
	ld	c,' '
	call	conout		;print blank
	jp	crlfp0

crlf:	ld	c,cr		;carriage return line feed sequence
	call	conout
	ld	c,lf
	jp	conout
;	ret
print:	ld	a,(bc)		;print message until M(BC) = '$'
	cp	'$'
	ret	z		;stop on $
				;more to print
	inc	bc
	push	bc
	ld	c,a		;char to C
	call	tabout		;another character printed
	pop	bc
	jp	print

read:				;read to info address (max length, current length, buffer)
	ld	a,(column)
	ld	(strtcol),a	;save start for ctl-x, ctl-h
	ld	hl,(info)
	ld	c,(hl)
	inc	hl
	push	hl
	ld	b,0
				;B = current buffer length,
				;C = maximum buffer length,
				;HL= next to fill - 1
readnx:				;read next character, BC, HL active
	push	bc
	push	hl		;blen, cmax, HL saved
readn0:	call	conin		;next char in A
	and	7fh		;mask parity bit
	pop	hl
	pop	bc		;reactivate counters
	cp	cr
	jp	z,readen	;end of line?
	cp	lf
	jp	z,readen	;also end of line
	cp	ctlh
	jp	nz,noth		;backspace?
				;do we have any characters to back over?
	ld	a,b
	or	a
	jp	z,readnx
				;characters remain in buffer, backup one
	dec	b		;remove one character
	ld	a,(column)
	ld	(compcol),a	;col > 0
				;compcol > 0 marks repeat as length compute
	jp	linelen		;uses same code as repeat

noth:				;not a backspace
	cp	rubout
	jp	nz,notrub	;rubout char?
				;rubout encountered, rubout if possible
	ld	a,b
	or	a
	jp	z,readnx	;skip if len=0
				;buffer has characters, resend last char
	ld	a,(hl)
	dec	b
	dec	hl		;A = last char
				;blen=blen-1, next to fill - 1 decremented
	jp	rdech1		;act like this is an echo

notrub:				;not a rubout character, check end line
	cp	ctle
	jp	nz,note		;physical end line?
				;yes, save active counters and force eol
	push	bc
	push	hl
	call	crlf
	xor	a
	ld	(strtcol),a	;start position = 00
	jp	readn0		;for another character

note:				;not end of line, list toggle?
	cp	ctlp
	jp	nz,notp		;skip if not ctlp
				;list toggle - change parity
	push	hl		;save next to fill - 1
	ld	hl,listcp	;HL=.listcp flag
	ld	a,1
	sub	(hl)		;True-listcp
	ld	(hl),a		;listcp = not listcp
	pop	hl
	jp	readnx		;for another char

notp:				;not a ctlp, line delete?
	cp	ctlx
	jp	nz,notx
	pop	hl		;discard start position
				;loop while column > strtcol
backx:	ld	a,(strtcol)
	ld	hl,column
	cp	(hl)
	jp	nc,read		;start again
	dec	(hl)		;column = column - 1
	call	backup		;one position
	jp	backx

notx:				;not a control x, control u?
				;not control-X, control-U?
	cp	ctlu
	jp	nz,notu		;skip if not
				;delete line (ctlu)
	call	crlfp		;physical eol
	pop	hl		;discard starting position
	jp	read		;to start all over

notu:				;not line delete, repeat line?
	cp	ctlr
	jp	nz,notr
linelen:			;repeat line, or compute line len (ctlh)
				;if compcol > 0
	push	bc
	call	crlfp		;save line length
	pop	bc
	pop	hl
	push	hl
	push	bc
				;bcur, cmax active, beginning buff at HL
rep0:	ld	a,b
	or	a
	jp	z,rep1		;count len to 00
	inc	hl
	ld	c,(hl)		;next to print
	dec	b
	push	bc
	push	hl		;count length down
	call	ctlout		;character echoed
	pop	hl
	pop	bc		;recall remaining count
	jp	rep0		;for the next character

rep1:				;end of repeat, recall lengths
				;original BC still remains pushed
	push	hl		;save next to fill
	ld	a,(compcol)
	or	a		;>0 if computing length
	jp	z,readn0	;for another char if so
				;column position computed for ctlh
	ld	hl,column
	sub	(hl)		;diff > 0
	ld	(compcol),a	;count down below
				;move back compcol-column spaces
backsp:				;move back one more space
	call	backup		;one space
	ld	hl,compcol
	dec	(hl)
	jp	nz,backsp
	jp	readn0		;for next character

notr:				;not a ctlr, place into buffer
rdecho:	inc	hl
	ld	(hl),a		;character filled to mem
	inc	b		;blen = blen + 1
rdech1:				;look for a random control character
	push	bc
	push	hl		;active values saved
	ld	c,a		;ready to print
	call	ctlout		;may be up-arrow C
	pop	hl
	pop	bc
	ld	a,(hl)		;recall char
	cp	ctlc		;set flags for reboot test
	ld	a,b		;move length to A
	jp	nz,notc		;skip if not a control c
	cp	1		;control C, must be length 1
	jp	z,reboot	;reboot if blen = 1
				;length not one, so skip reboot
notc:				;not reboot, are we at end of buffer?
	cp	c
	jp	c,readnx	;go for another if not
readen:				;end of read operation, store blen
	pop	hl
	ld	(hl),b		;M(current len) = B
	ld	c,cr
	jp	conout		;return carriage
;	ret
func1:				;return console character with echo
	call	conech
	jp	sta_ret

func2	equ	tabout
				;write console character with tab expansion

func3:				;return reader character
	call	readerf
	jp	sta_ret

;func4:	equated to punchf
				;write punch character

;func5:	equated to listf
				;write list character
				;write to list device

func6:				;direct console i/o - read if 0ffh
	ld	a,c
	inc	a
	jp	z,dirinp	;0ffh => 00h, means input mode
	inc	a
	jp	z,constf	;0feH in C for status
				;direct output function
	jp	conoutf

dirinp:	call	constf		;status check
	or	a
	jp	z,retmon	;skip, return 00 if not ready
				;character is ready, get it
	call	coninf		;to A
	jp	sta_ret

func7:				;return io byte
	ld	a,(ioloc)
	jp	sta_ret

func8:				;set i/o byte
	ld	hl,ioloc
	ld	(hl),c
	ret			;jmp goback

func9:				;write line until $ encountered
	ex	de,hl		;was lhld info
	ld	c,l
	ld	b,h		;BC=string address
	jp	print		;out to console

func10	equ	read
				;read a buffered console line

func11:				;check console status
	call	conbrk
				;(drop through to sta_ret)
sta_ret:			;store the A register to aret
	ld	(aret),a
func_ret:
	ret			;jmp goback (pop stack for non cp/m functions)

setlret1:			;set lret = 1
	ld	a,1
	jp	sta_ret

goback:				;arrive here at end of processing to return to user
	ld	a,(resel)
	or	a
	jp	z,retmon
				;reselection may have taken place
	ld	hl,(info)
	ld	(hl),0		;fcb(0)=0
	ld	a,(fcbdsk)
	or	a
	jp	z,retmon
				;restore disk number
	ld	(hl),a		;fcb(0)=fcbdsk
	;ld	a,(olddsk)
	ld	(linfo),a
	;call	curselect

;	return from the disk monitor
retmon:	ld	hl,(entsp)
	ld	sp,hl		;user stack restored
	ld	hl,(aret)
	ld	a,l
	ld	b,h		;BA = HL = aret
	ret

;	data areas

lret	equ	aret		;low(aret)
linfo:	ds	1		;low(info)
fcbdsk:	ds	1		;disk named in fcb
resel:	ds	1		;reselection flag

compcol:
	db	0		;true if computing column position
strtcol:
	db	0		;starting column position after read
column:	db	0		;column position
listcp:	db	0		;listing toggle
kbchar:	db	0		;initial key char = 00
entsp:	ds	2		;entry stack pointer
	ds	ssize*2		;stack size
lstack:
;	end of Basic I/O System

;*****************************************************************
; bdos disk functions
;*****************************************************************

dvers	equ	22h		;version 2.2
tbuff	equ	0080h		;default buffer location
bdosp:	equ	0ch		;port for bdos emulation

bdosemu:			;emulate bdos calls on fatfs
	out	(bdosp),a
	ret

func12:				;return version number
	ld	a,dvers
	jp	sta_ret		;lret = dvers (high = 00)

func13:				;reset disk system - initialize to disk 0
	;ld	hl,0
	;ld	(rodsk),hl
	;ld	(dlog),hl
	xor	a
	ld	(curdsk),a	;note that usrcode remains unchanged
	ld	hl,tbuff
	ld	(dmaad),hl	;dmaad = tbuff
	ret

func14:				;select disk info
	ld	a,e
	ld	(curdsk),a
	ret

func15	equ	bdosemu		;open file
func16	equ	bdosemu		;close file
func17	equ	bdosemu		;search for first occurrence of a file
func18	equ	bdosemu		;search for next occurrence of a file name
func19	equ	bdosemu		;delete a file
func20	equ	bdosemu		;read a file
func21	equ	bdosemu		;write a file
func22	equ	bdosemu		;make a file
func23	equ	bdosemu		;rename a file

func24:				;return the login vector
	ld	hl,(dlog)
	jp	sthl_ret

func25:				;return selected disk number
	ld	a,(curdsk)
	jp	sta_ret

func26:				;set the dma address
	ex	de,hl		;was lhld info
	ld	(dmaad),hl	;dmaad = info
	ret

func27:				;return the login vector address
	ld	hl,alvec
	jp	sthl_ret

func28:	ret			;write protect current disk

func29:				;return r/o bit vector
	ld	hl,(rodsk)
	jp	sthl_ret

func30	equ	bdosemu		;set file attributes

func31:				;return address of disk parameter block
	ld	hl,dpblk
sthl_ret:
	ld	(aret),hl
	ret

func32:				;set user code
	ld	a,(linfo)
	cp	0ffh
	jp	nz,setusrcode
				;interrogate user code instead
	ld	a,(usrcode)
	jp	sta_ret		;lret=usrcode

setusrcode:
	and	1fh
	ld	(usrcode),a
	ret

func33	equ	bdosemu		;random disk read operation
func34	equ	bdosemu		;random disk write operation
func35	equ	bdosemu		;return file size (0-65536)
func36	equ	bdosemu		;set random record
func37	equ	func_ret	;reset drive
func38	equ	func_ret	;unimplemented
func39	equ	func_ret	;unimplemented
func40	equ	bdosemu		;random disk write with zero fill of unallocated block

;*****************************************************************
; bios jump table
;*****************************************************************

	align	100h

bios:	jp 	cboot
wboote:	jp	wboot
	jp	const
	jp	bconin
	jp	bconout
	jp	list
	jp	punch
	jp	reader
	jp	noop
	jp	noop
	jp	noop
	jp	noop
	jp	noop
	jp	noop
	jp	noop
	jp	prstat
	jp	noop

;*****************************************************************
; bios functions
;*****************************************************************
; From SIMH BIOS by Peter Schorn

; SIO controller ports
constat	equ	16		; sio port 1 status port
condata	equ	17		; sio port 1 data port
punstat	equ	18		; sio port 2 status port
pundata	equ	19		; sio port 2 data port

; masks for SIO controller (constat, punstat)
mout	equ	02h		; output allowed mask


; console status
const:
	in	a,(constat)	; get console status
	rra			; I bit into carry
	ld	a,0		; prepare no character available
	ret	nc		; I bit clear means no character, done
	dec	a		; character available, result is 0ffh
	ret			; done

; console input
bconin:
	in	a,(constat)	; get console status
	rra			; I bit into carry
	jp	nc,bconin	; jump back if no character available
	in	a,(condata)	; read character
	and	7fh		; clear bit 8
	ret

; console output
bconout:
	in	a,(constat)	; get console status
	and	mout		; mask output bit
	jp	z,bconout	; jump back if not ready for output
	ld	a,c		; prepare character for output
	out	(condata),a	; do it
	ret

; list/punch output
list:				; list aliased to punch
punch:
	in	a,(punstat)	; get punch status
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

; cold boot
cboot:
	ld	sp,0ffffh	; set up stack pointer
	ld	c,bdosp		; set mailbox address
	in	a,(c)
	ld	hl,mailbox
	out	(c),l
	out	(c),h
	ld	a,0c3h		; jump instruction
	ld	(reboot),a	; set up warm boot vector
	ld	hl,wboote
	ld	(reboot+1),hl
	ld	(bdosa-1),a	; set up bdos vector
	ld	hl,bdos
	ld	(bdosa),hl
	xor	a		; set drive to A
	ld	(curdsk),a
	ld	hl,tbuff	; set default DMA address 80h
	ld	(dmaad),hl

	call	100h		; start program

; warm boot
wboot:
	ld	a,0			; close any open files
	out (bdosp),a
	halt			; return to z80ctrl

; unimplemented bios functions
noop:
	ret

;*****************************************************************
; bios data area
;*****************************************************************

; mailbox to transfer BDOS data
mailbox:
info:	defw	0		; fcb information address
aret:	defw	0		; return value
curdsk:	defb	0		; current disk number
dmaad:	defw	tbuff		; initial dma address
usrcode:db	0		;current user number
rodsk:	defw	0		;read only disk vector
dlog:	defw	1		;logged-in disk vector

; disk parameter block
dpblk:
	defw	64  		; SPT - sectors per track
	defb	5       	; BSH - block shift factor from BLS
	defb	01fh      	; BLM - block mask from BLS
	defb	1       	; EXM - Extent mask
	defw	07ffh		; DSM - Storage size (blocks - 1)
	defw	03ffh		; DRM - Number of directory entries - 1
	defb	0ffh     	; AL0 - 1 bit set per directory block (ALLOC0)
	defb	0   		; AL1 - 1 bit set per directory block (ALLOC0)
	defw	0       	; CKS - DIR check vector size (DRM+1)/4 (0=fixed disk) (ALLOC1)
	defw	2       	; OFF - Reserved tracks offset

alvec:	defw	0		;disk allocation vector