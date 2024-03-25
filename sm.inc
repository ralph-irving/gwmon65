;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;SM -- GWMON-65 Small Monitor
;
;This is the small monitor for GWMON-65. It includes the
;small command processor and common monitor routines.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Zero Page Storage
;
;Set the ZPBASE symbol in the customization file to move
;these variables. Defaults to $F0
;
; Free zero page address available on the Apple 2
; $06 - $09, $1E, $EB - $EF, $FA - $FD
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.IFNDEF	ZPBASE
ZPBASE		= $00
.ENDIF

ADRLO		= ZPBASE+$06	;Low address byte from GETADR
ADRHI		= ZPBASE+$07	;High address byte from GETADR
ENDLO		= ZPBASE+$08	;Low address byte for two-address commands
ENDHI		= ZPBASE+$09	;High address byte for two-address commands
TEMP		= ZPBASE+$1E	;Temporary storage
NIBHI		= ZPBASE+$EB	;Low nybble for ASCII to hex conversion
CKSUM		= ZPBASE+$EC 	;Checksum storage for Intel HEX loader
CSFLAG		= ZPBASE+$ED	;Flag to indicate checksum failure
READ_PTR	= ZPBASE+$EE	;Input buffer read pointer
WRITE_PTR	= ZPBASE+$EF	;Input buffer write pointer

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;SETUP -- Prepare hardware, print monitor sign-on message
;
;Falls through to WSTART.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
SETUP:
;	pha                     ;Save registers A
;	phx			; X
;	phy			; Y

	SEI			;Disable interrupts
	CLD			;Clear decimal arithmetic mode.

	LDX	#$FF		;Set up stack pointer
	TXS

	sta     $cfff           ;Turn off co-resident ROMs
        sta     $c200           ;Select co-resident ROM in slot 2

	JSR	IOSET		;Do I/O module setup
	CLI			;Enable interrupts
	LDY	#SIGNON		;Y = offset to SIGNON string
	JSR	PRTCLS		;Print signon message

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;WSTART -- Warm start the monitor
;
;Falls through to the command processor
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
WSTART:
	LDX	#$FF		;Reload stack pointer since
	TXS			;we may have CANCELed 
	.include "scp.inc"
	.include "common.inc"

;Interrupt Data
AIPARMS:
	.byte	$02
	.byte	$01
	.word	IRQ_HANDLER
DIPARMS:
	.byte	$01
	.byte	$00
QPARMS:
	.byte	$04
	.byte	$00
	.word	$0000
	.byte	$00
	.word	$0000

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Message Strings
;
;These strings are indexed by the labels following STRNGS.
;These indexes may be fed into PRTSTR, PRTCLS, or PRTERR in
;the Y register.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
STRNGS		= *
SIGNON		= ^(* - STRNGS - 1)
	.byte   "GWMON-65 0.2 D" ,'P' + $80

PROMPT		= * - STRNGS - 1
	.byte	LF, '<' + $80

ERRSTR		= * - STRNGS - 1
	.byte   "ERRO", 'R' + $80

