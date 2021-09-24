//
//  Instruction.swift
//  6502
//
//  Created by Michał Kałużny on 15/11/2016.
//
//

import Foundation

public struct Instruction {
    public let opcode: UInt8
    public let mnemonic: Mnemonic
    public let addressingMode: AddressingMode
    
    public enum Error: Swift.Error {
        case unknownOpcode(opcode: UInt8)
    }
    
    public init(from bus: Bus, PC: UInt16) throws {
        opcode = try bus.read(from: PC) as UInt8
        mnemonic = try Mnemonic(opcode)
        
        //MARK: Addressing Mode
        switch opcode {
        case 0x20: fallthrough
        case 0x0d: fallthrough
        case 0x0e: fallthrough
        case 0x2c: fallthrough
        case 0x2d: fallthrough
        case 0x2e: fallthrough
        case 0x4c: fallthrough
        case 0x4d: fallthrough
        case 0x4e: fallthrough
        case 0x6d: fallthrough
        case 0x7e: fallthrough
        case 0x8c: fallthrough
        case 0x8d: fallthrough
        case 0x8e: fallthrough
        case 0xac: fallthrough
        case 0xad: fallthrough
        case 0xae: fallthrough
        case 0xcc: fallthrough
        case 0xcd: fallthrough
        case 0xce: fallthrough
        case 0xec: fallthrough
        case 0xed: fallthrough
        case 0xee:
            let data = try bus.read(from: PC + 1) as UInt16
            addressingMode = .absolute(data: data)
        case 0x1d: fallthrough
        case 0x1e: fallthrough
        case 0x3d: fallthrough
        case 0x3e: fallthrough
        case 0x5d: fallthrough
        case 0x5e: fallthrough
        case 0x6e: fallthrough
        case 0x7d: fallthrough
        case 0x9d: fallthrough
        case 0xbc: fallthrough
        case 0xbd: fallthrough
        case 0xdd: fallthrough
        case 0xde: fallthrough
        case 0xfd: fallthrough
        case 0xfe:
            let data = try bus.read(from: PC + 1) as UInt16
            addressingMode = .absoluteIndexed(data: data, register: .X)
        case 0x19: fallthrough
        case 0x39: fallthrough
        case 0x59: fallthrough
        case 0x79: fallthrough
        case 0x99: fallthrough
        case 0xb9: fallthrough
        case 0xbe: fallthrough
        case 0xd9: fallthrough
        case 0xf9:
            let data = try bus.read(from: PC + 1) as UInt16
            addressingMode = .absoluteIndexed(data: data, register: .Y)
        case 0x0a: fallthrough
        case 0x2a: fallthrough
        case 0x4a: fallthrough
        case 0x6a:
            addressingMode = .accumulator
        case 0x09: fallthrough
        case 0x29: fallthrough
        case 0x49: fallthrough
        case 0x69: fallthrough
        case 0xa0: fallthrough
        case 0xa2: fallthrough
        case 0xa9: fallthrough
        case 0xc0: fallthrough
        case 0xc9: fallthrough
        case 0xe0: fallthrough
        case 0xe9:
            let data = try bus.read(from: PC + 1) as UInt8
            addressingMode = .immediate(data: data)
        case 0x0: fallthrough
        case 0x8: fallthrough
        case 0x18: fallthrough
        case 0x28: fallthrough
        case 0x38: fallthrough
        case 0x40: fallthrough
        case 0x48: fallthrough
        case 0x58: fallthrough
        case 0x60: fallthrough
        case 0x68: fallthrough
        case 0x78: fallthrough
        case 0x88: fallthrough
        case 0x98: fallthrough
        case 0x8a: fallthrough
        case 0x9a: fallthrough
        case 0xa8: fallthrough
        case 0xaa: fallthrough
        case 0xb8: fallthrough
        case 0xba: fallthrough
        case 0xc8: fallthrough
        case 0xca: fallthrough
        case 0xd8: fallthrough
        case 0xe8: fallthrough
        case 0xea: fallthrough
        case 0xf8:
            addressingMode = .implied
        case 0x1: fallthrough
        case 0x21: fallthrough
        case 0x41: fallthrough
        case 0x61: fallthrough
        case 0x81: fallthrough
        case 0xa1: fallthrough
        case 0xc1: fallthrough
        case 0xe1:
            let data = try bus.read(from: PC + 1) as UInt8
            addressingMode = .indexedIndirect(data: data, register: .X)
        case 0x6c:
            let data = try bus.read(from: PC + 1) as UInt16
            addressingMode = .indirect(data: data)
        case 0x11: fallthrough
        case 0x31: fallthrough
        case 0x51: fallthrough
        case 0x71: fallthrough
        case 0x91: fallthrough
        case 0xb1: fallthrough
        case 0xd1: fallthrough
        case 0xf1:
            let data = try bus.read(from: PC + 1) as UInt8
            addressingMode = .indirectIndexed(data: data, register: .Y)
        case 0x10: fallthrough
        case 0x30: fallthrough
        case 0x50: fallthrough
        case 0x70: fallthrough
        case 0x90: fallthrough
        case 0xb0: fallthrough
        case 0xd0: fallthrough
        case 0xf0:
            let data = try bus.read(from: PC + 1) as UInt8
            addressingMode = .relative(data: Int8(bitPattern: data))
        case 0x5: fallthrough
        case 0x6: fallthrough
        case 0x24: fallthrough
        case 0x25: fallthrough
        case 0x26: fallthrough
        case 0x45: fallthrough
        case 0x46: fallthrough
        case 0x65: fallthrough
        case 0x66: fallthrough
        case 0x84: fallthrough
        case 0x85: fallthrough
        case 0x86: fallthrough
        case 0xa4: fallthrough
        case 0xa5: fallthrough
        case 0xa6: fallthrough
        case 0xc4: fallthrough
        case 0xc5: fallthrough
        case 0xc6: fallthrough
        case 0xe4: fallthrough
        case 0xe5: fallthrough
        case 0xe6:
            let data = try bus.read(from: PC + 1) as UInt8
            addressingMode = .zeroPage(data: data)
        case 0x15: fallthrough
        case 0x16: fallthrough
        case 0x35: fallthrough
        case 0x36: fallthrough
        case 0x55: fallthrough
        case 0x56: fallthrough
        case 0x75: fallthrough
        case 0x76: fallthrough
        case 0x94: fallthrough
        case 0x95: fallthrough
        case 0xb4: fallthrough
        case 0xb5: fallthrough
        case 0xd5: fallthrough
        case 0xd6: fallthrough
        case 0xf5: fallthrough
        case 0xf6:
            let data = try bus.read(from: PC + 1) as UInt8
            addressingMode = .zeroPageIndexed(data: data, register: .X)
        case 0x96: fallthrough
        case 0xb6:
            let data = try bus.read(from: PC + 1) as UInt8
            addressingMode = .zeroPageIndexed(data: data, register: .Y)
        default:
            throw Error.unknownOpcode(opcode: opcode)
        }
    }
    
    public var size: UInt16 {
        return addressingMode.dataSize + 1
    }
}

extension Instruction: CustomStringConvertible {
    public var description: String {
        return "\(mnemonic) \(addressingMode)"
    }
}
