//
//  Flags.swift
//  6502
//
//  Created by Michał Kałużny on 17/11/2016.
//
//

import Foundation

extension CPU {
    struct Flags: OptionSet {
        let rawValue: UInt8
        
        static let negative = Flags(rawValue: 1 << 7)
        static let overflow = Flags(rawValue: 1 << 6)
        static let always = Flags(rawValue: 1 << 5)
        static let `break` = Flags(rawValue: 1 << 4)
        static let decimal = Flags(rawValue: 1 << 3)
        static let interrupt = Flags(rawValue: 1 << 2)
        static let zero = Flags(rawValue: 1 << 1)
        static let carry = Flags(rawValue: 1 << 0)
        
        init(rawValue: UInt8) {
            self.rawValue = rawValue | 0b0010_0000
        }
    }
    
    internal func recalculateStatus(flags: Flags, for value: UInt8) {
        if flags.contains(.carry) {
            calculateCarry(value: value)
        }
        
        if flags.contains(.overflow) {
            calculateCarry(value: value)
        }
        
        if flags.contains(.negative) {
            calculateSign(value: value)
        }
        
        if flags.contains(.zero) {
            calculateZero(value: value)
        }
    }
    
    private func calculateCarry(value: UInt8) {
        
    }
    
    private func calculateOverflow(value: UInt8) {
        
    }
    
    private func calculateSign(value: UInt8) {
        let bit = (value & (1 << 7)) != 0
        
        if bit {
            Status.insert(.negative)
        } else {
            Status.remove(.negative)
        }
    }
    
    private func calculateZero(value: UInt8) {
        if value != 0 {
            Status.remove(.zero)
        } else {
            Status.insert(.zero)
        }
    }
}
