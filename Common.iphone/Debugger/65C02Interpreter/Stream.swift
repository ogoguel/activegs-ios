//
//  Stream.swift
//  6502
//
//  Created by Michał Kałużny on 15/11/2016.
//
//

import Foundation

protocol RawStreamReadable {}

extension UInt8: RawStreamReadable {}
extension UInt16: RawStreamReadable {}

extension InputStream: Stream {
    func read(_ length: Int) -> [UInt8] {
        var buffer = Array<UInt8>(repeating: 0, count: length)
        _ = read(&buffer, maxLength: length)
        return buffer
    }
}


class MemoryStream: Stream {
    let storage: [UInt8]
    var position: Int = 0
    
    init(storage: [UInt8]) {
        self.storage = storage
    }
    
    open func read(_ length: Int) -> [UInt8] {
        if position+length > storage.count {
            return [UInt8](repeating: 0, count: length)
        }
        
        let slice = storage[position..<position+length]
        position += length
        return Array(slice)
    }
}

protocol Stream {
    func read(_ length: Int) -> [UInt8]
}

extension Stream {
    func read<T: RawStreamReadable>() -> T {
        let size = MemoryLayout<T>.size
        let bytes = read(size)
        let data = Data(bytes: bytes)
        let value: T = data.withUnsafeBytes { $0.pointee }
        return value
    }
    
    func skip(_ count: Int) {
        _ = read(count)
    }
}
