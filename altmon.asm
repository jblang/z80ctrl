;-------------------------------------------------------------------------
;  ALTMON.ASM - 1K ROM monitor for the Altair 8800.
; 
;     This monitor is based on the 2.0C monitor from Vector Graphic. The
;     original version has been updated to use Altair 2SIO serial ports
;     for I/O and several commands have been added and/or modified. 
;
;     A typical location for this PROM in an Altair is at F800, though
;     it can be assembled at most any address. The stack is typically
;     placed at the top of the minimum RAM you expect to have in your
;     system.
;
;  Version     Date	Author
;  -------  ----------	---------------------------------------
;    1.0    01/10/2016  Mike Douglas  (Original)
;
;    1.1    02/29/2016  Mike Douglas
;		Fix bug in DUMP code that caused improper range of bytes
;		to display. Also in DUMP, display '.' for all characters
;		7Fh or above.
;
;		Initialize 2nd 2SIO port so that loading of Intel HEX
;		files works over the 2nd port. Only flush hex file
;		input on the console serial port to free up code space
;		and because it's not really required for the 2nd port.
;
;-------------------------------------------------------------------------
;
;   Following is a summary of changes from the original VG 2.0c monitor:
;
;	All commands immediately echo a full command name as soon as the 
;	first command letter is typed (e.g., typing "M" immediately
;	displays "MOVE"). This makes it easier to identify commands 
;	without a list of commands present.
;
;	The ESC key can be pressed to abort input or commands as in
;	the later 4.x monitors from VG. The original ctrl-c abort is
;	still present as well.
;
;	The B (boot) command jumps to the Altair disk boot loader PROM
;	at FF00 instead of the North Star boot ROM.
;
;	A (ASCII dump) command removed and D (hex dump) updated to display
;	both hex and ASCII.
;
;	X (exchange) command changed to the E command.
;
;	H command added to load Intel hex file via either serial port
;	on a 2SIO. The L (load and go from tape) does a hex file load
;	as well (all tape commands eliminated).
;
;	J treated as jump (i.e., go to) command instead of jump to North
;	Star DOS.
;
;	K treated as fill memory with "K"onstant instead of jump to zero
;	(was the Z command which has been removed).
;
;	R command sizes RAM (i.e., runs the N non-destructive memory test)
;	Was previously a read from cassette command. All cassette commands
;	have been removed.
;
;	The Y command (Vector Graphic relocating loader) command has been
;	removed.
;
;	The T test memory command skips the 256 byte page the stack is on
;	to prevent crashing the program. A "." pacifier is displayed 
;	after each cycle through the memory test range is completed
;
;-------------------------------------------------------------------------
;
;  Command Summary:
;
;	B jump to Altair disk boot loader (FF00)
;	C SSSS FFFF CCCC compare blocks
;	D SSSS FFFF dump in hex and ASCII
;	E SSSS FFFF DDDD exchange block
;	F SSSS FFFF DD DD find two byte sequence
;	G SSSS go to and execute
;	H P load Intel hex file from 2SIO port 0 or 1
;	I PP input from I/O port
;	J SSSS go to and execute (G)
;	K SSSS FFFF DD fill with "K"onstant
;	L P load Intel hex file from 2SIO port 0 or 1
;	M SSSS FFFF DDDD move block
;	N non destructive memory test (size RAM)
;	O PP DD output to port
;	P LLLL program memory
;	Q SSSS FFFF compute checksum
;	R non destructive memory test (size RAM)
;	S SSSS FFFF DD search for single byte sequence
;	T SSSS FFFF test memory
;
;-------------------------------------------------------------------------
;
; Memory location equates

	org	0f800h		;ROM location

SPTR	equ	0c000h		;stack pointer (use 256 byte boundary)
SIOPORT	equ	SPTR-32		;2SIO port used for hex load
BOOT	equ	0ff00h		;Altair disk boot loader ROM

; 88-2SIO equates

CONS	equ	10h		;console status port
COND	equ	11h		;console data port
TBE	equ	2		;transmit buffer entry
RDA	equ	1		;receive data available

; Misc Equates

CR	equ	13		;ASCII carriage return
LF	equ	10		;ASCII line feed
CTRLC	equ	3		;ASCII control-c
ESC	equ	27		;ASCII ESCAPE

