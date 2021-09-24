//
//  BUS.swift
//  Monitor
//
//  Created by Michał Kałużny on 08.01.18.
//

import Foundation

public protocol Bus {
    func read(from address: UInt16) throws -> UInt8
    func read(from address: UInt16) throws -> UInt16
    func write(to address: UInt16, value: UInt8) throws
}
