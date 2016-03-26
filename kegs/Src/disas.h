/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

enum {
	ABS = 1,
	ABSX,
	ABSY,
	ABSLONG,
	ABSIND,
	ABSXIND,
	IMPLY,
	ACCUM,
	IMMED,
	JUST8,
	DLOC,
	DLOCX,
	DLOCY,
	LONG,
	LONGX,
	DLOCIND,
	DLOCINDY,
	DLOCXIND,
	DLOCBRAK,
	DLOCBRAKY,
	DISP8,
	DISP8S,
	DISP8SINDY,
	DISP16,
	MVPMVN,
	REPVAL,
	SEPVAL
};


const char * const disas_opcodes[256] = {
	"BRK", "ORA", "COP", "ORA", "TSB", "ORA", "ASL", "ORA",  /* 00-07 */
	"PHP", "ORA", "ASL", "PHD", "TSB", "ORA", "ASL", "ORA",  /* 08-0f */
	"BPL", "ORA", "ORA", "ORA", "TRB", "ORA", "ASL", "ORA",  /* 10-17 */
	"CLC", "ORA", "INC", "TCS", "TRB", "ORA", "ASL", "ORA",  /* 18-1f */
	"JSR", "AND", "JSL", "AND", "BIT", "AND", "ROL", "AND",  /* 20-27 */
	"PLP", "AND", "ROL", "PLD", "BIT", "AND", "ROL", "AND",  /* 28-2f */
	"BMI", "AND", "AND", "AND", "BIT", "AND", "ROL", "AND",  /* 30-37 */
	"SEC", "AND", "DEC", "TSC", "BIT", "AND", "ROL", "AND",  /* 38-3f */
	"RTI", "EOR", "WDM", "EOR", "MVP", "EOR", "LSR", "EOR",  /* 40-47 */
	"PHA", "EOR", "LSR", "PHK", "JMP", "EOR", "LSR", "EOR",  /* 48-4f */
	"BVC", "EOR", "EOR", "EOR", "MVN", "EOR", "LSR", "EOR",  /* 50-57 */
	"CLI", "EOR", "PHY", "TCD", "JMP", "EOR", "LSR", "EOR",  /* 58-5f */
	"RTS", "ADC", "PER", "ADC", "STZ", "ADC", "ROR", "ADC",  /* 60-67 */
	"PLA", "ADC", "ROR", "RTL", "JMP", "ADC", "ROR", "ADC",  /* 68-6f */
	"BVS", "ADC", "ADC", "ADC", "STZ", "ADC", "ROR", "ADC",  /* 70-77 */
	"SEI", "ADC", "PLY", "TDC", "JMP", "ADC", "ROR", "ADC",  /* 78-7f */
	"BRA", "STA", "BRL", "STA", "STY", "STA", "STX", "STA",  /* 80-87 */
	"DEY", "BIT", "TXA", "PHB", "STY", "STA", "STX", "STA",  /* 88-8f */
	"BCC", "STA", "STA", "STA", "STY", "STA", "STX", "STA",  /* 90-97 */
	"TYA", "STA", "TXS", "TXY", "STZ", "STA", "STZ", "STA",  /* 98-9f */
	"LDY", "LDA", "LDX", "LDA", "LDY", "LDA", "LDX", "LDA",  /* a0-a7 */
	"TAY", "LDA", "TAX", "PLB", "LDY", "LDA", "LDX", "LDA",  /* a8-af */
	"BCS", "LDA", "LDA", "LDA", "LDY", "LDA", "LDX", "LDA",  /* b0-b7 */
	"CLV", "LDA", "TSX", "TYX", "LDY", "LDA", "LDX", "LDA",  /* b8-bf */
	"CPY", "CMP", "REP", "CMP", "CPY", "CMP", "DEC", "CMP",  /* c0-c7 */
	"INY", "CMP", "DEX", "WAI", "CPY", "CMP", "DEC", "CMP",  /* c8-cf */
	"BNE", "CMP", "CMP", "CMP", "PEI", "CMP", "DEC", "CMP",  /* d0-d7 */
	"CLD", "CMP", "PHX", "STP", "JML", "CMP", "DEC", "CMP",  /* d8-df */
	"CPX", "SBC", "SEP", "SBC", "CPX", "SBC", "INC", "SBC",  /* e0-e7 */
	"INX", "SBC", "NOP", "XBA", "CPX", "SBC", "INC", "SBC",  /* e8-ef */
	"BEQ", "SBC", "SBC", "SBC", "PEA", "SBC", "INC", "SBC",  /* f0-f7 */
	"SED", "SBC", "PLX", "XCE", "JSR", "SBC", "INC", "SBC",  /* f8-ff */
};


