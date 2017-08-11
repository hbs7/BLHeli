;**** **** **** **** ****
;
; BLHeli program for controlling brushless motors in helicopters and multirotors
;
; Copyright 2011, 2012 Steffen Skaug
; This program is distributed under the terms of the GNU General Public License
;
; This file is part of BLHeli.
;
; BLHeli is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
;
; BLHeli is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with BLHeli.  If not, see <http://www.gnu.org/licenses/>.
;
;**** **** **** **** ****
;
; Flycolor Fairy V2 30A hardware definition file
;
;**** **** **** **** ****



;*********************
; Device SiLabs F850
;*********************
$include (c8051f850.inc)

;**** **** **** **** ****
; Uses internal calibrated oscillator set to 24Mhz
;**** **** **** **** ****

;**** **** **** **** ****
; Constant definitions
;**** **** **** **** ****
CSEG AT 1A40h
Eep_ESC_Layout:		DB	"#FC_FairyV2_30A#"	; ESC layout tag
CSEG AT 1A50h
Eep_ESC_MCU:			DB	"#BLHELI#F850#   "	; Project and MCU tag (16 Bytes)

MCU_48MHZ				EQU	0	; Set to 1 if MCU can run at 48MHz
ONE_S_CAPABLE			EQU	0	; Set to 1 if ESC can operate at 1S
PORT3_EXIST			EQU	0	; Set to 1 if MCU has port3
COMP1_USED			EQU	0	; Set to 1 if MCU has comparator 1 and it is being used
LOCK_BYTE_ADDRESS_16K	EQU	3FFFh	; Address of lock byte if 16k flash size
LOCK_BYTE_ADDRESS_8K	EQU	1FFFh	; Address of lock byte if 8k flash size
HIGH_BEC_VOLTAGE		EQU	0	; Set to 1 or more if high BEC voltage is supported
DAMPED_MODE_ENABLE		EQU	1	; Damped mode enabled
NFETON_DELAY			EQU	8	; Wait delay from pfets off to nfets on
PFETON_DELAY			EQU	8	; Wait delay from nfets off to pfets on
ADC_LIMIT_L			EQU 	85	; Power supply measurement ADC value for which main motor power is limited (low byte)
ADC_LIMIT_H			EQU	0	; Power supply measurement ADC value for which main motor power is limited (2 MSBs)
TEMP_LIMIT			EQU 	103	; Temperature measurement ADC value for which main motor power is limited (low byte, assuming high byte is 1)
TEMP_LIMIT_STEP		EQU	4	; Temperature measurement ADC value increment for which main motor power is further limited

;**** **** **** **** ****
; ESC specific defaults
;**** **** **** **** ****
DEFAULT_PGM_MAIN_SPOOLUP_TIME		EQU 10	; Main motor spoolup time
DEFAULT_PGM_MAIN_STARTUP_PWR 		EQU 9 	; 1=0.031 2=0.047 3=0.063 4=0.094 5=0.125 6=0.188	7=0.25  8=0.38  9=0.50  10=0.75 11=1.00 12=1.25 13=1.50
DEFAULT_PGM_TAIL_STARTUP_PWR 		EQU 9 	; 1=0.031 2=0.047 3=0.063 4=0.094 5=0.125 6=0.188	7=0.25  8=0.38  9=0.50  10=0.75 11=1.00 12=1.25 13=1.50
DEFAULT_PGM_MULTI_STARTUP_PWR 	EQU 9 	; 1=0.031 2=0.047 3=0.063 4=0.094 5=0.125 6=0.188	7=0.25  8=0.38  9=0.50  10=0.75 11=1.00 12=1.25 13=1.50

;**** **** **** **** ****
; Bootloader definitions
;**** **** **** **** ****
RTX_PORT		  		EQU 	P0		; Receive/Transmit port
RTX_MDOUT				EQU	P0MDOUT	; Set to 1 for PUSHPULL				
RTX_MDIN				EQU	P0MDIN	; Set to 1 for DIGITAL				
RTX_SKIP				EQU	P0SKIP	; Set to 1 for SKIP				
RTX_PIN				EQU 	5		; RTX pin

SIGNATURE_001			EQU	0f8h		; Device signature
SIGNATURE_002			EQU	050h


;*********************
; PORT 0 definitions *
;*********************  
;			EQU	7	;i (Temp)
Adc_Ip		EQU	6	;i
Rcp_In		EQU	5	;i
;			EQU	4	;i (Rcp 1k)
Mux_C		EQU	3	;i
Mux_B		EQU	2	;i
Mux_A		EQU	1	;i
Comp_Com		EQU	0	;i

P0_DIGITAL	EQU	NOT((1 SHL Mux_A)+(1 SHL Mux_B)+(1 SHL Mux_C)+(1 SHL Comp_Com)+(1 SHL Adc_Ip))
P0_INIT		EQU	0FFh
P0_PUSHPULL	EQU	0
P0_SKIP		EQU	NOT(1 SHL Rcp_In) AND 0FFh

MACRO Get_Rcp_Capture_Values
	mov	Temp1, PCA0CPL0			; Get PCA capture values
	mov	Temp2, PCA0CPH0
ENDM
MACRO Read_Rcp_Int
	mov	A, P0
	jnb	Flags3.PGM_RCP_PWM_POL, ($+4)	; Is pwm polarity negative?
	cpl	A						; Yes - invert
ENDM
MACRO Rcp_Int_Enable
	orl	PCA0CPM0, #01h				; Interrupt enabled
ENDM
MACRO Rcp_Int_Disable
	anl	PCA0CPM0, #0FEh			; Interrupt disabled
