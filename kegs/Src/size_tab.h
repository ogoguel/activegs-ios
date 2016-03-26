/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#ifdef INCLUDE_RCSID_S
	.stringz "@(#)$KmKId: size_tab.h,v 1.13 2002-11-19 03:10:38-05 kadickey Exp $"
#else

	.word	inst00_SYM+1	/* brk */
	.word	inst01_SYM+1	/* ORA (Dloc,X) */
	.word	inst02_SYM+1	/* COP */
	.word	inst03_SYM+1	/* ORA Disp8,S */
	.word	inst04_SYM+1	/* TSB Dloc */
	.word	inst05_SYM+1	/* ORA Dloc */
	.word	inst06_SYM+1	/* ASL Dloc */
	.word	inst07_SYM+1	/* ORA [Dloc] */
	.word	inst08_SYM+0	/* PHP */
	.word	inst09_SYM+4	/* ORA #imm */
	.word	inst0a_SYM+0	/* ASL a */
	.word	inst0b_SYM+0	/* PHD */
	.word	inst0c_SYM+2	/* TSB abs */
	.word	inst0d_SYM+2	/* ORA abs */
	.word	inst0e_SYM+2	/* ASL abs */
	.word	inst0f_SYM+3	/* ORA long */
	.word	inst10_SYM+1	/* BPL disp8 */
	.word	inst11_SYM+1	/* ORA (),y */
	.word	inst12_SYM+1	/* ORA () */
	.word	inst13_SYM+1	/* ORA (disp8,s),y */
	.word	inst14_SYM+1	/* TRB Dloc */
	.word	inst15_SYM+1	/* ORA Dloc,x */
	.word	inst16_SYM+1	/* ASL Dloc,x */
	.word	inst17_SYM+1	/* ORA [],y */
	.word	inst18_SYM+0	/* clc */
	.word	inst19_SYM+2	/* ORA abs,y */
	.word	inst1a_SYM+0	/* INC a */
	.word	inst1b_SYM+0	/* TCS */
	.word	inst1c_SYM+2	/* TRB Abs */
	.word	inst1d_SYM+2	/* ORA Abs,X */
	.word	inst1e_SYM+2	/* ASL abs,x */
	.word	inst1f_SYM+3	/* ORA Long,x */
	.word	inst20_SYM+2	/* JSR abs */
	.word	inst21_SYM+1	/* AND (Dloc,X) */
	.word	inst22_SYM+3	/* JSL Abslong */
	.word	inst23_SYM+1	/* AND Disp8,S */
	.word	inst24_SYM+1	/* BIT Dloc */
	.word	inst25_SYM+1	/* AND Dloc */
	.word	inst26_SYM+1	/* ROL Dloc */
	.word	inst27_SYM+1	/* AND [Dloc] */
	.word	inst28_SYM+0	/* PLP */
	.word	inst29_SYM+4	/* AND #imm */
	.word	inst2a_SYM+0	/* ROL a */
	.word	inst2b_SYM+0	/* PLD */
	.word	inst2c_SYM+2	/* BIT abs */
	.word	inst2d_SYM+2	/* AND abs */
	.word	inst2e_SYM+2	/* ROL abs */
	.word	inst2f_SYM+3	/* AND long */
	.word	inst30_SYM+1	/* BMI disp8 */
	.word	inst31_SYM+1	/* AND (),y */
	.word	inst32_SYM+1	/* AND () */
	.word	inst33_SYM+1	/* AND (disp8,s),y */
	.word	inst34_SYM+1	/* BIT Dloc,X */
	.word	inst35_SYM+1	/* AND Dloc,x */
	.word	inst36_SYM+1	/* ROL Dloc,x */
	.word	inst37_SYM+1	/* AND [],y */
	.word	inst38_SYM+0	/* SEC */
	.word	inst39_SYM+2	/* AND abs,y */
	.word	inst3a_SYM+0	/* DEC a */
	.word	inst3b_SYM+0	/* TSC */
	.word	inst3c_SYM+2	/* BIT Abs,X */
	.word	inst3d_SYM+2	/* AND Abs,X */
	.word	inst3e_SYM+2	/* ROL abs,x */
	.word	inst3f_SYM+3	/* AND Long,x */
	.word	inst40_SYM+0	/* RTI */
	.word	inst41_SYM+1	/* EOR (Dloc,X) */
	.word	inst42_SYM+1	/* WDM */
	.word	inst43_SYM+1	/* EOR Disp8,S */
	.word	inst44_SYM+2	/* MVP I,J */
	.word	inst45_SYM+1	/* EOR Dloc */
	.word	inst46_SYM+1	/* LSR Dloc */
	.word	inst47_SYM+1	/* EOR [Dloc] */
	.word	inst48_SYM+0	/* PHA */
	.word	inst49_SYM+4	/* EOR #imm */
	.word	inst4a_SYM+0	/* LSR a */
	.word	inst4b_SYM+0	/* PHK */
	.word	inst4c_SYM+2	/* JMP abs */
	.word	inst4d_SYM+2	/* EOR abs */
	.word	inst4e_SYM+2	/* LSR abs */
	.word	inst4f_SYM+3	/* EOR long */
	.word	inst50_SYM+1	/* BVC disp8 */
	.word	inst51_SYM+1	/* EOR (),y */
	.word	inst52_SYM+1	/* EOR () */
	.word	inst53_SYM+1	/* EOR (disp8,s),y */
	.word	inst54_SYM+2	/* MVN I,J */
	.word	inst55_SYM+1	/* EOR Dloc,x */
	.word	inst56_SYM+1	/* LSR Dloc,x */
	.word	inst57_SYM+1	/* EOR [],y */
	.word	inst58_SYM+0	/* CLI */
	.word	inst59_SYM+2	/* EOR abs,y */
	.word	inst5a_SYM+0	/* PHY */
	.word	inst5b_SYM+0	/* TCD */
	.word	inst5c_SYM+3	/* JMP Long */
	.word	inst5d_SYM+2	/* EOR Abs,X */
	.word	inst5e_SYM+2	/* LSR abs,x */
	.word	inst5f_SYM+3	/* EOR Long,x */
	.word	inst60_SYM+0	/* RTS */
	.word	inst61_SYM+1	/* ADC (Dloc,X) */
	.word	inst62_SYM+2	/* PER DISP16 */
	.word	inst63_SYM+1	/* ADC Disp8,S */
	.word	inst64_SYM+1	/* STZ Dloc */
	.word	inst65_SYM+1	/* ADC Dloc */
	.word	inst66_SYM+1	/* ROR Dloc */
	.word	inst67_SYM+1	/* ADC [Dloc] */
	.word	inst68_SYM+0	/* PLA */
	.word	inst69_SYM+4	/* ADC #imm */
	.word	inst6a_SYM+0	/* ROR a */
	.word	inst6b_SYM+0	/* RTL */
	.word	inst6c_SYM+2	/* JMP (abs) */
	.word	inst6d_SYM+2	/* ADC abs */
	.word	inst6e_SYM+2	/* ROR abs */
	.word	inst6f_SYM+3	/* ADC long */
	.word	inst70_SYM+1	/* BVS disp8 */
	.word	inst71_SYM+1	/* ADC (),y */
	.word	inst72_SYM+1	/* ADC () */
	.word	inst73_SYM+1	/* ADC (disp8,s),y */
	.word	inst74_SYM+1	/* STZ Dloc,X */
	.word	inst75_SYM+1	/* ADC Dloc,x */
	.word	inst76_SYM+1	/* ROR Dloc,x */
	.word	inst77_SYM+1	/* ADC [],y */
	.word	inst78_SYM+0	/* SEI */
	.word	inst79_SYM+2	/* ADC abs,y */
	.word	inst7a_SYM+0	/* PLY */
	.word	inst7b_SYM+0	/* TDC */
	.word	inst7c_SYM+2	/* JMP (abs,x) */
	.word	inst7d_SYM+2	/* ADC Abs,X */
	.word	inst7e_SYM+2	/* ROR abs,x */
	.word	inst7f_SYM+3	/* ADC Long,x */
	.word	inst80_SYM+1	/* BRA Disp8 */
	.word	inst81_SYM+1	/* STA (Dloc,X) */
	.word	inst82_SYM+2	/* BRL DISP16 */
	.word	inst83_SYM+1	/* STA Disp8,S */
	.word	inst84_SYM+1	/* STY Dloc */
	.word	inst85_SYM+1	/* STA Dloc */
	.word	inst86_SYM+1	/* STX Dloc */
	.word	inst87_SYM+1	/* STA [Dloc] */
	.word	inst88_SYM+0	/* DEY */
	.word	inst89_SYM+4	/* BIT #imm */
	.word	inst8a_SYM+0	/* TXA */
	.word	inst8b_SYM+0	/* PHB */
	.word	inst8c_SYM+2	/* STY abs */
	.word	inst8d_SYM+2	/* STA abs */
	.word	inst8e_SYM+2	/* STX abs */
	.word	inst8f_SYM+3	/* STA long */
	.word	inst90_SYM+1	/* BCC disp8 */
	.word	inst91_SYM+1	/* STA (),y */
	.word	inst92_SYM+1	/* STA () */
	.word	inst93_SYM+1	/* STA (disp8,s),y */
	.word	inst94_SYM+1	/* STY Dloc,X */
	.word	inst95_SYM+1	/* STA Dloc,x */
	.word	inst96_SYM+1	/* STX Dloc,y */
	.word	inst97_SYM+1	/* STA [],y */
	.word	inst98_SYM+0	/* TYA */
	.word	inst99_SYM+2	/* STA abs,y */
	.word	inst9a_SYM+0	/* TXS */
	.word	inst9b_SYM+0	/* TXY */
	.word	inst9c_SYM+2	/* STX abs */
	.word	inst9d_SYM+2	/* STA Abs,X */
	.word	inst9e_SYM+2	/* STZ abs,x */
	.word	inst9f_SYM+3	/* STA Long,x */
	.word	insta0_SYM+5	/* LDY #imm */
	.word	insta1_SYM+1	/* LDA (Dloc,X) */
	.word	insta2_SYM+5	/* LDX #imm */
	.word	insta3_SYM+1	/* LDA Disp8,S */
	.word	insta4_SYM+1	/* LDY Dloc */
	.word	insta5_SYM+1	/* LDA Dloc */
	.word	insta6_SYM+1	/* LDX Dloc */
	.word	insta7_SYM+1	/* LDA [Dloc] */
	.word	insta8_SYM+0	/* TAY */
	.word	insta9_SYM+4	/* LDA #imm */
	.word	instaa_SYM+0	/* TAX */
	.word	instab_SYM+0	/* PLB */
	.word	instac_SYM+2	/* LDY abs */
	.word	instad_SYM+2	/* LDA abs */
	.word	instae_SYM+2	/* LDX abs */
	.word	instaf_SYM+3	/* LDA long */
	.word	instb0_SYM+1	/* BCS disp8 */
	.word	instb1_SYM+1	/* LDA (),y */
	.word	instb2_SYM+1	/* LDA () */
	.word	instb3_SYM+1	/* LDA (disp8,s),y */
	.word	instb4_SYM+1	/* LDY Dloc,X */
	.word	instb5_SYM+1	/* LDA Dloc,x */
	.word	instb6_SYM+1	/* LDX Dloc,y */
	.word	instb7_SYM+1	/* LDA [],y */
	.word	instb8_SYM+0	/* CLV */
	.word	instb9_SYM+2	/* LDA abs,y */
	.word	instba_SYM+0	/* TSX */
	.word	instbb_SYM+0	/* TYX */
	.word	instbc_SYM+2	/* LDY abs,x */
	.word	instbd_SYM+2	/* LDA Abs,X */
	.word	instbe_SYM+2	/* LDX abs,y */
	.word	instbf_SYM+3	/* LDA Long,x */
	.word	instc0_SYM+5	/* CPY #Imm */
	.word	instc1_SYM+1	/* CMP (Dloc,X) */
	.word	instc2_SYM+1	/* REP #8bit */
	.word	instc3_SYM+1	/* CMP Disp8,S */
	.word	instc4_SYM+1	/* CPY Dloc */
	.word	instc5_SYM+1	/* CMP Dloc */
	.word	instc6_SYM+1	/* DEC Dloc */
	.word	instc7_SYM+1	/* CMP [Dloc] */
	.word	instc8_SYM+0	/* INY */
	.word	instc9_SYM+4	/* CMP #imm */
	.word	instca_SYM+0	/* DEX */
	.word	instcb_SYM+0	/* WAI */
	.word	instcc_SYM+2	/* CPY abs */
	.word	instcd_SYM+2	/* CMP abs */
	.word	instce_SYM+2	/* DEC abs */
	.word	instcf_SYM+3	/* CMP long */
	.word	instd0_SYM+1	/* BNE disp8 */
	.word	instd1_SYM+1	/* CMP (),y */
	.word	instd2_SYM+1	/* CMP () */
	.word	instd3_SYM+1	/* CMP (disp8,s),y */
	.word	instd4_SYM+1	/* PEI Dloc */
	.word	instd5_SYM+1	/* CMP Dloc,x */
	.word	instd6_SYM+1	/* DEC Dloc,x */
	.word	instd7_SYM+1	/* CMP [],y */
	.word	instd8_SYM+0	/* CLD */
	.word	instd9_SYM+2	/* CMP abs,y */
	.word	instda_SYM+0	/* PHX */
	.word	instdb_SYM+0	/* STP */
	.word	instdc_SYM+2	/* JML (Abs) */
	.word	instdd_SYM+2	/* CMP Abs,X */
	.word	instde_SYM+2	/* DEC abs,x */
	.word	instdf_SYM+3	/* CMP Long,x */
	.word	inste0_SYM+5	/* CPX #Imm */
	.word	inste1_SYM+1	/* SBC (Dloc,X) */
	.word	inste2_SYM+1	/* SEP #8bit */
	.word	inste3_SYM+1	/* SBC Disp8,S */
	.word	inste4_SYM+1	/* CPX Dloc */
	.word	inste5_SYM+1	/* SBC Dloc */
	.word	inste6_SYM+1	/* INC Dloc */
	.word	inste7_SYM+1	/* SBC [Dloc] */
	.word	inste8_SYM+0	/* INX */
	.word	inste9_SYM+4	/* SBC #imm */
	.word	instea_SYM+0	/* NOP */
	.word	insteb_SYM+0	/* XBA */
	.word	instec_SYM+2	/* CPX abs */
	.word	insted_SYM+2	/* SBC abs */
	.word	instee_SYM+2	/* INC abs */
	.word	instef_SYM+3	/* SBC long */
	.word	instf0_SYM+1	/* BEQ disp8 */
	.word	instf1_SYM+1	/* SBC (),y */
	.word	instf2_SYM+1	/* SBC () */
	.word	instf3_SYM+1	/* SBC (disp8,s),y */
	.word	instf4_SYM+2	/* PEA Imm */
	.word	instf5_SYM+1	/* SBC Dloc,x */
	.word	instf6_SYM+1	/* INC Dloc,x */
	.word	instf7_SYM+1	/* SBC [],y */
	.word	instf8_SYM+0	/* SED */
	.word	instf9_SYM+2	/* SBC abs,y */
	.word	instfa_SYM+0	/* PLX */
	.word	instfb_SYM+0	/* XCE */
	.word	instfc_SYM+2	/* JSR (Abs,x) */
	.word	instfd_SYM+2	/* SBC Abs,X */
	.word	instfe_SYM+2	/* INC abs,x */
	.word	instff_SYM+3	/* SBC Long,x */

	.block	4*16

#endif
