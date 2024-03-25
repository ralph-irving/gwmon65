;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;6850ACIA -- Console I/O Drivers for Motorola 6850 ACIA
;
;Set CTLREG and DATREG in the customization file.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;IOSET -- I/O specific setup routine
;
;pre: none
;post :console serial channel is initialized
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
IOSET:
	JSR	INIT_BUFFER	;Init circular buffer ptrs
	JSR	MLI		;Allocate interrupt
	.BYTE	$40
	.WORD	AIPARMS
	BEQ	@OK
	JMP	PDOSERR		;Bail if error
@OK:
	LDA	#$23		;Reset ACIA
	STA	CCCOM
	LDA	#$15		;8 Bits/1 Stop/No Parity/
				;/No Interrupts/16 baud rate
	STA	CCCOM
	LDX	CCDTA		;Throw away any garbage
	ORA	#$80		;Enable Interrupts
	STA	CCCOM
	RTS

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;IOCLR -- I/O specific clear routine
;
;pre :console serial channel is initialized
;post :interrupts disabled
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
IOCLR:
	LDA	#$23		;Stop Interrupts/DTR Off
	STA	CCCOM
	CLI			;Just for safety
	LDA	AIPARMS+1	;Get Interrupt number
	STA	DIPARMS+1	; for deallocation.
	JSR	MLI
	.BYTE	$41		;Deallocate interupt handler
	.WORD	DIPARMS
	RTS

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;CINNE -- Get a character from the console, no echo
;
;Blocks until a character is available. Warm starts the
;monitor if the received character is the CANCEL char.
;
;pre: serial UART initialized
;post: A register contains received character
;post: monitor warm started if CANCEL received
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
CINNE:	phx
@no_keypressed:
	jsr	BUFFER_SIZE
	beq	@no_keypressed

;	LDA	CTLREG		;See if we got an incoming char
;	LSR			;Test bit 0
;	BCC	CINNE		;Wait for character

	jsr	READ_BUFFER

;	LDA	DATREG		;Load char
	CMP	#CANCEL		;CANCEL ASCII code?
	BNE	CINNE2		;No, return
	JMP	WSTART		;Yes, warm start

CINNE2:	plx
	rts

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;CIN -- Get a char from the console and echo
;
;Falls through to COUT.
;
;pre: console device is initialized
;post: received char is in A register
;post: received char is echoed
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
CIN:	JSR	CINNE

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;COUT -- Print a character to the console
;
;Strips parity, preserves A register.
;
;pre: A register contains character to print
;post: character printed to console
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
COUT:	PHA			;Save A register
COUT1:	LDA	CTLREG		;Get UART status
	AND	#$02		;Mask off XMTR Data Reg Empty
	BEQ	COUT1		;Transmitter busy, wait
	PLA			;A = character to print
	PHA			;Back on stack
	AND	#$7F		;Strip high bit
	STA	DATREG		;Send it to UART
	PLA			;A = original character to send
COUT2:	RTS


; Initialize the circular input buffer
; Modifies: flags, A
INIT_BUFFER:
	lda READ_PTR
	sta WRITE_PTR
	rts

; Write a character (from the A register) to the circular input buffer
; Modifies: flags, X
WRITE_BUFFER:
	ldx WRITE_PTR
	sta INPUT_BUFFER,x
	inc WRITE_PTR
	rts

; Read a character from the circular input buffer and put it in the A register
; Modifies: flags, A, X
READ_BUFFER:
	sei
	ldx READ_PTR
	lda INPUT_BUFFER,x
	inc READ_PTR
	cli
	rts

; Return (in A) the number of unread bytes in the circular input buffer
; Modifies: flags, A
BUFFER_SIZE:
	sei
	lda WRITE_PTR
	sec
	sbc READ_PTR
	cli
	rts

; Interrupt request handler
; http://forum.6502.org/viewtopic.php?f=2&t=7579
IRQ_HANDLER:
	CLD			; ProDOS requirement to indicate a valid external routine
	LDA	CCSTS		; Check status
	BMI	COMME		; Interrupt was from CCS
	SEC			; Indicate not my interrupt
	RTS
COMME:
	PHA
	AND	#$70		; Any errors occured?
	BEQ	COMNE		; No
;	INC	ERRORS		; Yes, bump error count
COMNE:
	PLA
	AND	#$01		; Check for incoming data
	BEQ	CLAIM		; Nothing, ignore other interrupts
	LDA	CCDTA		; Get incoming byte
	JSR	WRITE_BUFFER
CLAIM:
	CLC
	RTS			; ProDOS chains irq handlers. No rti here.
