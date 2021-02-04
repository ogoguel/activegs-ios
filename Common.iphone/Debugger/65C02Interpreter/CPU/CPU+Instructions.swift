//
//  CPU+Instructions.swift
//  MOS6502PackageDescription
//
//  Created by Michał Kałużny on 09.01.18.
//

import Foundation

extension CPU {
    internal func execute(instruction: Instruction) throws {
        switch instruction.mnemonic {
        //MARK: Branch Instructions
        case .BNE:
            PC += instruction.size
            
            if Status.contains(.zero) != true {
                PC = try instruction.addressingMode.value(with: self, bus: bus)
            }
        case .BEQ:
            PC += instruction.size
            
            if Status.contains(.zero) == true {
                PC = try instruction.addressingMode.value(with: self, bus: bus)
            }
        case .BPL:
            PC += instruction.size
            
            if Status.contains(.negative) != true {
                PC = try instruction.addressingMode.value(with: self, bus: bus)
            }
        case .BCC:
            PC += instruction.size

            if Status.contains(.carry) != true {
                PC = try instruction.addressingMode.value(with: self, bus: bus)
            }
        case .BCS:
            PC += instruction.size

            if Status.contains(.carry) == true {
                PC = try instruction.addressingMode.value(with: self, bus: bus)
            }
        case .BMI:
            PC += instruction.size
            
            if Status.contains(.negative) == true {
                PC = try instruction.addressingMode.value(with: self, bus: bus)
            }
        case .BVC:
            PC += instruction.size
            
            if Status.contains(.overflow) != true {
                PC = try instruction.addressingMode.value(with: self, bus: bus)
            }
        case .BVS:
            PC += instruction.size
            
            if Status.contains(.overflow) == true {
                PC = try instruction.addressingMode.value(with: self, bus: bus)
            }
        //MARK: Stack Operations
        case .TXS:
            SP = X
            PC += instruction.size
        case .TSX:
            X = SP
            recalculateStatus(flags: [.zero, .negative], for: X)
            PC += instruction.size
        case .PLP:
            let value = try pop() as UInt8
            Status = Flags(rawValue: value)
            PC += instruction.size
        case .PHP:
            try push(Status.rawValue | 0b0001_0000)
            PC += instruction.size
        case .PHA:
            try push(A)
            PC += instruction.size
        case .PLA:
            A = try pop()
            recalculateStatus(flags: [.zero, .negative], for: A)
            PC += instruction.size
        case .JSR:
            // JSR writes the *address of the last byte* of the instruction.
            try push(PC + instruction.size - 1)
            PC = try instruction.addressingMode.value(with: self, bus: bus)
        case .RTS:
            PC += instruction.size
            PC = try pop()
            // We would land on the *last byte* of the JSR, just before the jump, we need to advance the PC
            PC += 1
        //MARK: Register Operations
        case .INX:
            X = X &+ 1
            recalculateStatus(flags: [.zero, .negative], for: X)
            PC += instruction.size
        case .DEX:
            X = X &- 1
            recalculateStatus(flags: [.zero, .negative], for: X)
            PC += instruction.size
        case .INY:
            Y = Y &+ 1
            recalculateStatus(flags: [.zero, .negative], for: Y)
            PC += instruction.size
        case .DEY:
            Y = Y &- 1
            recalculateStatus(flags: [.zero, .negative], for: Y)
            PC += instruction.size
        case .TYA:
            A = Y
            recalculateStatus(flags: [.negative, .zero], for: A)
            PC += instruction.size
        case .TXA:
            A = X
            recalculateStatus(flags: [.negative, .zero], for: A)
            PC += instruction.size
        case .TAX:
            X = A
            recalculateStatus(flags: [.negative, .zero], for: X)
            PC += instruction.size
        case .TAY:
            Y = A
            recalculateStatus(flags: [.negative, .zero], for: Y)
            PC += instruction.size
        //MARK: Flags Operations:
        case .SEI:
            Status.insert(.interrupt)
            PC += instruction.size
        case .CLD:
            Status.remove(.decimal)
            PC += instruction.size
        case .CLC:
            Status.remove(.carry)
            PC += instruction.size
        //MARK: Interrupt Operations:
        case .BRK:
            try push(PC)
            try push(Status.rawValue)
            
            PC = try bus.read(from: CPU.interruptVector)
            
            Status.insert(.break)
        //MARK: Other Instructions, clean me up please.
        case .LDA:
            A = try instruction.addressingMode.value(with: self, bus: bus)
            recalculateStatus(flags: [.zero, .negative], for: A)
            PC += instruction.size
        case .STA:
            let address: UInt16 = try instruction.addressingMode.value(with: self, bus: bus)
            try bus.write(to: address, value: A)
            PC += instruction.size
        case .LDX:
            X = try instruction.addressingMode.value(with: self, bus: bus)
            recalculateStatus(flags: [.zero, .negative], for: X)
            PC += instruction.size
        case .STX:
            let address = try instruction.addressingMode.value(with: self, bus: bus) as UInt16
            try bus.write(to: address, value: X)
            PC += instruction.size
        case .LDY:
            Y = try instruction.addressingMode.value(with: self, bus: bus)
            recalculateStatus(flags: [.zero, .negative], for: Y)
            PC += instruction.size
        case .STY:
            let address = try instruction.addressingMode.value(with: self, bus: bus) as UInt16
            try bus.write(to: address, value: Y)
            PC += instruction.size
        case .JMP:
            PC = try instruction.addressingMode.value(with: self, bus: bus)
        case .CMP:
            let value = try instruction.addressingMode.value(with: self, bus: bus) as UInt8
            
            if A >= value {
                Status.insert(.carry)
            } else {
                Status.remove(.carry)
            }
            
            let result = A &- value
            recalculateStatus(flags: [.negative, .zero], for: result)
            
            PC += instruction.size
        case .CPY:
            let value = try instruction.addressingMode.value(with: self, bus: bus) as UInt8
            
            if Y >= value {
                Status.insert(.carry)
            } else {
                Status.remove(.carry)
            }
            
            let result = Y &- value
            recalculateStatus(flags: [.negative, .zero], for: result)
            
            PC += instruction.size
        case .CPX:
            let value = try instruction.addressingMode.value(with: self, bus: bus) as UInt8
            
            if X >= value {
                Status.insert(.carry)
            } else {
                Status.remove(.carry)
            }
            
            let result = X &- value
            recalculateStatus(flags: [.negative, .zero], for: result)
            
            PC += instruction.size
        case .ADC:
            let value = try instruction.addressingMode.value(with: self, bus: bus) as UInt8
            let carry: UInt8 = Status.contains(.carry) ? 1 : 0
            
            let result = [A, value, carry].map(UInt16.init).reduce(0, +)
            if result > UInt8.max {
                Status.insert(.carry)
            } else {
                Status.remove(.carry)
            }
            
            let final: UInt8 = UInt8(result & 0xFF)
            
            let overflow = (((A ^ final) & 0x80) != 0) && (((UInt16(A) ^ result) & 0x80) == 0)
            if overflow {
                Status.insert(.overflow)
            } else {
                Status.remove(.overflow)
            }
            
            A = final
            
            recalculateStatus(flags: [.negative, .zero], for: A)
            PC += instruction.size
        case .EOR:
            let value = try instruction.addressingMode.value(with: self, bus: bus) as UInt8
            A = A ^ value
            
            recalculateStatus(flags: [.negative, .zero], for: A)
            PC += instruction.size
        case .NOP:
            PC += instruction.size
        case _:
            throw Error.unimplementedInstruction(instruction: instruction)
        }
    }
}
