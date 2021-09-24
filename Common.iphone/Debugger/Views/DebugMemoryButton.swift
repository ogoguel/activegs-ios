//
//  DebugMemoryButton.swift
//  ActiveGS
//
//  Created by Yoshi Sugawara on 2/4/21.
//

import Foundation

class ToggleButton: UIButton {
    var onTapped: ((Bool) -> Void)?

    convenience init() {
        self.init(type: .custom)
        addTarget(self, action: #selector(tapped(_:)), for: .touchUpInside)
    }
    
    @objc func tapped(_ sender: UIButton) {
        isSelected.toggle()
        onTapped?(isSelected)
    }
}

class DebugMemoryButton: ToggleButton {
    override open var isSelected: Bool {
        didSet {
            backgroundColor = isSelected ? .white : .clear
        }
    }
}

class DebugPauseResumeButton: ToggleButton {
    override open var isSelected: Bool {
        didSet {
            backgroundColor = isSelected ? .red : .clear
        }
    }
    
    convenience init() {
        self.init(type: .custom)
        addTarget(self, action: #selector(tapped(_:)), for: .touchUpInside)
        setTitle("RESUME", for: .normal)
        setTitleColor(.green, for: .normal)
        setTitle("PAUSE", for: .selected)
        setTitleColor(.white, for: .selected)
        titleLabel?.font = UIFont(name: "Print Char 21", size: 11)
    }
}
