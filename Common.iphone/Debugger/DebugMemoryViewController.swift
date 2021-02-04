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

extension UIView {
    func getSnapshot() -> UIImage {
        UIGraphicsBeginImageContextWithOptions(bounds.size, false, UIScreen.main.scale)
        drawHierarchy(in: self.bounds, afterScreenUpdates: true)
        let image = UIGraphicsGetImageFromCurrentImageContext()!
        UIGraphicsEndImageContext()
        return image
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
    let numToDisplayPerCell = 8
    let maxMemorySize = 128 * 1024
    
    private(set) var memory = EmuWrapper.memory()
    
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
        let interpreter = Debug6502Interpreter(memory: memoryAsArray)
        return interpreter.interpret()
    }()

    func refresh() {
        memory = EmuWrapper.memory()
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

class DebugPauseResumeButton: DebugMemoryButton {
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
    
    weak var delegate: DebugMemoryViewControllerDelegate?
    
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
    
    var actionControllerBottomConstraint: NSLayoutConstraint?
    
    var isShowingActionController = false
    var actionControllerAnimatorProgress = 0.0
    var animator: UIViewPropertyAnimator?
    
    private var displayLink: CADisplayLink?
    private var framesSince = 0
    private var framesSinceUpdateScreen = 0
    
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

    let testCodeButton: UIButton = {
        let button = UIButton(type: .custom)
        button.translatesAutoresizingMaskIntoConstraints = false
        button.setTitle("[CODE]", for: .normal)
        button.addTarget(self, action: #selector(testCodeButtonTapped(_:)), for: .touchUpInside)
        button.titleLabel?.font = UIFont(name: "Print Char 21", size: 12)
        button.setTitleColor(.red, for: .normal)
        return button
    }()
    @objc func testCodeButtonTapped(_ sender: UIButton) {
        let controller = DebugMemoryCodeViewController(memoryModel: self.memoryModel)
        present(controller, animated: true, completion: nil)
    }

    let tableView: UITableView = {
        let view = UITableView(frame: .zero)
        view.translatesAutoresizingMaskIntoConstraints = false
        view.backgroundColor = .clear
        view.separatorStyle = .none
        return view
    }()
    
    let resumePauseEmulationButton: DebugPauseResumeButton = {
        let button = DebugPauseResumeButton()
        button.translatesAutoresizingMaskIntoConstraints = false
        return button
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
        view.addSubview(resumePauseEmulationButton)
        titleLabel.centerXAnchor.constraint(equalTo: tableView.centerXAnchor).isActive = true
        titleLabel.topAnchor.constraint(equalTo: view.safeAreaLayoutGuide.topAnchor, constant: 16).isActive = true
        tableView.leadingAnchor.constraint(equalTo: view.safeAreaLayoutGuide.leadingAnchor, constant: 8).isActive = true
//        tableView.trailingAnchor.constraint(equalTo: view.safeAreaLayoutGuide.trailingAnchor, constant: -8).isActive = true
        tableView.topAnchor.constraint(equalTo: titleLabel.bottomAnchor, constant: 16).isActive = true
//        tableView.bottomAnchor.constraint(equalTo: view.bottomAnchor).isActive = true
        dismissButton.trailingAnchor.constraint(equalTo: tableView.trailingAnchor, constant: -8).isActive = true
        dismissButton.topAnchor.constraint(equalTo: view.safeAreaLayoutGuide.topAnchor, constant: 8.0).isActive = true
        resumePauseEmulationButton.leadingAnchor.constraint(equalTo: view.safeAreaLayoutGuide.leadingAnchor, constant: 8).isActive = true
        resumePauseEmulationButton.topAnchor.constraint(equalTo: view.safeAreaLayoutGuide.topAnchor, constant: 8).isActive = true
        view.backgroundColor = .black
        
        view.addSubview(testCodeButton)
        testCodeButton.trailingAnchor.constraint(equalTo: dismissButton.trailingAnchor).isActive = true
        testCodeButton.topAnchor.constraint(equalTo: dismissButton.bottomAnchor, constant: 4).isActive = true
    }
    
    func setupTableView() {
        tableView.dataSource = self
        tableView.delegate = self
        tableView.register(DebugMemoryCell.self, forCellReuseIdentifier: DebugMemoryCell.identifier)
    }
    
    func setupActionController() {
        let actionController = DebugMemoryActionViewController()
        delegate = actionController
        actionController.delegate = self
        addChild(actionController)
        actionController.didMove(toParent: self)
        actionController.view.translatesAutoresizingMaskIntoConstraints = false
        view.addSubview(actionController.view)
        actionController.view.trailingAnchor.constraint(equalTo: view.trailingAnchor).isActive = true
        actionControllerBottomConstraint = actionController.view.bottomAnchor.constraint(equalTo: view.safeAreaLayoutGuide.bottomAnchor, constant: 250)
        actionControllerBottomConstraint?.isActive = true

        actionControllerHeightConstraint = actionController.view.heightAnchor.constraint(equalToConstant: 320)
        actionControllerLeadingToViewLeadingConstraint = actionController.view.leadingAnchor.constraint(equalTo: view.leadingAnchor)
        actionControllerTopToTableViewBottomConstraint = actionController.view.topAnchor.constraint(equalTo: tableView.bottomAnchor)
        tableViewTrailingToViewTrailingConstraint = tableView.trailingAnchor.constraint(equalTo: view.trailingAnchor, constant: -8)

        actionControllerWidthConstraint = actionController.view.widthAnchor.constraint(equalToConstant: 400)
        actionControllerTopToViewTopConstraint = actionController.view.topAnchor.constraint(equalTo: view.safeAreaLayoutGuide.topAnchor)
        actionControllerLeadingToTableViewTrailingConstraint = actionController.view.leadingAnchor.constraint(equalTo: tableView.trailingAnchor)
        tableViewToViewBottomConstraint = tableView.bottomAnchor.constraint(equalTo: view.safeAreaLayoutGuide.bottomAnchor)
        let pan = UIPanGestureRecognizer(target: self, action: #selector(handlePan(_:)))
        actionController.view.addGestureRecognizer(pan)
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
            actionControllerBottomConstraint?.constant = 0
        }
    }
    
    func setupPauseResumeButton() {
        resumePauseEmulationButton.onTapped = { selected in
            if selected {
                EmuWrapper.resume()
                self.displayLink = CADisplayLink(target: self, selector: #selector(self.updateMemoryIfNeeded))
                self.displayLink?.isPaused = false
                self.displayLink?.add(to: RunLoop.main, forMode: .common)
            } else {
                EmuWrapper.pause()
                self.displayLink?.isPaused = true
                self.displayLink = nil
            }
        }
    }
    
    @objc func updateMemoryIfNeeded() {
        // maybe apply cheats here?
        if framesSinceUpdateScreen > 3 {
            delegate?.updateEmulatorScreen()
            framesSinceUpdateScreen = 1
        }
        if framesSince > 30 {
            memoryModel.refresh()
            tableView.reloadData()
            delegate?.refreshActionController()
            framesSince = 1
        }
        framesSince += 1
        framesSinceUpdateScreen += 1
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        setupView()
        setupActionController()
        setupTableView()
        setupPauseResumeButton()
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        memoryModel.refresh()
        setupActionControllerOrientationConstraints()
        resumePauseEmulationButton.isSelected = false
    }
    
    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)
        tableView.reloadData()
        delegate?.refreshActionController()
    }
    
    @objc func handlePan(_ recognizer: UIPanGestureRecognizer) {
        switch recognizer.state {
        case .began:
            animator = UIViewPropertyAnimator(duration: 0.2, curve: .linear, animations: {
                self.actionControllerBottomConstraint?.constant = self.isShowingActionController ? 250 : 0
                self.view.layoutIfNeeded()
            })
            animator?.addCompletion { _ in
                self.isShowingActionController = !self.isShowingActionController
            }
            animator?.pauseAnimation()
        case .changed:
            let translation = recognizer.translation(in: self.view)
            var fraction = -translation.y / 300
            if self.isShowingActionController { fraction *= -1 }
            animator?.fractionComplete = fraction
        case .ended:
            animator?.continueAnimation(withTimingParameters: nil, durationFactor: 0)
        default:
            break
        }
    }
    
    @objc func closeTapped(_ sender: UIButton) {
        displayLink?.isPaused = true
        displayLink = nil
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

extension DebugMemoryViewController: UITableViewDelegate {
    func tableView(_ tableView: UITableView, viewForHeaderInSection section: Int) -> UIView? {
        let title = self.tableView(tableView, titleForHeaderInSection: section)
        let label = UILabel(frame: .zero)
        label.text = title
        label.font = UIFont(name: "Print Char 21", size: 12)
        label.textColor = .cyan
        label.translatesAutoresizingMaskIntoConstraints = false
        let view = UIView(frame: .zero)
        view.addSubview(label)
        label.centerYAnchor.constraint(equalTo: view.centerYAnchor).isActive = true
        label.leadingAnchor.constraint(equalTo: view.leadingAnchor, constant: 8).isActive = true
        view.backgroundColor = .black
        view.layer.borderColor = UIColor.cyan.cgColor
        view.layer.borderWidth = 1.0
        view.heightAnchor.constraint(equalToConstant: 30).isActive = true
        return view
    }
}

extension DebugMemoryViewController: DebugMemoryCellDelegate {
    func updateSelection(to address: Int) {
        memoryModel.selectedAddress = address
        self.tableView.reloadData()
        // communicate selected address to action controller
        delegate?.refreshActionController()
    }
    
    func isAddressSelected(_ address: Int) -> Bool {
        if let selectedAddress = memoryModel.selectedAddress,
           selectedAddress == address {
            return true
        }
        return false
    }
}

protocol DebugMemoryViewControllerDelegate: class {
    func refreshActionController()
    func updateEmulatorScreen()
}

protocol DebugMemoryActionViewControllerDelegate: class {
    func jump(to address: Int)
    func memoryHex(at address: Int) -> String
    func updateMemory(at address: Int, with memory:UInt8)
    var memory: UnsafeMutablePointer<UInt8>? { get }
    var selectedAddress: Int? { get }
}

extension DebugMemoryViewController: DebugMemoryActionViewControllerDelegate {
    func updateMemory(at address: Int, with memory:UInt8) {
        memoryModel.setMemory(at: address, value: memory)
        self.tableView.reloadData()
    }
    
    func jump(to address: Int) {
        print("jumping to address: \(String(format: "%04X",address)) decimal: \(address)")
        let indexPath = memoryModel.indexPath(for: address)
        tableView.scrollToRow(at: indexPath, at: .middle, animated: true)
        updateSelection(to: address)
    }
    func memoryHex(at address: Int) -> String {
        return memoryModel.getMemoryHexString(at: address)
    }
    var memory: UnsafeMutablePointer<UInt8>? {
        return memoryModel.memory
    }
    var selectedAddress: Int? {
        return memoryModel.selectedAddress
    }
}

class DebugMemoryActionViewController: UIViewController {
    
    enum Mode {
        case jumpToAddress, changeMemory, cheat, screen
    }
    
    var mode: Mode = .jumpToAddress
    var cheatFinder = CheatFinderManager()
    
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
        cheatFinderMatchesTableView.trailingAnchor.constraint(equalTo: view.trailingAnchor, constant: 24).isActive = true
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
        titleLabel.topAnchor.constraint(equalTo: view.safeAreaLayoutGuide.topAnchor, constant: 16).isActive = true
        titleLabel.centerXAnchor.constraint(equalTo: view.centerXAnchor).isActive = true
        segmentedControl.topAnchor.constraint(equalTo: titleLabel.bottomAnchor, constant: 8).isActive = true
        segmentedControl.centerXAnchor.constraint(equalTo: titleLabel.centerXAnchor).isActive = true
        editFieldsStackView.topAnchor.constraint(equalTo: segmentedControl.bottomAnchor, constant: 8).isActive = true
//        memoryField.widthAnchor.constraint(equalToConstant: 80).isActive = true
//        memoryField.heightAnchor.constraint(equalToConstant: 40).isActive = true
        editFieldsStackView.centerXAnchor.constraint(equalTo: titleLabel.centerXAnchor).isActive = true
        keyboardView.centerXAnchor.constraint(equalTo: titleLabel.centerXAnchor).isActive = true
        keyboardView.topAnchor.constraint(equalTo: editFieldsStackView.bottomAnchor, constant: 8).isActive = true
//        keyboardView.widthAnchor.constraint(equalToConstant: 200).isActive = true
        keyboardView.heightAnchor.constraint(equalToConstant: 200).isActive = true
        keyboardView.viewModel.delegate = self
        
        emulatorScreenView.topAnchor.constraint(equalTo: segmentedControl.bottomAnchor, constant: 8).isActive = true
        emulatorScreenView.leadingAnchor.constraint(equalTo: view.safeAreaLayoutGuide.leadingAnchor, constant: 4.0).isActive = true
        emulatorScreenView.trailingAnchor.constraint(equalTo: view.safeAreaLayoutGuide.trailingAnchor, constant: -4.0).isActive = true
        emulatorScreenView.bottomAnchor.constraint(equalTo: view.safeAreaLayoutGuide.bottomAnchor, constant: -16).isActive = true
        emulatorScreenView.isHidden = true
        
        view.backgroundColor = .black
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        setupView()
        cheatFinderSetupView()
        update()
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
        if mode == .jumpToAddress {
            let scanner = Scanner(string: text)
            var address: UInt64 = 0
            if scanner.scanHexInt64(&address) && address < 0x95ff {
                delegate?.jump(to: Int(address))
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
        switch mode {
        case .jumpToAddress:
            memoryField.isHidden = false
            memoryField.layer.borderColor = UIColor.cyan.cgColor
            memoryField.textColor = .cyan
            editFieldsStackView.isHidden = false
            updateMemoryButton.isHidden = true
            resetMemoryButton.isHidden = true
            keyboardView.isHidden = false
            if let selectedAddress = delegate?.selectedAddress {
                memoryField.text = String(format: "%04X", selectedAddress)
            }
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
        return cheatFinder.matchedMemoryAddresses.keys.count
    }
    
    func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = tableView.dequeueReusableCell(withIdentifier: "CheatFinderMatchCell", for: indexPath)
        let addresses = cheatFinder.matchedMemoryAddresses.keys
        let index = addresses.index(addresses.startIndex, offsetBy: indexPath.row)
        let address = addresses[index]
        cell.textLabel?.text = String(format: "%04X",address)
        cell.textLabel?.font = UIFont(name: "Print Char 21", size: 14)
        cell.textLabel?.textColor = .red
        cell.textLabel?.textAlignment = .center
        return cell
    }
}

extension DebugMemoryActionViewController: UITableViewDelegate {
    func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        let addresses = cheatFinder.matchedMemoryAddresses.keys
        let index = addresses.index(addresses.startIndex, offsetBy: indexPath.row)
        let address = addresses[index]
        delegate?.jump(to: address)
        tableView.deselectRow(at: indexPath, animated: true)
    }
}

