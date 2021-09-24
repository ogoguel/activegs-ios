//
//  CheatFinderManager.swift
//  ActiveGS
//
//  Created by Yoshi Sugawara on 1/29/21.
//

import Foundation

class CheatFinderManager {
    private(set) var matchedMemoryAddresses = [Int: UInt8]()
    var comparisonMemory = [UInt8]()
    
    private let dispatchQueue = DispatchQueue(label: "CheatFinderManager", qos: .background)
    var timer: DispatchSourceTimer?
    
    var savedMatches = [Int: (value: UInt8, enabled: Bool)]()
    
    enum UIState {
        case initial, startedNewSearch, isSearching, didSearch, showSaved
    }
    
    enum SearchMode {
        case less, greater, same
        case equalTo(Int)
    }
    
    var uiState = UIState.initial
    
    func start() {
        if timer == nil {
            timer = DispatchSource.makeTimerSource(queue: dispatchQueue)
            timer?.setEventHandler(handler: {
                self.updateMemoryWithCheats()
            })
            timer?.schedule(deadline: .now(), repeating: .seconds(1))
            timer?.resume()
        }
    }
    
    func updateMemoryWithCheats() {
        print("CheatFinderManager updating memory...")
        let memory = EmuWrapper.memory()
        for (address, memoryValue) in savedMatches {
            if let memory = memory, memoryValue.enabled {
                print("updating memory at \(String(format: "%05X",address)) to \(memoryValue.value)")
                memory[address] = memoryValue.value
            }
        }
    }
    
    func update(with memory: UnsafeMutablePointer<UInt8>) {
        for address in 0..<EmuMemoryModel.maxMemorySize {
            matchedMemoryAddresses[address] = memory[address]
        }
    }
    
    func findNewMatches(searchMode: SearchMode) {
        var newMatches = [Int: UInt8]()
        for (address, oldValue) in matchedMemoryAddresses {
            let newValue = comparisonMemory[address]
            var matched = false
            switch searchMode {
            case .less:
                matched = newValue < oldValue
            case .greater:
                matched = newValue > oldValue
            case .same:
                matched = newValue == oldValue
            case .equalTo(let searchValue):
                matched = newValue == searchValue
            }
            if matched {
                newMatches[address] = newValue
            }
//            // avoid having too many matches
//            if newMatches.count > 10000 {
//                break
//            }
        }
        matchedMemoryAddresses = newMatches
    }
    
}
