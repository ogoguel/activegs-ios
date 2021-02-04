//
//  CPU+Stack.swift
//  MOS6502PackageDescription
//
//  Created by Michał Kałużny on 09.01.18.
//

import Foundation

extension CPU {
    static let stackPointerBase: UInt16 = 0x100

    internal func pop() throws -> UInt8 {
        SP += 1
        
        let value: UInt8 = try bus.read(from: UInt16(SP) + CPU.stackPointerBase)
        
        print("Popping value: \(value.hex)")
        
        return value
    }
    
    internal func pop() throws -> UInt16 {
        let low: UInt8 = try pop()
        let high: UInt8 = try pop()
        
        return UInt16(low) | UInt16(high) << 8
    }
    
    internal func push(_ value: UInt8) throws {
        try bus.write(to: UInt16(SP) + CPU.stackPointerBase, value: value)
       
        print("Pushing value: \(value.hex)")

        SP -= 1
    }
    
    internal func push(_ value: UInt16) throws {
        let low: UInt8 = UInt8(value & 0xFF)
        let high: UInt8 = UInt8(value >> 8) & 0xFF
        
        try push(high)
        try push(low)
    }
}
