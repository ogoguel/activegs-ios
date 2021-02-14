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
    
    enum UIState {
        case initial, startedNewSearch, isSearching, didSearch
    }
    
    enum SearchMode {
        case less, greater, same
        case equalTo(Int)
    }
    
    var uiState = UIState.initial
    
    func update(with memory: UnsafeMutablePointer<UInt8>) {
        for address in 0..<0x95ff {
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
        }
        matchedMemoryAddresses = newMatches
    }
    
    // start new search:
    // put all memory into matched
    //
    // set less than
    // run findNewMatches
}
