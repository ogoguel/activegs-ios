//
//  Debug6502Interpreter.swift
//  ActiveGS
//
//  Created by Yoshi Sugawara on 2/2/21.
//

import Foundation

public struct AddressedInstruction: CustomStringConvertible {
    let address: UInt16
    let instruction: Instruction

    public var description: String {
        return String(format: "$%04X : \(instruction.description)", address)
    }
}

enum InterpreterError: Error {
    case instructionError(String)
}

public class Debug6502Interpreter: Bus {
    public func read(from address: UInt16) throws -> UInt8 {
        if address >= memory.count {
            return 0
        }
        return memory[Int(address)]
    }
    
    public func read(from address: UInt16) throws -> UInt16 {
        if address + 1 >= memory.count {
            throw InterpreterError.instructionError("Tried to read beyond memory bus")
        }
        let low: UInt8 = try read(from: address)
        let high: UInt8 = try read(from: address + 1)
        return (UInt16(high) << 8 | UInt16(low))
    }
    
    public func write(to address: UInt16, value: UInt8) throws {
        // no-op, interpreter only
    }
    
    var memory: [UInt8]
    var cpu: CPU! = nil
    
    init(memory: [UInt8]) {
        self.memory = memory
        self.cpu = CPU(bus: self)
    }
    
    public func interpret(startAt address:UInt16 = 0) -> [AddressedInstruction] {
        cpu.PC = address
        var instructions = [AddressedInstruction]()
        while cpu.PC < memory.count {
            do {
                let instruction = try cpu.fetch()
                instructions.append(AddressedInstruction(address: cpu.PC, instruction: instruction))
                cpu.PC += instruction.size
            } catch Instruction.Error.unknownOpcode(let opcode) {
                print("Unknown opcode: \(opcode), skipping...")
                cpu.PC += 1
            } catch {
                print("Error when interpreting, stopping...")
                break
            }
        }
        return instructions
    }

}