ENDM
MACRO Rcp_Int_First
	anl	PCA0CPM0, #0CFh			
	jb	Flags3.PGM_RCP_PWM_POL, ($+6)	; Is pwm polarity positive?
	orl	PCA0CPM0, #20h				; Capture rising edge
	jnb	Flags3.PGM_RCP_PWM_POL, ($+6)	; Is pwm polarity negative?
	orl	PCA0CPM0, #10h				; Capture falling edge
ENDM
MACRO Rcp_Int_Second
	anl	PCA0CPM0, #0CFh			
	jb	Flags3.PGM_RCP_PWM_POL, ($+6)	; Is pwm polarity positive?
	orl	PCA0CPM0, #10h				; Capture falling edge
	jnb	Flags3.PGM_RCP_PWM_POL, ($+6)	; Is pwm polarity negative?
	orl	PCA0CPM0, #20h				; Capture rising edge
ENDM
MACRO Rcp_Clear_Int_Flag
	clr 	CCF0						; Clear interrupt flag
ENDM		


;*********************
; PORT 1 definitions *
;*********************
;			EQU	7	;i
;			EQU	6	;i
CnFET		EQU	5	;o
CpFET		EQU	4	;o
BnFET		EQU	3	;o
BpFET		EQU	2	;o
AnFET		EQU	1	;o
ApFET		EQU	0	;o

P1_DIGITAL	EQU	(1 SHL AnFET)+(1 SHL BnFET)+(1 SHL CnFET)+(1 SHL ApFET)+(1 SHL BpFET)+(1 SHL CpFET)
P1_INIT		EQU 	0		
P1_PUSHPULL	EQU	(1 SHL AnFET)+(1 SHL BnFET)+(1 SHL CnFET)+(1 SHL ApFET)+(1 SHL BpFET)+(1 SHL CpFET)
P1_SKIP		EQU	0

MACRO AnFET_on
	setb	P1.AnFET
ENDM
MACRO AnFET_off
	clr	P1.AnFET
ENDM
MACRO BnFET_on
	setb	P1.BnFET
ENDM
MACRO BnFET_off
	clr	P1.BnFET
ENDM
MACRO CnFET_on
	setb	P1.CnFET
ENDM
MACRO CnFET_off
	clr	P1.CnFET
ENDM
MACRO All_nFETs_Off
	clr	P1.AnFET
	clr	P1.BnFET
	clr	P1.CnFET
ENDM

MACRO ApFET_on
	setb	P1.ApFET
ENDM
MACRO ApFET_off
	clr	P1.ApFET
ENDM
MACRO BpFET_on
	setb	P1.BpFET
ENDM
MACRO BpFET_off
	clr	P1.BpFET
ENDM
MACRO CpFET_on
	setb	P1.CpFET
ENDM
MACRO CpFET_off
	clr	P1.CpFET
ENDM
MACRO All_pFETs_Off
	clr	P1.ApFET
	clr	P1.BpFET
	clr	P1.CpFET
ENDM

MACRO Brake_FETs_On	
	AnFET_on
	BnFET_on
	CnFET_on
ENDM
MACRO Damping_FET_On
	mov	A, DampingFET
	orl	P1, A
ENDM

MACRO Set_Comp_Phase_A
	mov	CPT0MX, #01h	; Set comparator multiplexer to phase A
ENDM
MACRO Set_Comp_Phase_B
	mov	CPT0MX, #02h	; Set comparator multiplexer to phase B
ENDM
MACRO Set_Comp_Phase_C
	mov	CPT0MX, #03h	; Set comparator multiplexer to phase C
ENDM
MACRO Read_Comp_Out
	mov	A, CPT0CN		; Read comparator output
ENDM


;*********************
; PORT 2 definitions *
;*********************
DebugPin		EQU	0	;o

P2_PUSHPULL	EQU	(1 SHL DebugPin)


;**********************
; MCU specific macros *
;**********************
MACRO Interrupt_Table_Definition
CSEG AT 0				; Code segment start
	jmp reset
CSEG AT 0Bh			; Timer0 interrupt	
	jmp	t0_int	
CSEG AT 2Bh			; Timer2 interrupt	
	jmp	t2_int	
CSEG AT 5Bh			; PCA interrupt	
	jmp	pca_int	
CSEG AT 73h			; Timer3 interrupt	
	jmp	t3_int	
ENDM

MACRO Initialize_Xbar
	mov	XBR2, #40h	; Xbar enabled			
	mov	XBR1, #01h	; CEX0 routed to pin Rcp_In			
ENDM

MACRO Initialize_Adc
	mov	REF0CN, #0Ch	; Set vdd (3.3V) as reference. Enable temp sensor and bias
	mov	ADC0CF, #59h	; ADC clock 2MHz, PGA gain 1
	mov	ADC0MX, #Adc_Ip; Select ADC input
	mov	ADC0CN0, #80h	; ADC enabled 
	mov	ADC0CN1, #01h	; Common mode buffer enabled
ENDM
MACRO Set_Adc_Ip_Volt
	mov	ADC0MX, #Adc_Ip; Select ADC input
ENDM
MACRO Set_Adc_Ip_Temp
	mov	ADC0MX, #10h	; Select temp sensor input
ENDM
MACRO Start_Adc
	mov	ADC0CN0, #90h	; ADC start
ENDM
MACRO Read_Adc_Result
	mov	Temp1, ADC0L
	mov	Temp2, ADC0H
ENDM
MACRO Stop_Adc
ENDM
MACRO Set_RPM_Out
ENDM
MACRO Clear_RPM_Out
ENDM
MACRO Set_MCU_Clk_24MHz
ENDM
MACRO Set_MCU_Clk_48MHz
ENDM