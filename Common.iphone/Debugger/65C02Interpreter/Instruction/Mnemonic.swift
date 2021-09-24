//
//  Opcode.swift
//  6502
//
//  Created by Michał Kałużny on 16/11/2016.
//
//

import Foundation

private let mnemonicTable: [Instruction.Mnemonic?] = [
    /*        |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |  A  |  B  |  C  |  D  |  E  |  F  |      */
    /* 0 */    .BRK, .ORA,  nil, .SLO,  nil, .ORA, .ASL, .SLO, .PHP, .ORA, .ASL,  nil,  nil, .ORA, .ASL, .SLO, /* 0 */
    /* 1 */    .BPL, .ORA,  nil, .SLO,  nil, .ORA, .ASL, .SLO, .CLC, .ORA,  nil, .SLO,  nil, .ORA, .ASL, .SLO, /* 1 */
    /* 2 */    .JSR, .AND,  nil, .RLA, .BIT, .AND, .ROL, .RLA, .PLP, .AND, .ROL,  nil, .BIT, .AND, .ROL, .RLA, /* 2 */
    /* 3 */    .BMI, .AND,  nil, .RLA,  nil, .AND, .ROL, .RLA, .SEC, .AND,  nil, .RLA,  nil, .AND, .ROL, .RLA, /* 3 */
    /* 4 */    .RTI, .EOR,  nil, .SRE,  nil, .EOR, .LSR, .SRE, .PHA, .EOR, .LSR,  nil, .JMP, .EOR, .LSR, .SRE, /* 4 */
    /* 5 */    .BVC, .EOR,  nil, .SRE,  nil, .EOR, .LSR, .SRE, .CLI, .EOR,  nil, .SRE,  nil, .EOR, .LSR, .SRE, /* 5 */
    /* 6 */    .RTS, .ADC,  nil, .RRA,  nil, .ADC, .ROR, .RRA, .PLA, .ADC, .ROR,  nil, .JMP, .ADC, .ROR, .RRA, /* 6 */
    /* 7 */    .BVS, .ADC,  nil, .RRA,  nil, .ADC, .ROR, .RRA, .SEI, .ADC,  nil, .RRA,  nil, .ADC, .ROR, .RRA, /* 7 */
    /* 8 */     nil, .STA,  nil, .SAX, .STY, .STA, .STX, .SAX, .DEY,  nil, .TXA,  nil, .STY, .STA, .STX, .SAX, /* 8 */
    /* 9 */    .BCC, .STA,  nil,  nil, .STY, .STA, .STX, .SAX, .TYA, .STA, .TXS,  nil,  nil, .STA,  nil,  nil, /* 9 */
    /* A */    .LDY, .LDA, .LDX, .LAX, .LDY, .LDA, .LDX, .LAX, .TAY, .LDA, .TAX,  nil, .LDY, .LDA, .LDX, .LAX, /* A */
    /* B */    .BCS, .LDA,  nil, .LAX, .LDY, .LDA, .LDX, .LAX, .CLV, .LDA, .TSX, .LAX, .LDY, .LDA, .LDX, .LAX, /* B */
    /* C */    .CPY, .CMP,  nil, .DCP, .CPY, .CMP, .DEC, .DCP, .INY, .CMP, .DEX,  nil, .CPY, .CMP, .DEC, .DCP, /* C */
    /* D */    .BNE, .CMP,  nil, .DCP,  nil, .CMP, .DEC, .DCP, .CLD, .CMP,  nil, .DCP,  nil, .CMP, .DEC, .DCP, /* D */
    /* E */    .CPX, .SBC,  nil, .ISB, .CPX, .SBC, .INC, .ISB, .INX, .SBC, .NOP, .SBC, .CPX, .SBC, .INC, .ISB, /* E */
    /* F */    .BEQ, .SBC,  nil, .ISB,  nil, .SBC, .INC, .ISB, .SED, .SBC,  nil, .ISB,  nil, .SBC, .INC, .ISB  /* F */
]

public extension Instruction {
    enum Mnemonic: String {
        
        enum Error: Swift.Error {
            case unknownMnemonic(opcode: String)
        }
        
        init(_ opcode: UInt8) throws {
            guard let mnemonic = mnemonicTable[Int(opcode)] else {
                throw Instruction.Error.unknownOpcode(opcode: opcode)
            }
            self = mnemonic
        }
        
        init(_ string: String) throws {
            guard let value = Mnemonic(rawValue: string) else {
                throw Error.unknownMnemonic(opcode: string)
            }
            self = value
        }
        
        case ADC
        case AND
        case ASL
        case BCC
        case BCS
        case BEQ
        case BIT
        case BMI
        case BNE
        case BPL
        case BRK
        case BVC
        case BVS
        case CLC
        case CLD
        case CLI
        case CLV
        case CMP
        case CPX
        case CPY
        case DCP
        case DEC
        case DEX
        case DEY
        case EOR
        case INC
        case INX
        case INY
        case ISB
        case JMP
        case JSR
        case LAX
        case LDA
        case LDX
        case LDY
        case LSR
        case NOP
        case ORA
        case PHA
        case PHP
        case PLA
        case PLP
        case RLA
        case ROL
        case ROR
        case RRA
        case RTI
        case RTS
        case SBC
        case SEC
        case SED
        case SEI
        case SLO
        case SRE
        case SAX
        case STA
        case STX
        case STY
        case TAX
        case TAY
        case TSX
        case TXA
        case TXS
        case TYA
    }
}

extension Instruction.Mnemonic: CustomStringConvertible {
    public var description: String {
        return "\(rawValue)"
    }
}
