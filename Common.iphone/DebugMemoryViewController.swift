//
//  DebugMemoryViewController.swift
//  ActiveGS
//
//  Created by Yoshi Sugawara on 1/17/21.
//

import Foundation
import UIKit

struct Orientation {
    // indicate current device is in the LandScape orientation
    static var isLandscape: Bool {
        get {
            return UIDevice.current.orientation.isValidInterfaceOrientation
                ? UIDevice.current.orientation.isLandscape
                : (UIApplication.shared.windows.first?.windowScene?.interfaceOrientation.isLandscape)!
        }
    }
    // indicate current device is in the Portrait orientation
    static var isPortrait: Bool {
        get {
            return UIDevice.current.orientation.isValidInterfaceOrientation
                ? UIDevice.current.orientation.isPortrait
                : (UIApplication.shared.windows.first?.windowScene?.interfaceOrientation.isPortrait)!
        }
    }
}

enum EmuMemoryMapSection: Int {
    case zeroPage = 0
    case processorStack
    case getlnBuffer
    case freeSpace
    case dosProdosInterruptVectors
    case textVideoPageAndPeripheralScreenholes
    case textVideoPageTwoOrApplesoftProgramVariables
    case freespace2
    case highResGraphicsPage1
    case highResGraphicsPage2
    case applesoftStringData
    
    var range:Range<Int> {
        switch self {
        case .zeroPage: return 0..<0x100
        case .processorStack: return 0x100..<0x200
        case .getlnBuffer: return 0x200..<0x300
        case .freeSpace: return 0x300..<0x3d0
        case .dosProdosInterruptVectors: return 0x3d0..<0x400
        case .textVideoPageAndPeripheralScreenholes: return 0x400..<0x800
        case .textVideoPageTwoOrApplesoftProgramVariables: return 0x800..<0xc00
        case .freespace2: return 0xc00..<0x2000
        case .highResGraphicsPage1: return 0x2000..<0x4000
        case .highResGraphicsPage2: return 0x4000..<0x6000
        case .applesoftStringData: return 0x6000..<0x95ff
        }
    }
    
    var numberOfItems: Int {
        self.range.count
    }
    
    var title: String {
        switch self {
        case .zeroPage: return "Zero Page"
        case .processorStack: return "6502 Processor Stack"
        case .getlnBuffer: return "GETLN Line Input Buffer"
        case .freeSpace: return "Free Space for Machine Language, Shape Table, etc."
        case .dosProdosInterruptVectors: return "DOS, ProDOS, and Interrupt Vectors"
        case .textVideoPageAndPeripheralScreenholes: return "Text Video Page and Peripheral Screenholes"
        case .textVideoPageTwoOrApplesoftProgramVariables: return "Text Video Page 2 or Applesoft Program and Variables"
        case .freespace2: return "Free Space for Machine Language, Shapes, etc."
        case .highResGraphicsPage1: return "High Resolution Graphics Page 1"
        case .highResGraphicsPage2: return "High Resolution Graphics Page 2"
        case .applesoftStringData: return "Applesoft String Data"
        }
    }
    
    static func section(for address:Int) -> EmuMemoryMapSection {
        if EmuMemoryMapSection.zeroPage.range.contains(address) {
            return .zeroPage
        } else if EmuMemoryMapSection.processorStack.range.contains(address) {
            return .processorStack
        } else if EmuMemoryMapSection.getlnBuffer.range.contains(address) {
            return .getlnBuffer
        } else if EmuMemoryMapSection.freeSpace.range.contains(address) {
            return .freeSpace
        } else if EmuMemoryMapSection.dosProdosInterruptVectors.range.contains(address) {
            return .dosProdosInterruptVectors
        } else if EmuMemoryMapSection.textVideoPageAndPeripheralScreenholes.range.contains(address) {
            return .textVideoPageAndPeripheralScreenholes
        } else if EmuMemoryMapSection.textVideoPageTwoOrApplesoftProgramVariables.range.contains(address) {
            return .textVideoPageTwoOrApplesoftProgramVariables
        } else if EmuMemoryMapSection.freespace2.range.contains(address) {
            return .freespace2
        } else if EmuMemoryMapSection.highResGraphicsPage1.range.contains(address) {
            return .highResGraphicsPage1
        } else if EmuMemoryMapSection.highResGraphicsPage2.range.contains(address) {
            return .highResGraphicsPage2
        } else {
            return .applesoftStringData
        }
    }
}