;---------------------------------------------------------
;  monit - monitor entry point
;---------------------------------------------------------
monit	mvi	a,3		;reset 6850 uart
	out	CONS
	out	CONS+2		;2nd 2SIO port as well
	mvi	a,11h		;8N2
	out	CONS
	out	CONS+2		;2nd 2SIO port as well

	lxi	sp,SPTR
	call	dspMsg		;display welcome banner
	db	CR,LF,LF,'ALTMON 1.','1'+80h

; start - command processing loop

start	lxi	sp,SPTR		;re-init stack pointer
	lxi	h,start		;RET's go back to start
	push	h

	call	crlf		;display '*' prompt after CR/LF
	mvi	a,'*'
	call	ptcn

	call	getCon		;read command from keyboard
	ani	05FH		;lower case to upper case
	cpi	'B'
	rc			;too small
	cpi	'U'
	rnc			;too large

	lxi	h,cmdTbl+100h-2*'B'	;'B' indexes to start of cmdtbl
	add	a		;2 bytes per entry
	add	l
	mov	l,a

	mov	e,m		;e=lsb of jump address
	inx	h
	mov	d,m		;d=high byte of jump address
	xchg
	pchl			;away we go

; Command Table

cmdTbl	dw	doboot		;B jump to Altair disk boot loader
	dw	compr		;C SSSS FFFF CCCC compare blocks
	dw	disp		;D SSSS FFFF dump in hex
	dw	exchg		;E SSSS FFFF DDDD exchange block
	dw	srch2		;F SSSS FFFF DD DD two byte search
	dw	exec		;G SSSS go to and execute
	dw	hexLoad		;H P load Intel hex file from port
	dw	pinpt		;I PP input from I/O port
	dw	exec		;J SSSS jump to and execute (G)
	dw	fill		;K SSSS FFFF DD fill RAM with "k"onstant
	dw	hexLoad		;L P load Intel hex file from port
	dw	moveb		;M SSSS FFFF DDDD move block
	dw	ndmt		;N non destructive memory test (RAM size)
	dw	poutp		;O PP DD output to port
	dw	pgm		;P LLLL program memory
	dw	chksum		;Q SSSS FFFF compute checksum
	dw	ndmt		;R non destructive memory test (RAM size)
	dw	srch1		;S SSSS FFFF DD search for single byte
	dw	tmem		;T SSSS FFFF test memory

;--------------------------------------------------------------------------
; exec (G or J) - execute the program at the address
;--------------------------------------------------------------------------
exec	call	dspMsg
	db	'GOT','O'+80h

	call	ahex		;read address from keyboard
	xchg
	pchl

;--------------------------------------------------------------------------
; doBoot (B) - boot floppy disk by jumping to DBL PROM at FF00
;--------------------------------------------------------------------------
doBoot	call	dspMsg
	db	'BOO','T'+80h

	jmp	BOOT

;--------------------------------------------------------------------------
; chksum (Q) - compute checksum
;--------------------------------------------------------------------------
chksum	call	dspMsg
	db	'CSU','M'+80h

	call	tahex
	mvi	b,0		;start checksum = 0

csloop	mov	a,m		;get data from memory
	add	b		;add to checksum
	mov	b,a
	call	bmp
	jnz	csloop		;repeat loop

	mov	a,b		;a=checksum
	jmp	pt2		;print checksum and exit

;--------------------------------------------------------------------------
; tmem (T) - memory test routine
;--------------------------------------------------------------------------
tmem	call	dspMsg
	db	'TES','T'+80h

	call	tahex		;read addresses
	lxi	b,05a5ah	;init b,c

cycl	mvi	a,'.'		;display '.' before each cycle
	call	ptcn
	call	rndm
	push	b		;keep all registers
	push	h
	push	d

tlop	mov	a,h		;on stack page?
	cpi	(SPTR shr 8)-1	;compare to msb of stack
	jz	skipWr		;in stack, skip write
	call	rndm
	mov	m,b		;write in memory
skipWr	call	bmp
	jnz	tlop		;repeat loop

	pop	d		
	pop	h		;restore original
	pop	b		;values
	push	h
	push	d

rlop	mov	a,h		;on stack page?
	cpi	(SPTR shr 8)-1	;compare to msb of stack
	jz	skipRd		;in stack, skip the read
	call	rndm		;generate new sequence
	mov	a,m		;read memory
	cmp	b		;compare memory
	cnz	err		;call error routine
skipRd	call	bmp
	jnz	rlop

	pop	d
	pop	h
	call	pause
	jmp	cycl

; rndm - this routine generates random numbers

rndm	mov	a,b		;look at b
	ani	0b4h		;mask bits
	ana	a		;clear carry
	jpe	peve		;jump if even
	stc
