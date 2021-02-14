//
//  EmuMemoryModel.swift
//  ActiveGS
//
//  Created by Yoshi Sugawara on 2/4/21.
//

import Foundation

class EmuMemoryModel {
    let numToDisplayPerCell = 8
    let maxMemorySize = 128 * 1024
    
    private(set) var memory = EmuWrapper.memory()
    private(set) var referencedAddresses = [UInt16: [AddressedInstruction]]()
    
    var selectedAddress: Int?
    
    func offset(for indexPath: IndexPath) -> Int {
        let memMapSection = EmuMemoryMapSection(rawValue: indexPath.section)!
        return memMapSection.range.lowerBound + (indexPath.row * numToDisplayPerCell)
    }
    
    func hexStrings(for indexPath: IndexPath) -> [String] {
        let memMapSection = EmuMemoryMapSection(rawValue: indexPath.section)!
        let startIndex = offset(for: indexPath)
        let endIndex = min(startIndex + numToDisplayPerCell, memMapSection.range.upperBound)
        guard startIndex < memMapSection.range.upperBound,
              endIndex <= memMapSection.range.upperBound else {
            print("indexes are out of range of this section: startIndex=\(startIndex) endIndex=\(endIndex) section range: \(memMapSection.range.lowerBound) - \(memMapSection.range.upperBound)")
            return []
        }
        var row = [String]()
        guard let memory = memory else {
            return row
        }
        for i in startIndex..<endIndex {
            row.append(String(format: "%02X", memory[i]))
        }
        return row
    }
    
    func indexPath(for address: Int) -> IndexPath {
        let section = EmuMemoryMapSection.section(for: address)
        let offset = (address - section.range.lowerBound) / numToDisplayPerCell
        return IndexPath(item: offset, section: section.rawValue)
    }
    
    func getMemoryHexString(at address: Int) -> String {
        guard address > 0 && address < maxMemorySize else {
            print("Cannot get memory: address out of range \(address) > \(maxMemorySize)")
            return ""
        }
        guard let memory = memory else {
            return ""
        }
        let value = memory[address]
        return String(format: "%02X", value)
    }
    
    func getMemoryHeaderString(for address: Int) -> String {
        return String(format: "%02X", address)
    }
    
    func setMemory(at address:Int, value: UInt8) {
        guard address > 0 && address < maxMemorySize else {
            print("Cannot set memory: address out of range \(address) > \(maxMemorySize)")
            return
        }
        guard let memory = memory else {
            return
        }
        print("Setting memory address %04X to %02X",address,value)
        memory[address] = value
        refresh()
    }
    
    func getMemory(at address:Int) -> UInt8 {
        guard address > 0 && address < maxMemorySize else {
            print("Cannot get memory: address out of range \(address) > \(maxMemorySize)")
            return 0
        }
        guard let memory = memory else {
            return 0
        }
        return memory[address]
    }
    
    var memoryAsArray: [UInt8] {
        var buffer = [UInt8]()
        guard let memory = memory else {
            return buffer
        }
        for address in 0..<0x95ff {
            buffer.append(memory[address])
        }
        return buffer
    }
    
    lazy var interpretedInstructions: [AddressedInstruction] = {
        return interpreted()
    }()
    
    private func interpreted() -> [AddressedInstruction] {
        let interpreter = Debug6502Interpreter(memory: memoryAsArray)
        let instructions = interpreter.interpret()
        referencedAddresses = [UInt16: [AddressedInstruction]]()
        let updateBlock: ((UInt16, AddressedInstruction) -> Void) = { address, instruction in
            var instructions: [AddressedInstruction] = {
                if let existingInstructions = self.referencedAddresses[address] {
                    return existingInstructions
                }
                return [AddressedInstruction]()
            }()
            instructions.append(instruction)
            self.referencedAddresses[address] = instructions
        }
        for instruction in instructions {
            switch instruction.instruction.addressingMode {
            case .absolute(let address):
                updateBlock(address, instruction)
            case .zeroPage(let zeroPageAddress):
                updateBlock(UInt16(zeroPageAddress), instruction)
            case .indirect(let fromAddress):
                updateBlock(fromAddress, instruction)
            default:
                break
            }
        }
        return instructions
    }
    
    func refresh() {
        memory = EmuWrapper.memory()
        interpretedInstructions = interpreted()
    }
}