class EmuMemoryModel {
    // make this static for now
    let numToDisplayPerCell = 8
    let maxMemorySize = 128 * 1024
    
    let slowMemory = EmuWrapper.slowMemory()
    
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
        guard let slowMemory = slowMemory else {
            return row
        }
        for i in startIndex..<endIndex {
            row.append(String(format: "%02X", slowMemory[i]))
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
        guard let slowMemory = slowMemory else {
            return ""
        }
        let value = slowMemory[address]
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
        guard let slowMemory = slowMemory else {
            return
        }
        slowMemory[address] = value
    }

}

class DebugMemoryButton: UIButton {
    var onTapped: ((Bool) -> Void)?
    
    override open var isSelected: Bool {
        didSet {
            backgroundColor = isSelected ? .white : .clear
        }
    }

    convenience init() {
        self.init(type: .custom)
        addTarget(self, action: #selector(tapped(_:)), for: .touchUpInside)
    }
    
    @objc func tapped(_ sender: UIButton) {
        isSelected.toggle()
        onTapped?(isSelected)
    }
}

protocol DebugMemoryCellDelegate: class {
    func updateSelection(to address: Int)
    func isAddressSelected(_ address: Int) -> Bool
}

class DebugMemoryCell: UITableViewCell {
    static let identifier = "DebugMemoryCell"
    weak var delegate: DebugMemoryCellDelegate?
    
    var updateSelection: ((Int) -> Void)?
    
    var offset: Int?
    
    lazy var stackView: UIStackView = {
        let view = UIStackView(frame: .zero)
        view.translatesAutoresizingMaskIntoConstraints = false
        view.axis = .horizontal
        view.spacing = 8.0
        view.alignment = .center
        return view
    }()
    
    let addressLabel: UILabel = {
        let label = UILabel(frame: .zero)
        label.translatesAutoresizingMaskIntoConstraints = false
        label.font = UIFont(name: "Print Char 21", size: 16)
        label.textColor = .yellow
        label.setContentHuggingPriority(.defaultHigh, for: .horizontal)
        return label
    }()
    
    override init(style: UITableViewCell.CellStyle, reuseIdentifier: String?) {
        super.init(style: style, reuseIdentifier: reuseIdentifier)
        contentView.addSubview(stackView)
        stackView.leadingAnchor.constraint(equalTo: contentView.leadingAnchor, constant: 8.0).isActive = true
        stackView.trailingAnchor.constraint(equalTo: contentView.trailingAnchor, constant: 8.0).isActive = true
        stackView.topAnchor.constraint(equalTo: contentView.topAnchor, constant: 8.0).isActive = true
        stackView.bottomAnchor.constraint(equalTo: contentView.bottomAnchor, constant: 8.0).isActive = true
        backgroundColor = .clear
        selectionStyle = .none
    }
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
    func updateWith(delegate: DebugMemoryCellDelegate, offset: Int, hexMemoryValues: [String]) {
        self.delegate = delegate
        self.offset = offset
        stackView.arrangedSubviews.forEach{ $0.removeFromSuperview() }
        addressLabel.text = String(format: "%04X:", offset)
        stackView.addArrangedSubview(addressLabel)
        stackView.setCustomSpacing(3, after: addressLabel)
        for (index, hexValue) in hexMemoryValues.enumerated() {
            let button = DebugMemoryButton()
            let address = offset + index
            button.setTitle(hexValue, for: .normal)
            button.titleLabel?.font = UIFont(name: "Print Char 21", size: 16)
            button.setTitleColor(.green, for: .normal)
            button.tag = index
            button.setTitleColor(.black, for: .selected)
            button.onTapped = { didSelect in
                if didSelect {
                    self.delegate?.updateSelection(to: address)
                }
            }
            button.isSelected = self.delegate?.isAddressSelected(address) ?? false
//            button.addTarget(self, action: #selector(didTapOnButton(_:)), for: .touchUpInside)
            stackView.addArrangedSubview(button)
        }
        let spacer = UIView()
        spacer.setContentHuggingPriority(.defaultLow, for: .horizontal)
        stackView.addArrangedSubview(spacer)
    }
    