peve	mov	a,c		;look at c
	ral			;rotate carry in
	mov	c,a		;restore c
	mov	a,b		;look at b
	ral			;rotate carry in
	mov	b,a		;restore b
	ret			;return with new b,c

;--------------------------------------------------------------------------
; disp (D) - display memory contents
;--------------------------------------------------------------------------
disp	call	dspMsg
	db	'DUM','P'+80h

	call	tahex		;read addresses

dmpLine	push	h		;save address at start of line
	mvi	c,16		;16 locations per line
	call	ptad		;print current address

; dump line in hex

dmpHex	mov	a,m		;a=byte to display
	call	pt2		;display it
	call	spce
	inx	h	
	dcr	c		;decrement line byte count
	jnz	dmpHex		;loop until 16 bytes done

; dump line in ASCII

	call	spce
	pop	h		;hl->start of line
	mvi	c,16		;16 locations per line

dmpAsc	mov	a,m		;a=byte to display
	cpi	7Fh		;test if >= 7Fh
	jnc	dspDot		;non printable, show '.'

	cpi	' '		;displayable character?
	jnc	dspAsc		;yes, go display it

dspDot	mvi	a,'.'		;display '.' instead

dspAsc	call	ptcn		;display the character
	call	bmp		;increment hl, possibly de
	dcr	c		;decrement line byte count
	jnz	dmpAsc		;loop until 16 bytes done

	call	bmp		;done?
	rz			;yes
	dcx	h		;undo extra bump of hl
	jmp	dmpLine		;do another line	

;--------------------------------------------------------------------------
; pgm (P) - program memory
;--------------------------------------------------------------------------
pgm	call	dspMsg
	db	'PG','M'+80h

	call	ahex		;read address
	xchg
	call	crlf
		
pglp	mov	a,m		;read memory
	call	pt2		;print 2 digits
	mvi	a,'-'		;load dash
	call	ptcn		;print dash

crig	call	rdcn		;get user input
	cpi	' '		;space
	jz	con2		;skip if space
	cpi	CR		;skip if CR
	jnz	con1
	call	crlf		;print CR,LF
	jmp	crig		;back for more

con1	xchg			;HL->DE
	lxi	h,0		;get 16 bit zero
	mvi	c,2		;count 2 digits
	call	ahexNr		;convert to hex (no read)
	mov	m,e
con2	inx	h
	jmp	pglp

;--------------------------------------------------------------------------
; fill (K) - fill memory with a constant
;--------------------------------------------------------------------------
fill	call	dspMsg
	db	'FIL','L'+80h

	call	tahex		;read addresses
	push	h		;start addr on stack
	mvi	c,2		;reading 2 digits
	call	ahe0		;input fill byte
	xchg			;byte to write from e to l
	xthl			;hl=start addr, stack=fill byte
	pop	b		;c=fill byte from stack
		
zloop	mov	m,c		;write into memory
	call	bmp		;compare address, increment h
	rz
	jmp	zloop

;--------------------------------------------------------------------------
; moveb (M) - move a block of memory
; exchg (E) - exhange block of memory
;--------------------------------------------------------------------------
moveb	call	dspMsg
	db	'MOV','E'+80h
	xra	a		;a=0 means "move" command
	jmp	doMove

exchg	call	dspMsg
	db	'EXC','H'+80h
				;a returned <> 0 means "exchange" command
		
doMove	mov	b,a		;save move/exchange flag in b
	call	tahex		;read addresses
	push	h
	call	ahex
	xchg
	xthl			;HL->start, DE->end, stack has dest

mloop	mov	c,m		;c=byte from source
	xthl			;hl->destination

	mov	a,b		;move or exchange?
	ora	a
	jz	nexch		;0 means move only

	mov	a,m		;a=from destination
	xthl			;hl->source
	mov	m,a		;move destination to source
	xthl			;hl->destination

nexch	mov	m,c		;move source to destination
	inx	h		;increment destination
	xthl			;hl->source
	call	bmp		;increment source and compare to end
	jnz	mloop

	pop	h		;remove temp pointer from stack
	ret			;and exit

;--------------------------------------------------------------------------
; ndmt (N or R) - non destructive memory test (size RAM)
;--------------------------------------------------------------------------
ndmt	call	dspMsg
	db	'RAMTO','P'+80h

	lxi	h,0ffffh	;start at zero

