/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#ifdef INCLUDE_RCSID_S
	.stringz "@(#)$KmKId: size_tab.h,v 1.13 2002-11-19 03:10:38-05 kadickey Exp $"
#else

	0x1,	/* 00 */ 	/* brk */
	0x1,	/* 01 */ 	/* ORA (Dloc,X) */
	0x1,	/* 02 */ 	/* COP */
	0x1,	/* 03 */ 	/* ORA Disp8,S */
	0x1,	/* 04 */ 	/* TSB Dloc */
	0x1,	/* 05 */ 	/* ORA Dloc */
	0x1,	/* 06 */ 	/* ASL Dloc */
	0x1,	/* 07 */ 	/* ORA [Dloc] */
	0x0,	/* 08 */ 	/* PHP */
	0x4,	/* 09 */ 	/* ORA #imm */
	0x0,	/* 0a */ 	/* ASL a */
	0x0,	/* 0b */ 	/* PHD */
	0x2,	/* 0c */ 	/* TSB abs */
	0x2,	/* 0d */ 	/* ORA abs */
	0x2,	/* 0e */ 	/* ASL abs */
	0x3,	/* 0f */ 	/* ORA long */
	0x1,	/* 10 */ 	/* BPL disp8 */
	0x1,	/* 11 */ 	/* ORA (),y */
	0x1,	/* 12 */ 	/* ORA () */
	0x1,	/* 13 */ 	/* ORA (disp8,s),y */
	0x1,	/* 14 */ 	/* TRB Dloc */
	0x1,	/* 15 */ 	/* ORA Dloc,x */
	0x1,	/* 16 */ 	/* ASL Dloc,x */
	0x1,	/* 17 */ 	/* ORA [],y */
	0x0,	/* 18 */ 	/* clc */
	0x2,	/* 19 */ 	/* ORA abs,y */
	0x0,	/* 1a */ 	/* INC a */
	0x0,	/* 1b */ 	/* TCS */
	0x2,	/* 1c */ 	/* TRB Abs */
	0x2,	/* 1d */ 	/* ORA Abs,X */
	0x2,	/* 1e */ 	/* ASL abs,x */
	0x3,	/* 1f */ 	/* ORA Long,x */
	0x2,	/* 20 */ 	/* JSR abs */
	0x1,	/* 21 */ 	/* AND (Dloc,X) */
	0x3,	/* 22 */ 	/* JSL Abslong */
	0x1,	/* 23 */ 	/* AND Disp8,S */
	0x1,	/* 24 */ 	/* BIT Dloc */
	0x1,	/* 25 */ 	/* AND Dloc */
	0x1,	/* 26 */ 	/* ROL Dloc */
	0x1,	/* 27 */ 	/* AND [Dloc] */
	0x0,	/* 28 */ 	/* PLP */
	0x4,	/* 29 */ 	/* AND #imm */
	0x0,	/* 2a */ 	/* ROL a */
	0x0,	/* 2b */ 	/* PLD */
	0x2,	/* 2c */ 	/* BIT abs */
	0x2,	/* 2d */ 	/* AND abs */
	0x2,	/* 2e */ 	/* ROL abs */
	0x3,	/* 2f */ 	/* AND long */
	0x1,	/* 30 */ 	/* BMI disp8 */
	0x1,	/* 31 */ 	/* AND (),y */
	0x1,	/* 32 */ 	/* AND () */
	0x1,	/* 33 */ 	/* AND (disp8,s),y */
	0x1,	/* 34 */ 	/* BIT Dloc,X */
	0x1,	/* 35 */ 	/* AND Dloc,x */
	0x1,	/* 36 */ 	/* ROL Dloc,x */
	0x1,	/* 37 */ 	/* AND [],y */
	0x0,	/* 38 */ 	/* SEC */
	0x2,	/* 39 */ 	/* AND abs,y */
	0x0,	/* 3a */ 	/* DEC a */
	0x0,	/* 3b */ 	/* TSC */
	0x2,	/* 3c */ 	/* BIT Abs,X */
	0x2,	/* 3d */ 	/* AND Abs,X */
	0x2,	/* 3e */ 	/* ROL abs,x */
	0x3,	/* 3f */ 	/* AND Long,x */
	0x0,	/* 40 */ 	/* RTI */
	0x1,	/* 41 */ 	/* EOR (Dloc,X) */
	0x1,	/* 42 */ 	/* WDM */
	0x1,	/* 43 */ 	/* EOR Disp8,S */
	0x2,	/* 44 */ 	/* MVP I,J */
	0x1,	/* 45 */ 	/* EOR Dloc */
	0x1,	/* 46 */ 	/* LSR Dloc */
	0x1,	/* 47 */ 	/* EOR [Dloc] */
	0x0,	/* 48 */ 	/* PHA */
	0x4,	/* 49 */ 	/* EOR #imm */
	0x0,	/* 4a */ 	/* LSR a */
	0x0,	/* 4b */ 	/* PHK */
	0x2,	/* 4c */ 	/* JMP abs */
	0x2,	/* 4d */ 	/* EOR abs */
	0x2,	/* 4e */ 	/* LSR abs */
	0x3,	/* 4f */ 	/* EOR long */
	0x1,	/* 50 */ 	/* BVC disp8 */
	0x1,	/* 51 */ 	/* EOR (),y */
	0x1,	/* 52 */ 	/* EOR () */
	0x1,	/* 53 */ 	/* EOR (disp8,s),y */
	0x2,	/* 54 */ 	/* MVN I,J */
	0x1,	/* 55 */ 	/* EOR Dloc,x */
	0x1,	/* 56 */ 	/* LSR Dloc,x */
	0x1,	/* 57 */ 	/* EOR [],y */
	0x0,	/* 58 */ 	/* CLI */
	0x2,	/* 59 */ 	/* EOR abs,y */
	0x0,	/* 5a */ 	/* PHY */
	0x0,	/* 5b */ 	/* TCD */
	0x3,	/* 5c */ 	/* JMP Long */
	0x2,	/* 5d */ 	/* EOR Abs,X */
	0x2,	/* 5e */ 	/* LSR abs,x */
	0x3,	/* 5f */ 	/* EOR Long,x */
	0x0,	/* 60 */ 	/* RTS */
	0x1,	/* 61 */ 	/* ADC (Dloc,X) */
	0x2,	/* 62 */ 	/* PER DISP16 */
	0x1,	/* 63 */ 	/* ADC Disp8,S */
	0x1,	/* 64 */ 	/* STZ Dloc */
	0x1,	/* 65 */ 	/* ADC Dloc */
	0x1,	/* 66 */ 	/* ROR Dloc */
	0x1,	/* 67 */ 	/* ADC [Dloc] */
	0x0,	/* 68 */ 	/* PLA */
	0x4,	/* 69 */ 	/* ADC #imm */
	0x0,	/* 6a */ 	/* ROR a */
	0x0,	/* 6b */ 	/* RTL */
	0x2,	/* 6c */ 	/* JMP (abs) */
	0x2,	/* 6d */ 	/* ADC abs */
	0x2,	/* 6e */ 	/* ROR abs */
	0x3,	/* 6f */ 	/* ADC long */
	0x1,	/* 70 */ 	/* BVS disp8 */
	0x1,	/* 71 */ 	/* ADC (),y */
	0x1,	/* 72 */ 	/* ADC () */
	0x1,	/* 73 */ 	/* ADC (disp8,s),y */
	0x1,	/* 74 */ 	/* STZ Dloc,X */
	0x1,	/* 75 */ 	/* ADC Dloc,x */
	0x1,	/* 76 */ 	/* ROR Dloc,x */
	0x1,	/* 77 */ 	/* ADC [],y */
	0x0,	/* 78 */ 	/* SEI */
	0x2,	/* 79 */ 	/* ADC abs,y */
	0x0,	/* 7a */ 	/* PLY */
	0x0,	/* 7b */ 	/* TDC */
	0x2,	/* 7c */ 	/* JMP (abs,x) */
	0x2,	/* 7d */ 	/* ADC Abs,X */
	0x2,	/* 7e */ 	/* ROR abs,x */
	0x3,	/* 7f */ 	/* ADC Long,x */
	0x1,	/* 80 */ 	/* BRA Disp8 */
	0x1,	/* 81 */ 	/* STA (Dloc,X) */
	0x2,	/* 82 */ 	/* BRL DISP16 */
	0x1,	/* 83 */ 	/* STA Disp8,S */
	0x1,	/* 84 */ 	/* STY Dloc */
	0x1,	/* 85 */ 	/* STA Dloc */
	0x1,	/* 86 */ 	/* STX Dloc */
	0x1,	/* 87 */ 	/* STA [Dloc] */
	0x0,	/* 88 */ 	/* DEY */
	0x4,	/* 89 */ 	/* BIT #imm */
	0x0,	/* 8a */ 	/* TXA */
	0x0,	/* 8b */ 	/* PHB */
	0x2,	/* 8c */ 	/* STY abs */
	0x2,	/* 8d */ 	/* STA abs */
	0x2,	/* 8e */ 	/* STX abs */
	0x3,	/* 8f */ 	/* STA long */
	0x1,	/* 90 */ 	/* BCC disp8 */
	0x1,	/* 91 */ 	/* STA (),y */
	0x1,	/* 92 */ 	/* STA () */
	0x1,	/* 93 */ 	/* STA (disp8,s),y */
	0x1,	/* 94 */ 	/* STY Dloc,X */
	0x1,	/* 95 */ 	/* STA Dloc,x */
	0x1,	/* 96 */ 	/* STX Dloc,y */
	0x1,	/* 97 */ 	/* STA [],y */
	0x0,	/* 98 */ 	/* TYA */
	0x2,	/* 99 */ 	/* STA abs,y */
	0x0,	/* 9a */ 	/* TXS */
	0x0,	/* 9b */ 	/* TXY */
	0x2,	/* 9c */ 	/* STX abs */
	0x2,	/* 9d */ 	/* STA Abs,X */
	0x2,	/* 9e */ 	/* STZ abs,x */
	0x3,	/* 9f */ 	/* STA Long,x */
	0x5,	/* a0 */ 	/* LDY #imm */
	0x1,	/* a1 */ 	/* LDA (Dloc,X) */
	0x5,	/* a2 */ 	/* LDX #imm */
	0x1,	/* a3 */ 	/* LDA Disp8,S */
	0x1,	/* a4 */ 	/* LDY Dloc */
	0x1,	/* a5 */ 	/* LDA Dloc */
	0x1,	/* a6 */ 	/* LDX Dloc */
	0x1,	/* a7 */ 	/* LDA [Dloc] */
	0x0,	/* a8 */ 	/* TAY */
	0x4,	/* a9 */ 	/* LDA #imm */
	0x0,	/* aa */ 	/* TAX */
	0x0,	/* ab */ 	/* PLB */
	0x2,	/* ac */ 	/* LDY abs */
	0x2,	/* ad */ 	/* LDA abs */
	0x2,	/* ae */ 	/* LDX abs */
	0x3,	/* af */ 	/* LDA long */
	0x1,	/* b0 */ 	/* BCS disp8 */
	0x1,	/* b1 */ 	/* LDA (),y */
	0x1,	/* b2 */ 	/* LDA () */
	0x1,	/* b3 */ 	/* LDA (disp8,s),y */
	0x1,	/* b4 */ 	/* LDY Dloc,X */
	0x1,	/* b5 */ 	/* LDA Dloc,x */
	0x1,	/* b6 */ 	/* LDX Dloc,y */
	0x1,	/* b7 */ 	/* LDA [],y */
	0x0,	/* b8 */ 	/* CLV */
	0x2,	/* b9 */ 	/* LDA abs,y */
	0x0,	/* ba */ 	/* TSX */
	0x0,	/* bb */ 	/* TYX */
	0x2,	/* bc */ 	/* LDY abs,x */
	0x2,	/* bd */ 	/* LDA Abs,X */
	0x2,	/* be */ 	/* LDX abs,y */
	0x3,	/* bf */ 	/* LDA Long,x */
	0x5,	/* c0 */ 	/* CPY #Imm */
	0x1,	/* c1 */ 	/* CMP (Dloc,X) */
	0x1,	/* c2 */ 	/* REP #8bit */
	0x1,	/* c3 */ 	/* CMP Disp8,S */
	0x1,	/* c4 */ 	/* CPY Dloc */
	0x1,	/* c5 */ 	/* CMP Dloc */
	0x1,	/* c6 */ 	/* DEC Dloc */
	0x1,	/* c7 */ 	/* CMP [Dloc] */
	0x0,	/* c8 */ 	/* INY */
	0x4,	/* c9 */ 	/* CMP #imm */
	0x0,	/* ca */ 	/* DEX */
	0x0,	/* cb */ 	/* WAI */
	0x2,	/* cc */ 	/* CPY abs */
	0x2,	/* cd */ 	/* CMP abs */
	0x2,	/* ce */ 	/* DEC abs */
	0x3,	/* cf */ 	/* CMP long */
	0x1,	/* d0 */ 	/* BNE disp8 */
	0x1,	/* d1 */ 	/* CMP (),y */
	0x1,	/* d2 */ 	/* CMP () */
	0x1,	/* d3 */ 	/* CMP (disp8,s),y */
	0x1,	/* d4 */ 	/* PEI Dloc */
	0x1,	/* d5 */ 	/* CMP Dloc,x */
	0x1,	/* d6 */ 	/* DEC Dloc,x */
	0x1,	/* d7 */ 	/* CMP [],y */
	0x0,	/* d8 */ 	/* CLD */
	0x2,	/* d9 */ 	/* CMP abs,y */
	0x0,	/* da */ 	/* PHX */
	0x0,	/* db */ 	/* STP */
	0x2,	/* dc */ 	/* JML (Abs) */
	0x2,	/* dd */ 	/* CMP Abs,X */
	0x2,	/* de */ 	/* DEC abs,x */
	0x3,	/* df */ 	/* CMP Long,x */
	0x5,	/* e0 */ 	/* CPX #Imm */
	0x1,	/* e1 */ 	/* SBC (Dloc,X) */
	0x1,	/* e2 */ 	/* SEP #8bit */
	0x1,	/* e3 */ 	/* SBC Disp8,S */
	0x1,	/* e4 */ 	/* CPX Dloc */
	0x1,	/* e5 */ 	/* SBC Dloc */
	0x1,	/* e6 */ 	/* INC Dloc */
	0x1,	/* e7 */ 	/* SBC [Dloc] */
	0x0,	/* e8 */ 	/* INX */
	0x4,	/* e9 */ 	/* SBC #imm */
	0x0,	/* ea */ 	/* NOP */
	0x0,	/* eb */ 	/* XBA */
	0x2,	/* ec */ 	/* CPX abs */
	0x2,	/* ed */ 	/* SBC abs */
	0x2,	/* ee */ 	/* INC abs */
	0x3,	/* ef */ 	/* SBC long */
	0x1,	/* f0 */ 	/* BEQ disp8 */
	0x1,	/* f1 */ 	/* SBC (),y */
	0x1,	/* f2 */ 	/* SBC () */
	0x1,	/* f3 */ 	/* SBC (disp8,s),y */
	0x2,	/* f4 */ 	/* PEA Imm */
	0x1,	/* f5 */ 	/* SBC Dloc,x */
	0x1,	/* f6 */ 	/* INC Dloc,x */
	0x1,	/* f7 */ 	/* SBC [],y */
	0x0,	/* f8 */ 	/* SED */
	0x2,	/* f9 */ 	/* SBC abs,y */
	0x0,	/* fa */ 	/* PLX */
	0x0,	/* fb */ 	/* XCE */
	0x2,	/* fc */ 	/* JSR (Abs,x) */
	0x2,	/* fd */ 	/* SBC Abs,X */
	0x2,	/* fe */ 	/* INC abs,x */
	0x3,	/* ff */ 	/* SBC Long,x */



#endif