    @objc func didTapOnButton(_ button: UIButton) {
        let memoryAddr = String(format:"%04X",(offset ?? 0) + button.tag)
        print("tapped on button index \(button.tag), address: \(memoryAddr) ")
    }
}

@objc class DebugMemoryViewController: UIViewController {
    let memoryModel = EmuMemoryModel()
    
    // Landscape constraints
    var actionControllerTopToViewTopConstraint: NSLayoutConstraint?
    var actionControllerWidthConstraint: NSLayoutConstraint?
    var actionControllerLeadingToTableViewTrailingConstraint: NSLayoutConstraint?
    var tableViewToViewBottomConstraint: NSLayoutConstraint?
    
    // Portrait constraints
    var actionControllerLeadingToViewLeadingConstraint: NSLayoutConstraint?
    var actionControllerTopToTableViewBottomConstraint: NSLayoutConstraint?
    var actionControllerHeightConstraint: NSLayoutConstraint?
    var tableViewTrailingToViewTrailingConstraint: NSLayoutConstraint?
    
    let titleLabel: UILabel = {
        let label = UILabel(frame: .zero)
        label.translatesAutoresizingMaskIntoConstraints = false
        label.font = UIFont(name: "Print Char 21", size: 14)
        label.textColor = .white
        label.text = "Apple II Memory Debugger"
        return label
    }()
    