ndlop	inx	h
	mov	a,m		;read from address in hl
	mov	b,a		;save original value in b
	cma			;form and write inverted value
	mov	m,a
	cmp	m		;read and compare
	mov	m,b		;restore original value
	jz	ndlop		;keep going if still RAM

	jmp	err		;display end of RAM

;--------------------------------------------------------------------------
; compr (C) - compare two blocks of memory
;--------------------------------------------------------------------------
compr	call	dspMsg
	db	'COM','P'+80h

	call	tahex		;read addresses
	push	h		;source start on stack
	call	ahex
	xchg			;de=source end, hl=compare start

vmlop	mov	a,m		;a=compare byte
	inx	h
	xthl			;hl->source byte
	cmp	m		;same?
	mov	b,m		;b=source byte
	cnz	err		;display the error
	call	bmp		;increment pointers
	xthl			;hl->compare byte
	jnz	vmlop

	pop	h		;remove temp pointer from stack
	ret			;and exit

;--------------------------------------------------------------------------
; srch1 (S) - search for one byte
; srch2 (F) - search for two bytes
;--------------------------------------------------------------------------
srch1	call	dspMsg
	db	'FIND','1'+80h
	xra	a		;zero flag means one byte search
	jmp	doSrch

srch2	call	dspMsg
	db	'FIND','2'+80h
				;a returned <> 0 means two byte search

doSrch	push	psw		;save 1/2 byte flag on stack
	call	tahex

	push	h		;save h, getting 1st byte to find
	mvi	c,2		;reading 2 hex digits
	call	ahe0		;
	xchg			;h=code, d=f
	mov	b,l		;put code in b
	pop	h		;restore h

	pop	psw		;a=one/two byte flag
	ora	a		;zero true if one byte search
	push	psw
	jz	cont

	push	h		;save h, getting 2nd byte to find
	mvi	c,2
	call	ahe0
	xchg
	mov	c,l
	pop	h

cont	mov	a,m		;read memory
	cmp	b		;compare to code
	jnz	skp		;skip if no compare

	pop	psw		;a=one/two byte flag
	ora	a		;zero true if one byte serach
	push	psw
	jz	obcp

	inx	h		;two byte search
	mov	a,m
	dcx	h
	cmp	c
	jnz	skp

obcp	inx	h
	mov	a,m		;read next byte
	dcx	h		;decr address
	call	err		;print data found

skp	call	bmp		;check if done
	jnz	cont		;back for more
	pop	psw		;remove flag saved on stack
	ret

;--------------------------------------------------------------------------
; poutp (O) - output data to a port
;--------------------------------------------------------------------------
poutp	call	dspMsg
	db	'OU','T'+80h

	mvi	c,2
	call	ahe0		;port number in e

	mvi	c,2
	call	ahe0		;port to l, data in e

	mov	d,l		;d=port
	lxi	h,SPTR-30h	;form OUT nn, RET in memory at h
	mvi	m,0c9h		;RET opcode
	dcx	h
	mov	m,d		;output port for OUT instruction
	dcx	h
	mvi	m,0D3H		;OUT opcode
	mov	a,e
	pchl			;call OUT, RET

;--------------------------------------------------------------------------
; pinpt (I) - input data from a port
;--------------------------------------------------------------------------
pinpt	call	dspMsg
	db	'I','N'+80h

	mvi	c,2
	call	ahe0		;port number to e

	lxi	h,SPTR-30H	;form IN nn, RET in memory at h
	mvi	m,0C9H		;RET opcode
	dcx	h
	mov	m,e		;input port of IN instruction
	dcx	h
	mvi	m,0DBH		;IN opcode
	call	SPTR-32H
	jmp	pt2

;---------------------------------------------------------------------
; hexLoad (H or L) - load intel hex through 2SIO serial port 0 or 1
;---------------------------------------------------------------------
hexload	call	dspMsg
	db	'HEXLOA','D'+80h

	mvi	c,1		;read one hex digit
	call	ahe0		;digit is in e
	lxi	h,SIOPORT	;hl->location on stack to save port
	mov	m,e		;SIOPORT = 0 or 1

; rcvLine - receive a hex file line

rcvLine	call	crlf
	mvi	c,0		;clear echo character flag

wtMark	call	getChar		;read next character
	sui	':'		;record marker?
	jnz	wtMark		;no, keep looking

