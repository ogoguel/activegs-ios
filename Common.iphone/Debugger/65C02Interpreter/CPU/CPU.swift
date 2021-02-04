//
//  CPU.swift
//  6502
//
//  Created by Michał Kałużny on 15/11/2016.
//
//

import Foundation

public class CPU {
    static let nmiVector: UInt16 = 0xFFFA
    static let resetVector: UInt16 = 0xFFFC
    static let interruptVector: UInt16 = 0xFFFE
    
    enum Error: Swift.Error {
        case unimplementedInstruction(instruction: Instruction)
    }
    
    public enum Register {
        case X
        case Y
    }
    
    //MARK: Registers
    public var PC: UInt16 = 0
    var  A: UInt8 = 0
    var  X: UInt8 = 0
    var  Y: UInt8 = 0
    var  SP: UInt8 = 0
    var  Status: Flags = []
    
    let bus: Bus
   
    let breakpoints: [UInt16] = [0x378A]
    
    public init(bus: Bus) {
        self.bus = bus
    }
    
    public func reset() throws {
        PC = try bus.read(from: CPU.resetVector)
        A = 0
        X = 0
        Y = 0
        SP = 0xFF
        Status = [.break, .interrupt, .always]
    }
    
    public func step() throws {
        if breakpoints.contains(PC) {
            print("Breakpoint!")
        }
        print(self)
        let instruction = try fetch()
        print(instruction)
        try execute(instruction: instruction)
        print(self)
        print("=============================")
    }
    
    public func run() throws {
        while true {
            try step()
        }
    }
    
    public func fetch() throws -> Instruction {
        return try Instruction(from: bus, PC: PC)
    }
    
    subscript(register: Register) -> UInt8  {
        switch register {
        case .X:
            return X
        case .Y:
            return Y
        }
    }
}

extension CPU: CustomStringConvertible {
    public var description: String {
        return "PC: \(PC.hex) SP: \(SP.hex) A: \(A.hex) X: \(X.hex) Y: \(Y.hex) \nFlags: \(Status.rawValue.bin)"
    }
}

extension FixedWidthInteger {
    public var hex: String {
        return String(self, radix: 16, uppercase: true)
    }
    
    var bin: String {
        return String(self, radix: 2, uppercase: true)
    }
}