    let dismissButton: UIButton = {
        let button = UIButton(type: .custom)
        button.translatesAutoresizingMaskIntoConstraints = false
        button.setTitle("[X]", for: .normal)
        button.addTarget(self, action: #selector(closeTapped(_:)), for: .touchUpInside)
        button.titleLabel?.font = UIFont(name: "Print Char 21", size: 12)
        button.setTitleColor(.red, for: .normal)
        return button
    }()

    let tableView: UITableView = {
        let view = UITableView(frame: .zero)
        view.translatesAutoresizingMaskIntoConstraints = false
        view.backgroundColor = .clear
        return view
    }()
    
    init() {
        super.init(nibName: nil, bundle: nil)
    }
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
    func setupView() {
        view.addSubview(titleLabel)
        view.addSubview(tableView)
        view.addSubview(dismissButton)
        titleLabel.centerXAnchor.constraint(equalTo: tableView.centerXAnchor).isActive = true
        titleLabel.topAnchor.constraint(equalTo: view.safeAreaLayoutGuide.topAnchor, constant: 16).isActive = true
        tableView.leadingAnchor.constraint(equalTo: view.safeAreaLayoutGuide.leadingAnchor, constant: 8).isActive = true
//        tableView.trailingAnchor.constraint(equalTo: view.safeAreaLayoutGuide.trailingAnchor, constant: -8).isActive = true
        tableView.topAnchor.constraint(equalTo: titleLabel.bottomAnchor, constant: 16).isActive = true
//        tableView.bottomAnchor.constraint(equalTo: view.bottomAnchor).isActive = true
        dismissButton.trailingAnchor.constraint(equalTo: tableView.trailingAnchor, constant: -8).isActive = true
        dismissButton.topAnchor.constraint(equalTo: view.safeAreaLayoutGuide.topAnchor, constant: 8.0).isActive = true
        view.backgroundColor = .black
    }
    
    func setupTableView() {
        tableView.dataSource = self
        tableView.register(DebugMemoryCell.self, forCellReuseIdentifier: DebugMemoryCell.identifier)
    }
    
    func setupActionController() {
        let actionController = DebugMemoryActionViewController()
        actionController.delegate = self
        addChild(actionController)
        actionController.didMove(toParent: self)
        actionController.view.translatesAutoresizingMaskIntoConstraints = false
        view.addSubview(actionController.view)
        actionController.view.trailingAnchor.constraint(equalTo: view.trailingAnchor).isActive = true
        actionController.view.bottomAnchor.constraint(equalTo: view.safeAreaLayoutGuide.bottomAnchor).isActive = true

        actionControllerHeightConstraint = actionController.view.heightAnchor.constraint(equalToConstant: 320)
        actionControllerLeadingToViewLeadingConstraint = actionController.view.leadingAnchor.constraint(equalTo: view.leadingAnchor)
        actionControllerTopToTableViewBottomConstraint = actionController.view.topAnchor.constraint(equalTo: tableView.bottomAnchor)
        tableViewTrailingToViewTrailingConstraint = tableView.trailingAnchor.constraint(equalTo: view.trailingAnchor, constant: -8)

        actionControllerWidthConstraint = actionController.view.widthAnchor.constraint(equalToConstant: 400)
        actionControllerTopToViewTopConstraint = actionController.view.topAnchor.constraint(equalTo: view.safeAreaLayoutGuide.topAnchor)
        actionControllerLeadingToTableViewTrailingConstraint = actionController.view.leadingAnchor.constraint(equalTo: tableView.trailingAnchor)
        tableViewToViewBottomConstraint = tableView.bottomAnchor.constraint(equalTo: view.safeAreaLayoutGuide.bottomAnchor)
        
    }
    
    func setupActionControllerOrientationConstraints(orientation: UIInterfaceOrientation? = nil) {
        actionControllerLeadingToViewLeadingConstraint?.isActive = false
        actionControllerTopToTableViewBottomConstraint?.isActive = false
        actionControllerHeightConstraint?.isActive = false
        actionControllerTopToViewTopConstraint?.isActive = false
        actionControllerWidthConstraint?.isActive = false
        actionControllerLeadingToTableViewTrailingConstraint?.isActive = false
        tableViewToViewBottomConstraint?.isActive = false
        tableViewTrailingToViewTrailingConstraint?.isActive = false
        
        let isPortrait = orientation != nil ? orientation!.isPortrait : Orientation.isPortrait

        if isPortrait {
            actionControllerLeadingToViewLeadingConstraint?.isActive = true
            actionControllerTopToTableViewBottomConstraint?.isActive = true
            actionControllerHeightConstraint?.isActive = true
            tableViewTrailingToViewTrailingConstraint?.isActive = true
        } else {
            actionControllerTopToViewTopConstraint?.isActive = true
            actionControllerWidthConstraint?.isActive = true
            actionControllerLeadingToTableViewTrailingConstraint?.isActive = true
            tableViewToViewBottomConstraint?.isActive = true
        }
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        setupView()
        setupActionController()
        setupTableView()
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        setupActionControllerOrientationConstraints()
    }
    
    @objc func closeTapped(_ sender: UIButton) {
        dismiss(animated: true) {
            EmuWrapper.resume()
        }
    }
    
    override func viewDidLayoutSubviews() {
        super.viewDidLayoutSubviews()
        setupActionControllerOrientationConstraints()
    }
    
//    override func viewWillTransition(to size: CGSize, with coordinator: UIViewControllerTransitionCoordinator) {
//        if size.width > size.height {
//            setupActionControllerOrientationConstraints(orientation: .landscapeLeft)
//        } else {
//            setupActionControllerOrientationConstraints(orientation: .portrait)
//        }
//    }
}

extension DebugMemoryViewController: UITableViewDataSource {
    func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        let numItems = EmuMemoryMapSection(rawValue: section)!.numberOfItems
        var numRows = numItems / memoryModel.numToDisplayPerCell
        if numItems % memoryModel.numToDisplayPerCell > 0 {
            numRows += 1
        }
        return numRows
    }
    
    func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = tableView.dequeueReusableCell(withIdentifier: DebugMemoryCell.identifier, for: indexPath) as! DebugMemoryCell
        let memoryVals = memoryModel.hexStrings(for: indexPath)
        let offset = memoryModel.offset(for: indexPath)
        cell.updateWith(delegate: self, offset: offset, hexMemoryValues: memoryVals)
        return cell
    }
    
    func numberOfSections(in tableView: UITableView) -> Int {
        return EmuMemoryMapSection.applesoftStringData.rawValue + 1
    }
    
    func tableView(_ tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        return EmuMemoryMapSection.init(rawValue: section)!.title
    }
}

extension DebugMemoryViewController: DebugMemoryCellDelegate {
    func updateSelection(to address: Int) {
        memoryModel.selectedAddress = address
        self.tableView.reloadData()
        // communicate selected address to action controller
    }
    
    func isAddressSelected(_ address: Int) -> Bool {
        if let selectedAddress = memoryModel.selectedAddress,
           selectedAddress == address {
            return true
        }
        return false
    }
}