; Have start of new record. Save the byte count and load address.
;   The load address is echoed to the screen so the user can
;   see the file load progress.

	mov	d,a		;init checksum in D to zero

	call	iByte		;input two hex digits (byte count)
	mov	a,e		;test for zero byte count
	ora	a
	jz	flush		;count of 0 means end

	mov	b,e		;B = byte count on line

	inr	c		;set echo flag for address bytes
	call	iByte		;get MSB of address
	mov	h,e		;H = address MSB
	call	iByte		;get LSB of address
	mov	l,e		;L = address LSB
	dcr	c		;clear echo flag

	call	iByte		;ignore/discard record type

; Receive the data bytes of the record and move to memory

data	call	iByte		;read a data byte (2 hex digits)
	mov	m,e		;store in memory
	inx	h
	dcr	b
	jnz	data

; Validate checksum

	call	iByte		;read and add checksum
	jz	rcvLine		;checksum good, receive next line

	call	dspMsg		;display error message
	db	' ER','R'+80h
				;fall into flush

; flush - flush rest of file as it comes in until no characters
;    received for about 1/4 second to prevent incoming file
;    data looking like typed monitor commands. Only the console
;    port needs to be flushed. 

flush	in	COND		;clear possible received char
	lxi	d,10417		;.25s timeout for 48 cycle loop

flshLp	in	CONS		;(10) look for character on console
	rrc			;(4) data flag in carry
	jc	flush		;(10) data received, restart

	dcx	d		;(5) decrement timeout
	mov	a,d		;(5)
	ora	e		;(4)
	jnz	flshLp		;(10) loop until zero
	ret			;done

;-----------------------------------------------------------
; iByte	- read two ascii hex bytes and return binary
;    value in e. 
;-----------------------------------------------------------
iByte	call	getChar		;get a character
	call	asc2Bin		;ascii hex digit to binary
	add	a		;put in msn, zero lsn
	add	a
	add	a
	add	a
	mov	e,a		;save byte with MSN in E

; 2nd byte (LSN)

	call	getChar		;get a character
	call	asc2Bin		;ascii hex digit to binary
	add	e		;combine msn and lsn
	mov	e,a		;save in EH
	add	d		;add character to checksum
	mov	d,a
	ret		

;-------------------------------------------------------------
; asc2Bin - ASCII hex digit to binary conversion. Digit
;    passed in a, returned in a. Errors ignored as checksum
;    will eventually kick this out.
;-------------------------------------------------------------
asc2Bin	sui	'0'		;'0' to 0
	cpi	10		;0-9 ?
	rc

	sui	7		;'A-F' to A-F
	ret

;-------------------------------------------------------------
; getChar - read a character from the 2SIO port specified in
;    SIOPORT. The character is also echoed to the console port
;    if the echo flag (c) is set (non-zero)
;-------------------------------------------------------------
getChar	push	b		;save b,c
	lda	SIOPORT		;a=pseudo port to use
	ora	a		;port zero?
	jnz	inWait1		;no, use port 1

; in through 1st port (0) on 2SIO

inWait0	call	cntlc		;test for character from console
	jz	inWait0
	jmp	haveChr

; in through 2nd port (1) on 2SIO, check for ctrl-c on console
;    while waiting

inWait1	call	cntlc		;look for ctrl-c on console
	in	CONS+2		;wait for character on 2nd 2SIO
	rrc			;data flag in carry
	jnc	inWait1
	in	COND+2		;a=character read

; process new character in a. Echo to console if c is non-zero

haveChr	mov	b,a		;save character in b
	mov	a,c		;echo flag (c) set?
	ora	a
	jz	noEcho		;no echo

	mov	a,b		;a=character to send
	pop	b		;restore b,c
	jmp	ptcn		;display character and exit

noEcho	mov	a,b		;a=byte read
	pop	b		;restore b,c
	ret

;********************************************************************
;
;  Type conversion, input, output subroutines
;
;********************************************************************

;------------------------------------------------------------
; tahex - read two 16 bit addresses. 1st returned in HL, 2nd in DE
;------------------------------------------------------------
tahex	call	ahex		;get first address param
				;fall into ahex to get 2nd param

;------------------------------------------------------------
; ahex - read up to 4 hex digits to binary, return in de
;------------------------------------------------------------
ahex	mvi	c,4		;count of 4 digits
ahe0	lxi	h,0		;16 bit zero
ahe1	call	rdcn		;read a byte
ahexNr	cpi	'0'
	jc	start		;below '0', abort
	cpi	':'
	cnc	alph
	dad	h
	dad	h
	dad	h
	dad	h
	sui	'0'		;ascii bias
	cpi	10		;digit 0-10
	jc	alf
	sui	7		;alpha bias