const word32 disas_types[256] = {
	JUST8+0x100, DLOCXIND+0x100,		/* 00-01 */
	JUST8+0x100, DISP8S+0x100,		/* 02-03 */
	DLOC+0x100, DLOC+0x100,			/* 04-05 */
	DLOC+0x100, DLOCBRAK+0x100,		/* 06-07 */
	IMPLY+0x000, IMMED+0x400,		/* 08-9 */
	ACCUM+0x000, IMPLY+0x000,		/* 0a-b */
	ABS+0x200, ABS+0x200,			/* c-d */
	ABS+0x200, LONG+0x300,			/* e-f */
	DISP8+0x100, DLOCINDY+0x100,		/* 10-11 */
	DLOCIND+0x100, DISP8SINDY+0x100,	/* 12-13 */
	DLOC+0x100, DLOCX+0x100,		/* 14-15 */
	DLOCX+0x100, DLOCBRAKY+0x100,		/* 16-17 */
	IMPLY+0x000, ABSY+0x200,		/* 18-19 */
	ACCUM+0x000, IMPLY+0x000,		/* 1a-1b */
	ABS+0x200, ABSX+0x200,			/* 1c-1d */
	ABSX+0x200, LONGX+0x300,		/* 1e-1f */
	ABS+0x200, DLOCXIND+0x100,		/* 20-21 */
	ABSLONG+0x300, DISP8S+0x100,		/* 22-23 */
	DLOC+0x100, DLOC+0x100,			/* 24-25 */
	DLOC+0x100, DLOCBRAK+0x100,		/* 26-27 */
	IMPLY+0x000, IMMED+0x400,		/* 28-29 */
	ACCUM+0x000, IMPLY+0x000,		/* 2a-2b */
	ABS+0x200, ABS+0x200,			/* 2c-2d */
	ABS+0x200, LONG+0x300,			/* 2e-2f */
	DISP8+0x100, DLOCINDY+0x100,		/* 30-31 */
	DLOCIND+0x100, DISP8SINDY+0x100,	/* 32-33 */
	DLOCX+0x100, DLOCX+0x100,		/* 34-35 */
	DLOCX+0x100, DLOCBRAKY+0x100,		/* 36-37 */
	IMPLY+0x000, ABSY+0x200,		/* 38-39 */
	ACCUM+0x000, IMPLY+0x000,		/* 3a-3b */
	ABSX+0x200, ABSX+0x200,			/* 3c-3d */
	ABSX+0x200, LONGX+0x300,		/* 3e-3f */
	IMPLY+0x000, DLOCXIND+0x100,		/* 40-41 */
	JUST8+0x100, DISP8S+0x100,		/* 42-43 */
	MVPMVN+0x200, DLOC+0x100,		/* 44-45 */
	DLOC+0x100, DLOCBRAK+0x100,		/* 46-47 */
	IMPLY+0x000, IMMED+0x400,		/* 48-49 */
	ACCUM+0x000, IMPLY+0x000,		/* 4a-4b */
	ABS+0x200, ABS+0x200,			/* 4c-4d */
	ABS+0x200, LONG+0x300,			/* 4e-4f */
	DISP8+0x100, DLOCINDY+0x100,		/* 50-51 */
	DLOCIND+0x100, DISP8SINDY+0x100,	/* 52-53 */
	MVPMVN+0x200, DLOCX+0x100,		/* 54-55 */
	DLOCX+0x100, DLOCBRAKY+0x100,		/* 56-57 */
	IMPLY+0x000, ABSY+0x200,		/* 58-59 */
	IMPLY+0x000, IMPLY+0x000,		/* 5a-5b */
	LONG+0x300, ABSX+0x200,			/* 5c-5d */
	ABSX+0x200, LONGX+0x300,		/* 5e-5f */
	IMPLY+0x000, DLOCXIND+0x100,		/* 60-61 */
	DISP16+0x200, DISP8S+0x100,		/* 62-63 */
	DLOC+0x100, DLOC+0x100,			/* 64-65 */
	DLOC+0x100, DLOCBRAK+0x100,		/* 66-67 */
	IMPLY+0x000, IMMED+0x400,		/* 68-69 */
	ACCUM+0x000, IMPLY+0x000,		/* 6a-6b */
	ABSIND+0x200, ABS+0x200,		/* 6c-6d */
	ABS+0x200, LONG+0x300,			/* 6e-6f */
	DISP8+0x100, DLOCINDY+0x100,		/* 70-71 */
	DLOCIND+0x100, DISP8SINDY+0x100,	/* 72-73 */
	DLOCX+0x100, DLOCX+0x100,		/* 74-75 */
	DLOCX+0x100, DLOCBRAKY+0x100,		/* 76-77 */
	IMPLY+0x000, ABSY+0x200,		/* 78-79 */
	IMPLY+0x000, IMPLY+0x000,		/* 7a-7b */
	ABSXIND+0x200, ABSX+0x200,		/* 7c-7d */
	ABSX+0x200, LONGX+0x300,		/* 7e-7f */
	DISP8+0x100, DLOCXIND+0x100,		/* 80-81 */
	DISP16+0x200, DISP8S+0x100,		/* 82-83 */
	DLOC+0x100, DLOC+0x100,			/* 84-85 */
	DLOC+0x100, DLOCBRAK+0x100,		/* 86-87 */
	IMPLY+0x000, IMMED+0x400,		/* 88-89 */
	IMPLY+0x000, IMPLY+0x000,		/* 8a-8b */
	ABS+0x200, ABS+0x200,			/* 8c-8d */
	ABS+0x200, LONG+0x300,			/* 8e-8f */
	DISP8+0x100, DLOCINDY+0x100,		/* 90-91 */
	DLOCIND+0x100, DISP8SINDY+0x100,	/* 92-93 */
	DLOCX+0x100, DLOCX+0x100,		/* 94-95 */
	DLOCY+0x100, DLOCBRAKY+0x100,		/* 96-97 */
	IMPLY+0x000, ABSY+0x200,		/* 98-99 */
	IMPLY+0x000, IMPLY+0x000,		/* 9a-9b */
	ABS+0x200, ABSX+0x200,			/* 9c-9d */
	ABSX+0x200, LONGX+0x300,		/* 9e-9f */
	IMMED+0x500, DLOCXIND+0x100,		/* a0-a1 */
	IMMED+0x500, DISP8S+0x100,		/* a2-a3 */
	DLOC+0x100, DLOC+0x100,			/* a4-a5 */
	DLOC+0x100, DLOCBRAK+0x100,		/* a6-a7 */
	IMPLY+0x000, IMMED+0x400,		/* a8-a9 */
	IMPLY+0x000, IMPLY+0x000,		/* aa-ab */
	ABS+0x200, ABS+0x200,			/* ac-ad */
	ABS+0x200, LONG+0x300,			/* ae-af */
	DISP8+0x100, DLOCINDY+0x100,		/* b0-b1 */
	DLOCIND+0x100, DISP8SINDY+0x100,	/* b2-b3 */
	DLOCX+0x100, DLOCX+0x100,		/* b4-b5 */
	DLOCY+0x100, DLOCBRAKY+0x100,		/* b6-b7 */
	IMPLY+0x000, ABSY+0x200,		/* b8-b9 */
	IMPLY+0x000, IMPLY+0x000,		/* ba-bb */
	ABSX+0x200, ABSX+0x200,			/* bc-bd */
	ABSY+0x200, LONGX+0x300,		/* be-bf */
	IMMED+0x500, DLOCXIND+0x100,		/* c0-c1 */
	REPVAL+0x100, DISP8S+0x100,		/* c2-c3 */
	DLOC+0x100, DLOC+0x100,			/* c4-c5 */
	DLOC+0x100, DLOCBRAK+0x100,		/* c6-c7 */
	IMPLY+0x000, IMMED+0x400,		/* c8-c9 */
	IMPLY+0x000, IMPLY+0x000,		/* ca-cb */
	ABS+0x200, ABS+0x200,			/* cc-cd */
	ABS+0x200, LONG+0x300,			/* ce-cf */
	DISP8+0x100, DLOCINDY+0x100,		/* d0-d1 */
	DLOCIND+0x100, DISP8SINDY+0x100,	/* d2-d3 */
	DLOC+0x100, DLOCX+0x100,		/* d4-d5 */
	DLOCX+0x100, DLOCBRAKY+0x100,		/* d6-d7 */
	IMPLY+0x000, ABSY+0x200,		/* d8-d9 */
	IMPLY+0x000, IMPLY+0x000,		/* da-db */
	ABSIND+0x200, ABSX+0x200,		/* dc-dd */
	ABSX+0x200, LONGX+0x300,		/* de-df */
	IMMED+0x500, DLOCXIND+0x100,		/* e0-e1 */
	SEPVAL+0x100, DISP8S+0x100,		/* e2-e3 */
	DLOC+0x100, DLOC+0x100,			/* e4-e5 */
	DLOC+0x100, DLOCBRAK+0x100,		/* e6-e7 */
	IMPLY+0x000, IMMED+0x400,		/* e8-e9 */
	IMPLY+0x000, IMPLY+0x000,		/* ea-eb */
	ABS+0x200, ABS+0x200,			/* ec-ed */
	ABS+0x200, LONG+0x300,			/* ee-ef */
	DISP8+0x100, DLOCINDY+0x100,		/* f0-f1 */
	DLOCIND+0x100, DISP8SINDY+0x100,	/* f2-f3 */
	IMMED+0x200, DLOCX+0x100,		/* f4-f5 */
	DLOCX+0x100, DLOCBRAKY+0x100,		/* f6-f7 */
	IMPLY+0x000, ABSY+0x200,		/* f8-f9 */
	IMPLY+0x000, IMPLY+0x000,		/* fa-fb */
	ABSXIND+0x200, ABSX+0x200,		/* fc-fd */
	ABSX+0x200, LONGX+0x300,		/* fe-ff */
};
	