protocol DebugMemoryActionViewControllerDelegate: class {
    func jump(to address: Int)
}

extension DebugMemoryViewController: DebugMemoryActionViewControllerDelegate {
    func jump(to address: Int) {
        print("jumping to address: \(String(format: "%04X",address)) decimal: \(address)")
        let indexPath = memoryModel.indexPath(for: address)
        tableView.scrollToRow(at: indexPath, at: .middle, animated: true)
        updateSelection(to: address)
    }
}

class DebugMemoryActionViewController: UIViewController {
    
    enum Mode {
        case jumpToAddress, changeMemory
    }
    
    var mode: Mode = .jumpToAddress
    
    weak var delegate:DebugMemoryActionViewControllerDelegate?
    
    let segmentedControl: UISegmentedControl = {
        let control = UISegmentedControl(items: ["Jump to Address", "Edit Memory"])
        control.translatesAutoresizingMaskIntoConstraints = false
        control.tintColor = .orange
        control.selectedSegmentIndex = 0
        return control
    }()
    
    let jumpToAddressField: UITextField = {
        let field = UITextField(frame: .zero)
        field.translatesAutoresizingMaskIntoConstraints = false
        field.font = UIFont(name: "Print Char 21", size: 14)
        field.isUserInteractionEnabled = false
        field.text = ""
        field.textColor = .cyan
        field.layer.borderWidth = 1.0
        field.layer.borderColor = UIColor.cyan.cgColor
        field.textAlignment = .center
        return field
    }()
    
    let changeMemoryField: UITextField = {
        let field = UITextField(frame: .zero)
        field.translatesAutoresizingMaskIntoConstraints = false
        field.font = UIFont(name: "Print Char 21", size: 14)
        field.isUserInteractionEnabled = false
        field.text = ""
        return field
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
        label.translatesAutoresizingMaskIntoConstraints = false
        label.font = UIFont(name: "Print Char 21", size: 14)
        label.text = "Memory Tools"
        label.translatesAutoresizingMaskIntoConstraints = false
        label.textColor = .orange
        return label
    }()
    
    func setupView() {
        view.addSubview(titleLabel)
        view.addSubview(segmentedControl)
        view.addSubview(jumpToAddressField)
        view.addSubview(keyboardView)
        titleLabel.topAnchor.constraint(equalTo: view.safeAreaLayoutGuide.topAnchor, constant: 16).isActive = true
        titleLabel.centerXAnchor.constraint(equalTo: view.centerXAnchor).isActive = true
        segmentedControl.topAnchor.constraint(equalTo: titleLabel.bottomAnchor, constant: 8).isActive = true
        segmentedControl.centerXAnchor.constraint(equalTo: titleLabel.centerXAnchor).isActive = true
        jumpToAddressField.topAnchor.constraint(equalTo: segmentedControl.bottomAnchor, constant: 8).isActive = true
        jumpToAddressField.widthAnchor.constraint(equalToConstant: 80).isActive = true
        jumpToAddressField.heightAnchor.constraint(equalToConstant: 40).isActive = true
        jumpToAddressField.centerXAnchor.constraint(equalTo: titleLabel.centerXAnchor).isActive = true
        keyboardView.centerXAnchor.constraint(equalTo: titleLabel.centerXAnchor).isActive = true
        keyboardView.topAnchor.constraint(equalTo: jumpToAddressField.bottomAnchor, constant: 8).isActive = true
//        keyboardView.widthAnchor.constraint(equalToConstant: 200).isActive = true
        keyboardView.heightAnchor.constraint(equalToConstant: 200).isActive = true
        keyboardView.viewModel.delegate = self
        view.backgroundColor = .black
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        setupView()
    }
    
    func updateTextField(with keyCode: Int) {
        guard var text = jumpToAddressField.text else {
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
            }
        }()
        if text.count > charLimit {
            text.removeFirst()
        }
        if mode == .jumpToAddress {
            let scanner = Scanner(string: text)
            var address: UInt64 = 0
            if scanner.scanHexInt64(&address) && address < 0x95ff {
                delegate?.jump(to: Int(address))
            }
        }
        jumpToAddressField.text = text
        
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

// delegate impl: EmulatorKeyboardKeyPressedDelegate
// make delegate:
