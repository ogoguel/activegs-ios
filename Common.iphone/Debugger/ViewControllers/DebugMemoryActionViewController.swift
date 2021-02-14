//
//  DebugMemoryActionViewController.swift
//  ActiveGS
//
//  Created by Yoshi Sugawara on 2/4/21.
//

import Foundation

protocol DebugMemoryActionViewControllerDelegate: class {
    func jump(to address: Int)
    func memoryHex(at address: Int) -> String
    func updateMemory(at address: Int, with memory:UInt8)
    var memory: UnsafeMutablePointer<UInt8>? { get }
    var selectedAddress: Int? { get }
    var referencedMemoryAddresses: [UInt16: [AddressedInstruction]] { get }
}

class DebugMemoryActionViewController: UIViewController {
    
    enum Mode {
        case jumpToAddress, changeMemory, cheat, screen
    }
    
    var mode: Mode = .jumpToAddress
    var cheatFinder = CheatFinderManager()
    
    var matchedInstructions = [AddressedInstruction]()
    
    weak var delegate:DebugMemoryActionViewControllerDelegate?
    
    let segmentedControl: UISegmentedControl = {
        let control = UISegmentedControl(items: ["Jump", "Edit", "Cheat", "Screen"])
        control.translatesAutoresizingMaskIntoConstraints = false
        control.tintColor = .orange
        control.selectedSegmentIndex = 0
        control.addTarget(self, action: #selector(segmentedControlChanged(_:)), for: .valueChanged)
        control.setTitleTextAttributes([NSAttributedString.Key.font: UIFont(name: "Print Char 21", size: 14)!], for: .normal)
        return control
    }()
    
    let memoryField: UITextField = {
        let field = UITextField(frame: .zero)
        field.translatesAutoresizingMaskIntoConstraints = false
        field.font = UIFont(name: "Print Char 21", size: 14)
        field.isUserInteractionEnabled = false
        field.text = ""
        field.textColor = .cyan
        field.layer.borderWidth = 1.0
        field.layer.borderColor = UIColor.cyan.cgColor
        field.textAlignment = .center
        field.widthAnchor.constraint(equalToConstant: 80).isActive = true
        field.heightAnchor.constraint(equalToConstant: 40).isActive = true
        return field
    }()
    
    let updateMemoryButton: UIButton = {
        let button = UIButton(type: .custom)
        button.titleLabel?.font = UIFont(name: "Print Char 21", size: 12)
        button.setTitle("Update", for: .normal)
        button.layer.borderWidth = 1
        button.layer.borderColor = UIColor.orange.cgColor
        button.addTarget(self, action: #selector(updateButtonPressed(_:)), for: .touchUpInside)
        return button
    }()
    
    let resetMemoryButton: UIButton = {
        let button = UIButton(type: .custom)
        button.titleLabel?.font = UIFont(name: "Print Char 21", size: 12)
        button.setTitle("Reset", for: .normal)
        button.layer.borderWidth = 1
        button.layer.borderColor = UIColor.orange.cgColor
        return button
    }()
    
    lazy var editFieldsStackView: UIStackView = {
        let stackView = UIStackView(arrangedSubviews: [memoryField, updateMemoryButton, resetMemoryButton])
        stackView.translatesAutoresizingMaskIntoConstraints = false
        stackView.axis = .horizontal
        stackView.spacing = 4
        return stackView
    }()
    
    let keyboardModel: EmulatorKeyboardViewModel = {
        let model = EmulatorKeyboardViewModel(keys:
                                                [
                                                    [
                                                        AppleIIKey(label: "0", code: 0),
                                                        AppleIIKey(label: "1", code: 1),
                                                        AppleIIKey(label: "2", code: 2),
                                                        AppleIIKey(label: "3", code: 3)
                                                    ],
                                                    [
                                                        AppleIIKey(label: "4", code: 4),
                                                        AppleIIKey(label: "5", code: 5),
                                                        AppleIIKey(label: "6", code: 6),
                                                        AppleIIKey(label: "7", code: 7)
                                                    ],
                                                    [
                                                        AppleIIKey(label: "8", code: 8),
                                                        AppleIIKey(label: "9", code: 9),
                                                        AppleIIKey(label: "A", code: 10),
                                                        AppleIIKey(label: "B", code: 11)
                                                    ],
                                                    [
                                                        AppleIIKey(label: "C", code: 12),
                                                        AppleIIKey(label: "D", code: 13),
                                                        AppleIIKey(label: "E", code: 14),
                                                        AppleIIKey(label: "F", code: 15)
                                                    ]
                                                ]
        )
        model.isDraggable = false
        return model
    }()
    
    lazy var keyboardView: EmulatorKeyboardView = {
        let view = keyboardModel.createView()
        view.translatesAutoresizingMaskIntoConstraints = false
        return view
    }()
    
    let titleLabel: UILabel = {
        let label = UILabel(frame: .zero)
        label.font = UIFont(name: "Print Char 21", size: 14)
        label.text = "Memory Tools"
        label.translatesAutoresizingMaskIntoConstraints = false
        label.textColor = .orange
        label.heightAnchor.constraint(equalToConstant: 20).isActive = true
        return label
    }()
    
    let cheatFinderNewSearchButton: UIButton = {
        let button = UIButton(type: .custom)
        button.titleLabel?.font = UIFont(name: "Print Char 21", size: 12)
        button.setTitle("New Search", for: .normal)
        button.layer.borderWidth = 1
        button.layer.borderColor = UIColor.purple.cgColor
        button.addTarget(self, action: #selector(cheatFinderNewSearchButtonPressed(_:)), for: .touchUpInside)
        return button
    }()
    
    let cheatFinderContinueSearchButton: UIButton = {
        let button = UIButton(type: .custom)
        button.titleLabel?.font = UIFont(name: "Print Char 21", size: 12)
        button.setTitle("Continue Search", for: .normal)
        button.layer.borderWidth = 1
        button.layer.borderColor = UIColor.red.cgColor
        button.addTarget(self, action: #selector(cheatFinderContinueSearchButtonPressed(_:)), for: .touchUpInside)
        return button
    }()
    
    lazy var cheatFinderInitialActionsStackView: UIStackView = {
        let stackView = UIStackView(arrangedSubviews: [cheatFinderNewSearchButton, cheatFinderContinueSearchButton, cheatFinderSearchLessButton, cheatFinderSearchGreaterButton, cheatFinderSearchEqualButton])
        stackView.translatesAutoresizingMaskIntoConstraints = false
        stackView.axis = .horizontal
        stackView.spacing = 4
        return stackView
    }()
    
    let cheatFinderSearchLessButton: UIButton = {
        let button = UIButton(type: .custom)
        button.titleLabel?.font = UIFont(name: "Print Char 21", size: 12)
        button.setTitle("LT", for: .normal)
        button.layer.borderWidth = 1
        button.layer.borderColor = UIColor.purple.cgColor
        button.addTarget(self, action: #selector(cheatFinderSearchButtonPressed(_:)), for: .touchUpInside)
        button.tag = 0
        button.translatesAutoresizingMaskIntoConstraints = false
        button.widthAnchor.constraint(equalToConstant: 30).isActive = true
        return button
    }()
    
    let cheatFinderSearchGreaterButton: UIButton = {
        let button = UIButton(type: .custom)
        button.titleLabel?.font = UIFont(name: "Print Char 21", size: 12)
        button.setTitle("GT", for: .normal)
        button.layer.borderWidth = 1
        button.layer.borderColor = UIColor.purple.cgColor
        button.addTarget(self, action: #selector(cheatFinderSearchButtonPressed(_:)), for: .touchUpInside)
        button.tag = 1
        button.translatesAutoresizingMaskIntoConstraints = false
        button.widthAnchor.constraint(equalToConstant: 30).isActive = true
        return button
    }()

    let cheatFinderSearchEqualButton: UIButton = {
        let button = UIButton(type: .custom)
        button.titleLabel?.font = UIFont(name: "Print Char 21", size: 12)
        button.setTitle("EQ", for: .normal)
        button.layer.borderWidth = 1
        button.layer.borderColor = UIColor.purple.cgColor
        button.addTarget(self, action: #selector(cheatFinderSearchButtonPressed(_:)), for: .touchUpInside)
        button.tag = 2
        button.translatesAutoresizingMaskIntoConstraints = false
        button.widthAnchor.constraint(equalToConstant: 30).isActive = true
        return button
    }()
    
    let findCodeButton: DebugMemoryButton = {
        let button = DebugMemoryButton()
        button.titleLabel?.font = UIFont(name: "Print Char 21", size: 9)
        button.setTitle("Find in Code >", for: .normal)
        button.setTitle("Cancel", for: .selected)
        button.setTitleColor(.red, for: .selected)
        button.translatesAutoresizingMaskIntoConstraints = false
        return button
    }()

//    lazy var cheatFinderSearchStackView: UIStackView = {
//        let stackView = UIStackView(arrangedSubviews: [cheatFinderSearchLessButton, cheatFinderSearchGreaterButton])
//        stackView.translatesAutoresizingMaskIntoConstraints = false
//        stackView.axis = .horizontal
//        stackView.spacing = 4
//        return stackView
//    }()

    
    let cheatFinderPromptLabel: UILabel = {
        let label = UILabel(frame: .zero)
        label.font = UIFont(name: "Print Char 21", size: 11)
        label.text = "Start a new search to begin!"
        label.translatesAutoresizingMaskIntoConstraints = false
        label.textColor = .red
        label.textAlignment = .center
        label.numberOfLines = 0
        return label
    }()
    
    lazy var cheatFinderMatchesTableView: UITableView = {
        let view = UITableView(frame: .zero)
        view.translatesAutoresizingMaskIntoConstraints = false
        view.backgroundColor = .clear
        view.dataSource = self
        view.delegate = self
        view.register(UITableViewCell.self, forCellReuseIdentifier: "CheatFinderMatchCell")
        return view
    }()
    
    lazy var emulatorScreenView: UIImageView = {
        let screenView = UIImageView(frame: .zero)
        screenView.translatesAutoresizingMaskIntoConstraints = false
        return screenView
    }()
    
    @objc func cheatFinderNewSearchButtonPressed(_ sender: UIButton) {
        if let memory = delegate?.memory {
            cheatFinder.update(with: memory)
        }
        cheatFinder.uiState = .startedNewSearch
        cheatFinderUpdateUI()
    }
    
    @objc func cheatFinderContinueSearchButtonPressed(_ sender: UIButton) {
        cheatFinder.uiState = .isSearching
        cheatFinderUpdateUI()
    }
    
    func cheatFinderSetupView() {
        view.addSubview(cheatFinderInitialActionsStackView)
//        view.addSubview(cheatFinderSearchStackView)
        view.addSubview(cheatFinderPromptLabel)
        view.addSubview(cheatFinderMatchesTableView)
        cheatFinderInitialActionsStackView.topAnchor.constraint(equalTo: segmentedControl.bottomAnchor, constant: 8).isActive = true
        cheatFinderInitialActionsStackView.centerXAnchor.constraint(equalTo: segmentedControl.centerXAnchor).isActive = true
//        cheatFinderInitialActionsStackView.leadingAnchor.constraint(equalTo: view.leadingAnchor, constant: 16).isActive = true
//        cheatFinderInitialActionsStackView.trailingAnchor.constraint(equalTo: view.trailingAnchor, constant: -16).isActive = true

//        cheatFinderSearchStackView.topAnchor.constraint(equalTo: cheatFinderInitialActionsStackView.bottomAnchor, constant: 8).isActive = true
//        cheatFinderSearchStackView.centerXAnchor.constraint(equalTo: segmentedControl.centerXAnchor).isActive = true
        cheatFinderPromptLabel.topAnchor.constraint(equalTo: cheatFinderInitialActionsStackView.bottomAnchor, constant: 16).isActive = true
        cheatFinderPromptLabel.leadingAnchor.constraint(equalTo: view.leadingAnchor, constant: 48).isActive = true
        cheatFinderPromptLabel.trailingAnchor.constraint(equalTo: view.trailingAnchor, constant: -48).isActive = true
        cheatFinderMatchesTableView.topAnchor.constraint(equalTo: cheatFinderPromptLabel.bottomAnchor, constant: 4).isActive = true
        cheatFinderMatchesTableView.leadingAnchor.constraint(equalTo: view.leadingAnchor, constant: 24).isActive = true
        cheatFinderMatchesTableView.trailingAnchor.constraint(equalTo: view.trailingAnchor, constant: -24).isActive = true
        cheatFinderMatchesTableView.bottomAnchor.constraint(equalTo: view.safeAreaLayoutGuide.bottomAnchor, constant: 8).isActive = true
        [cheatFinderInitialActionsStackView, cheatFinderPromptLabel, cheatFinderMatchesTableView].forEach{ $0.isHidden = true }
    }

    func cheatFinderHide() {
        [cheatFinderInitialActionsStackView, cheatFinderPromptLabel, cheatFinderMatchesTableView, cheatFinderSearchLessButton, cheatFinderSearchGreaterButton, cheatFinderNewSearchButton, cheatFinderContinueSearchButton, cheatFinderSearchEqualButton].forEach{ $0.isHidden = true }
    }
    
    func cheatFinderUpdateUI() {
        cheatFinderHide()
        switch cheatFinder.uiState {
        case .initial:
            [cheatFinderInitialActionsStackView, cheatFinderNewSearchButton, cheatFinderPromptLabel].forEach{ $0.isHidden = false }
            cheatFinderPromptLabel.text = "Start a new search to begin!"
        case .startedNewSearch:
            [cheatFinderInitialActionsStackView, cheatFinderNewSearchButton,
             cheatFinderSearchLessButton, cheatFinderSearchGreaterButton,
             cheatFinderSearchEqualButton,
             cheatFinderPromptLabel].forEach{ $0.isHidden = false }
            cheatFinderPromptLabel.text = "New search started! Search to find matches..."
        case .isSearching:
            [cheatFinderInitialActionsStackView, cheatFinderNewSearchButton, cheatFinderSearchLessButton, cheatFinderSearchGreaterButton,
             cheatFinderSearchEqualButton, cheatFinderPromptLabel].forEach{ $0.isHidden = false }
            cheatFinderPromptLabel.text = "Search for values..."
        case .didSearch:
            [cheatFinderInitialActionsStackView, cheatFinderNewSearchButton,
             cheatFinderSearchLessButton, cheatFinderSearchGreaterButton,
             cheatFinderSearchEqualButton,
             cheatFinderPromptLabel, cheatFinderMatchesTableView].forEach{ $0.isHidden = false }
            cheatFinderPromptLabel.text = "Number of matches: \(cheatFinder.matchedMemoryAddresses.count)"
            cheatFinderMatchesTableView.reloadData()
        }
    }
    
    @objc func cheatFinderSearchButtonPressed(_ sender: UIButton) {
        guard let memory = delegate?.memory else {
            print("Cannot proceed cheat search without reference to memory!")
            return
        }
        cheatFinder.comparisonMemory = [UInt8]()
        for address in 0..<0x95ff {
            cheatFinder.comparisonMemory.append(memory[address])
        }
        switch sender.tag {
        case 0:
            cheatFinder.findNewMatches(searchMode: CheatFinderManager.SearchMode.less)
        case 1:
            cheatFinder.findNewMatches(searchMode: CheatFinderManager.SearchMode.greater)
        case 2:
            cheatFinder.findNewMatches(searchMode: CheatFinderManager.SearchMode.same)
        default:
            break
        }
        cheatFinder.uiState = .didSearch
        cheatFinderUpdateUI()
    }
    
    func setupView() {
        view.addSubview(titleLabel)
        view.addSubview(segmentedControl)
        
        view.addSubview(editFieldsStackView)

        view.addSubview(emulatorScreenView)
        
//        view.addSubview(memoryField)
        
        view.addSubview(keyboardView)
        titleLabel.topAnchor.constraint(equalTo: view.topAnchor, constant: 16).isActive = true
        titleLabel.centerXAnchor.constraint(equalTo: view.centerXAnchor).isActive = true
        segmentedControl.topAnchor.constraint(equalTo: titleLabel.bottomAnchor, constant: 8).isActive = true
        segmentedControl.centerXAnchor.constraint(equalTo: titleLabel.centerXAnchor).isActive = true
        editFieldsStackView.topAnchor.constraint(equalTo: segmentedControl.bottomAnchor, constant: 8).isActive = true
//        memoryField.widthAnchor.constraint(equalToConstant: 80).isActive = true
//        memoryField.heightAnchor.constraint(equalToConstant: 40).isActive = true
        editFieldsStackView.centerXAnchor.constraint(equalTo: keyboardView.centerXAnchor).isActive = true
        keyboardView.topAnchor.constraint(equalTo: editFieldsStackView.bottomAnchor, constant: 8).isActive = true
//        keyboardView.widthAnchor.constraint(equalToConstant: 200).isActive = true
        keyboardView.heightAnchor.constraint(equalToConstant: 200).isActive = true
        keyboardView.viewModel.delegate = self
        
        emulatorScreenView.topAnchor.constraint(equalTo: segmentedControl.bottomAnchor, constant: 8).isActive = true
        emulatorScreenView.leadingAnchor.constraint(equalTo: view.safeAreaLayoutGuide.leadingAnchor, constant: 4.0).isActive = true
        emulatorScreenView.trailingAnchor.constraint(equalTo: view.safeAreaLayoutGuide.trailingAnchor, constant: -4.0).isActive = true
        emulatorScreenView.bottomAnchor.constraint(equalTo: view.safeAreaLayoutGuide.bottomAnchor, constant: -16).isActive = true
        emulatorScreenView.isHidden = true
        
        view.backgroundColor = UIColor.init(red: 0.14, green: 0.13, blue: 0.14, alpha: 1)
    }
    
    // Jump tab: normal constraints
    private var keyboardViewCenterXConstraint: NSLayoutConstraint!
    private var findCodeButtonLeadingConstraint: NSLayoutConstraint!
    private var findCodeButtonCenterYConstraint: NSLayoutConstraint!
    private var findInCodeTableViewLeadingConstraintOutOfView: NSLayoutConstraint!
    private var findInCodeTableViewBottomConstraintOutOfView: NSLayoutConstraint!
    var jumpTabNormalModeConstraints: [NSLayoutConstraint] {
        [
            keyboardViewCenterXConstraint,
            findCodeButtonLeadingConstraint,
            findCodeButtonCenterYConstraint,
            findInCodeTableViewLeadingConstraintOutOfView,
            findInCodeTableViewBottomConstraintOutOfView
        ]
    }
    
    // Jump tab: find in code constraints
    private var keyboardViewLeadingConstraint: NSLayoutConstraint!
    private var findCodeButtonBottomConstraint: NSLayoutConstraint!
    private var findCodeButtonCenterXConstraint: NSLayoutConstraint!
    private var findInCodeTableViewLeadingConstraint: NSLayoutConstraint!
    private var findInCodeTableViewTrailingConstraint: NSLayoutConstraint!
    private var findInCodeTableViewBottomConstraint: NSLayoutConstraint!
    var jumpTabFindInCodeConstraints: [NSLayoutConstraint] {
        [
            keyboardViewLeadingConstraint,
            findCodeButtonBottomConstraint,
            findInCodeTableViewLeadingConstraint,
            findInCodeTableViewTrailingConstraint,
            findCodeButtonCenterXConstraint,
            findInCodeTableViewBottomConstraint
        ]
    }

    
    // Jump tab: find in code table
    lazy var findInCodeResultsTableView: UITableView = {
        let view = UITableView(frame: .zero)
        view.translatesAutoresizingMaskIntoConstraints = false
        view.backgroundColor = .clear
        view.dataSource = self
        view.delegate = self
        view.register(UITableViewCell.self, forCellReuseIdentifier: "FindInCodeResultsCell")
        return view
    }()
    
    func findInCodeSetupView() {
        view.addSubview(findCodeButton)
        view.addSubview(findInCodeResultsTableView)

        // Find Code Button Normal config
        keyboardViewCenterXConstraint = keyboardView.centerXAnchor.constraint(equalTo: titleLabel.centerXAnchor)
        findCodeButtonLeadingConstraint = findCodeButton.leadingAnchor.constraint(equalTo: keyboardView.trailingAnchor, constant: 8)
        findCodeButtonCenterYConstraint = findCodeButton.centerYAnchor.constraint(equalTo: keyboardView.centerYAnchor)
        findInCodeTableViewLeadingConstraintOutOfView  = findInCodeResultsTableView.leadingAnchor.constraint(equalTo: view.trailingAnchor, constant: 50)
        findInCodeTableViewBottomConstraintOutOfView = findInCodeResultsTableView.bottomAnchor.constraint(equalTo: view.safeAreaLayoutGuide.bottomAnchor, constant: -8)
        
        // Find Code Button Searching Config
        keyboardViewLeadingConstraint = keyboardView.leadingAnchor.constraint(equalTo: view.safeAreaLayoutGuide.leadingAnchor, constant: 8)
        findCodeButtonBottomConstraint = findCodeButton.bottomAnchor.constraint(equalTo: view.safeAreaLayoutGuide.bottomAnchor, constant: 4)
        findCodeButtonCenterXConstraint = findCodeButton.centerXAnchor.constraint(equalTo: findInCodeResultsTableView.centerXAnchor)
        findInCodeTableViewBottomConstraint = findInCodeResultsTableView.bottomAnchor.constraint(equalTo: findCodeButton.topAnchor, constant: -4)
        findInCodeResultsTableView.topAnchor.constraint(equalTo: segmentedControl.bottomAnchor, constant: 8).isActive = true
        findInCodeTableViewLeadingConstraint = findInCodeResultsTableView.leadingAnchor.constraint(equalTo: keyboardView.trailingAnchor, constant: 8)
        findInCodeTableViewTrailingConstraint = findInCodeResultsTableView.trailingAnchor.constraint(equalTo: view.safeAreaLayoutGuide.trailingAnchor, constant: -8)

        NSLayoutConstraint.deactivate(jumpTabFindInCodeConstraints)
        NSLayoutConstraint.activate(jumpTabNormalModeConstraints)
        
        findCodeButton.onTapped = { isSelected in
            self.updateKeyboardPosition(isFindingCode: isSelected)
            self.updateMatchedInstructions()
        }
    }
    
    private func updateMatchedInstructions() {
        if let delegate = delegate,
           let text = memoryField.text,
           let address = getAddressFromText(text) {
            matchedInstructions = delegate.referencedMemoryAddresses[UInt16(address)] ?? [AddressedInstruction]()
            findInCodeResultsTableView.reloadData()
        }
    }
    
    private func updateKeyboardPosition(isFindingCode: Bool = false) {
        let animator = UIViewPropertyAnimator(duration: 0.2, curve: .linear) {
            if isFindingCode {
                NSLayoutConstraint.deactivate(self.jumpTabNormalModeConstraints)
                NSLayoutConstraint.activate(self.jumpTabFindInCodeConstraints)
            } else {
                NSLayoutConstraint.deactivate(self.jumpTabFindInCodeConstraints)
                NSLayoutConstraint.activate(self.jumpTabNormalModeConstraints)
            }
            self.view.layoutIfNeeded()
        }
        animator.startAnimation()
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        setupView()
        cheatFinderSetupView()
        findInCodeSetupView()
        update()
    }
    
    func getAddressFromText(_ text: String?) -> UInt64? {
        guard let text = text else {
            return nil
        }
        let scanner = Scanner(string: text)
        var address: UInt64 = 0
        if scanner.scanHexInt64(&address) && address < 0x95ff {
            return address
        }
        return nil
    }
    
    func updateTextField(with keyCode: Int) {
        guard var text = memoryField.text else {
            print("no text in address field!")
            return
        }
        text.append(String(format: "%1X",keyCode))
        let charLimit: Int = {
            switch mode {
            case .jumpToAddress:
                return 4
            case .changeMemory:
                return 2
            default:
                return 4
            }
        }()
        if text.count > charLimit {
            text.removeFirst()
        }
        if mode == .jumpToAddress, let address = getAddressFromText(text) {
            delegate?.jump(to: Int(address))
            if findCodeButton.isSelected {
                updateMatchedInstructions()
            }
        }
        memoryField.text = text
    }
    
    @objc func segmentedControlChanged(_ sender: UISegmentedControl) {
        switch sender.selectedSegmentIndex {
        case 0:
            mode = .jumpToAddress
        case 1:
            mode = .changeMemory
        case 2:
            mode = .cheat
        case 3:
            mode = .screen
        default:
            mode = .jumpToAddress
        }
        update()
    }
    
    func update() {
        cheatFinderHide()
        emulatorScreenView.isHidden = true
        findCodeButton.isHidden = true
        findInCodeResultsTableView.isHidden = true
        switch mode {
        case .jumpToAddress:
            memoryField.isHidden = false
            memoryField.layer.borderColor = UIColor.cyan.cgColor
            memoryField.textColor = .cyan
            editFieldsStackView.isHidden = false
            updateMemoryButton.isHidden = true
            resetMemoryButton.isHidden = true
            keyboardView.isHidden = false
            findCodeButton.isHidden = false
            findInCodeResultsTableView.isHidden = false
            if let selectedAddress = delegate?.selectedAddress {
                memoryField.text = String(format: "%04X", selectedAddress)
            }
            updateKeyboardPosition(isFindingCode: findCodeButton.isSelected)
        case .changeMemory:
            memoryField.isHidden = false
            memoryField.layer.borderColor = UIColor.orange.cgColor
            memoryField.textColor = .orange
            editFieldsStackView.isHidden = false
            updateMemoryButton.isHidden = false
            resetMemoryButton.isHidden = false
            keyboardView.isHidden = false
            if let selectedAddress = delegate?.selectedAddress {
                memoryField.text = delegate?.memoryHex(at: selectedAddress)
            }
            updateKeyboardPosition()
        case .cheat:
            memoryField.isHidden = true
            editFieldsStackView.isHidden = true
            keyboardView.isHidden = true
            cheatFinderUpdateUI()
        case .screen:
            memoryField.isHidden = true
            editFieldsStackView.isHidden = true
            keyboardView.isHidden = true
            emulatorScreenView.isHidden = false
            updateEmulatorScreen()
        }
    }
    
    @objc func updateButtonPressed(_ sender: UIButton) {
        guard let selectedAddress = delegate?.selectedAddress,
              let enteredText = memoryField.text,
              let memory = UInt8(enteredText, radix: 16) else {
            print("Could not get memory to update!")
            return
        }
        delegate?.updateMemory(at: selectedAddress, with: memory)
    }
}

extension DebugMemoryActionViewController: EmulatorKeyboardKeyPressedDelegate {
    func keyDown(_ key: KeyCoded) {
        print("DebugMemoryActionViewController keydown: \(key.keyLabel) ( \(key.keyCode) )")
    }
    
    func keyUp(_ key: KeyCoded) {
        print("DebugMemoryActionViewController keyUp: \(key.keyLabel) ( \(key.keyCode) )")
        updateTextField(with: key.keyCode)
    }
    
    func updateTransparency(toAlpha alpha: CGFloat) {
        // no op
    }
}

extension DebugMemoryActionViewController: DebugMemoryViewControllerDelegate {
    func refreshActionController() {
        update()
        updateMatchedInstructions()
    }
    func updateEmulatorScreen() {
        let emulatorView = EmuWrapper.getEmulatorView()
        if let snapshot = emulatorView?.getSnapshot() {
            let flipped = UIImage(cgImage: snapshot.cgImage!, scale: 1.0, orientation: .downMirrored)
            emulatorScreenView.image = flipped
        }
    }
}

extension DebugMemoryActionViewController: UITableViewDataSource {
    func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        if tableView == findInCodeResultsTableView {
            return matchedInstructions.count
        } else {
            return cheatFinder.matchedMemoryAddresses.keys.count
        }
    }
    
    func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        if tableView == findInCodeResultsTableView {
            let cell = tableView.dequeueReusableCell(withIdentifier: "FindInCodeResultsCell", for: indexPath)
            let instruction = matchedInstructions[indexPath.row]
            cell.textLabel?.text = instruction.description
            cell.textLabel?.font = UIFont(name: "Print Char 21", size: 11)
            cell.textLabel?.textColor = .yellow
            cell.textLabel?.textAlignment = .center
            return cell
        } else {
            let cell = tableView.dequeueReusableCell(withIdentifier: "CheatFinderMatchCell", for: indexPath)
            let addresses = cheatFinder.matchedMemoryAddresses.keys
            let index = addresses.index(addresses.startIndex, offsetBy: indexPath.row)
            let address = addresses[index]
            cell.textLabel?.text = String(format: "%04X: %02X",address,cheatFinder.matchedMemoryAddresses[address]!)
            cell.textLabel?.font = UIFont(name: "Print Char 21", size: 14)
            cell.textLabel?.textColor = .red
            cell.textLabel?.textAlignment = .center
            return cell
        }
    }
}

extension DebugMemoryActionViewController: UITableViewDelegate {
    func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        if tableView == findInCodeResultsTableView {
            let instruction = matchedInstructions[indexPath.row]
            let address = instruction.address
            delegate?.jump(to: Int(address))
        } else {
            let addresses = cheatFinder.matchedMemoryAddresses.keys
            let index = addresses.index(addresses.startIndex, offsetBy: indexPath.row)
            let address = addresses[index]
            delegate?.jump(to: address)
            tableView.deselectRow(at: indexPath, animated: true)
        }
    }
}