alf	add	l
	mov	l,a
	dcr	c
	jnz	ahe1		;keep reading
	xchg			;result in de
				;fall through to print a space
;------------------------------------------------------------
; spce - print a space
; ptcn - print character passed in a
;------------------------------------------------------------
spce	mvi	a,' '		;print space
ptcn	push	psw

ptlop	in	CONS		;wait for OK to transmit
	ani	TBE
	jz	ptlop

	pop	psw		;recover a
	ani	07fh		;get rid of msbit
	out	COND		;and print it
	ret			;return from ptcn

;------------------------------------------------------------
; alph - verify valid hex digit, abort to command loop if not
;------------------------------------------------------------
alph	cpi	'A'
	jc	start
	ani	05fh
	cpi	'G'
	jnc	start
	ret

;------------------------------------------------------------
; crlf - print CR/LF
;------------------------------------------------------------
crlf	mvi	a,CR
	call	ptcn
	mvi	a,LF	
	jmp	ptcn

;------------------------------------------------------------
; err - display the address in hl followed by the value
;    in b, then the value in a.
;------------------------------------------------------------
err	push	psw		;save A
	call	ptad		;print address
	mov	a,b		;print B
	call	pt2
	call	spce
	pop	psw		;print A
pt2	push	psw
	call	binh
	pop	psw
	jmp	binl

;------------------------------------------------------------
; ptad - display the address in h
;------------------------------------------------------------
ptad	call	crlf		;print cr,lf
	call	pause
	mov	a,h		;print
	call	pt2		;ascii
	mov	a,l		;codes
	call	pt2		;for
	call	spce		;address
	ret

;------------------------------------------------------------
; binh - print MSN of byte passed in A
; binl - print LSN of byte passed in A
;------------------------------------------------------------
binh	rar
	rar
	rar
	rar
binl	ani	0fh		;low 4 bits
	adi	'0'		;ascii bias
	cpi	03ah		;digit 0-9
	jc	ptcn
	adi	7		;digit A-F
	jmp	ptcn

;------------------------------------------------------------
; dspMsg - display in-line message. String terminated by byte
;      with msbit set.
;------------------------------------------------------------
dspMsg	pop	h		;hl->string to display

dspLoop	mov	a,m		;a=next character to display
	call	ptcn		;display character
	ora	m		;MSB set? (last byte)
	inx	h		;point to next character
	jp	dspLoop		;no, keep looping

	call	spce		;display a trailing space
	pchl			;return past the string

;------------------------------------------------------------
; rdcn - read from console to A with echo to screen
; getCon - read from console to A without echo
;------------------------------------------------------------
rdcn	call	getCon		;get character from console
	cpi	ESC		;ESC confuses smart terminals
	rz			;    so don't echo escape
	jmp	ptcn		;echo onto printer

getCon	in	CONS		;read keyboard status
	rrc			;data available flag in carry
	jnc	getCon

	in	COND		;read from keyboard
	ani	07fh		;strip off msb
	ret

;------------------------------------------------------------
; pause - pause/resume with spacebar. Also look for a ctrl-c
;    or ESC to abort.
;------------------------------------------------------------
pause	call	cntlc		;look for abort or other character
	cpi	' '
	rnz			;return if not space or abort

ploop	call	cntlc		;loop here until space or abort pressed
	cpi	' '
	jnz	ploop
	ret

;------------------------------------------------------------
; cntlc - see if a character has been typed. If not, return
;   zero true. If ctrl-c or ESC typed, abort and return to 
;   the command loop. Otherwise, return the character typed.
;------------------------------------------------------------
cntlc	in	CONS		;anything typed?
	ani	RDA
	rz			;no, exit with zero true

	in	COND		;get the typed character
	ani	07fh
	cpi	CTRLC		;abort with ctrl-c (2.0 style)
	jz	start
	cpi	ESC		;or ESC (4.x style)
	jz	start
	ret

;------------------------------------------------------------
; bmp - compare address and increment h. Return zero true
;   if hl=de. Once hl=de, then de is incremented each time
;   so the comparison remains true for subsequent calls.
;------------------------------------------------------------
bmp	mov	a,e		;compare lsb's of hl,de
	sub	l
	jnz	goon		;not equal

	mov	a,d		;compare msb's of hl,de
	sbb	h		;gives zero true if equal

goon	inx	h		;increment hl
	rnz			;exit if hl <> de yet

	inx	d		;increase de as well so it will
	ret			;    still be equal next time

	end
