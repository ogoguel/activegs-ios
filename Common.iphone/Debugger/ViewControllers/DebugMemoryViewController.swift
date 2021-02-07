//
//  DebugMemoryViewController.swift
//  ActiveGS
//
//  Created by Yoshi Sugawara on 1/17/21.
//

import Foundation
import UIKit

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

protocol DebugMemoryViewControllerDelegate: class {
    func refreshActionController()
    func updateEmulatorScreen()
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

    let dataTableView: UITableView = {
        let view = UITableView(frame: .zero)
        view.translatesAutoresizingMaskIntoConstraints = false
        view.backgroundColor = .clear
        view.separatorStyle = .none
        return view
    }()
    
    let codeTableView: UITableView = {
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
    
    let memoryViewModeControl: UISegmentedControl = {
        let control = UISegmentedControl(items: ["Data","Code"])
        control.translatesAutoresizingMaskIntoConstraints = false
        control.tintColor = .green
        control.setTitleTextAttributes([NSAttributedString.Key.font: UIFont(name: "Print Char 21", size: 14)!], for: .normal)
        control.addTarget(self, action: #selector(memoryViewModeControlChanged(_:)), for: .valueChanged)
        control.selectedSegmentIndex = 0
        return control
    }()
    
    init() {
        super.init(nibName: nil, bundle: nil)
    }
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
    func setupView() {
        view.addSubview(titleLabel)
        view.addSubview(memoryViewModeControl)
        view.addSubview(dataTableView)
        view.addSubview(codeTableView)
        view.addSubview(dismissButton)
        view.addSubview(resumePauseEmulationButton)
        titleLabel.centerXAnchor.constraint(equalTo: dataTableView.centerXAnchor).isActive = true
        titleLabel.topAnchor.constraint(equalTo: view.safeAreaLayoutGuide.topAnchor, constant: 16).isActive = true
        memoryViewModeControl.topAnchor.constraint(equalTo: titleLabel.bottomAnchor, constant: 8).isActive = true
        memoryViewModeControl.centerXAnchor.constraint(equalTo: dataTableView.centerXAnchor).isActive = true
        dataTableView.leadingAnchor.constraint(equalTo: view.safeAreaLayoutGuide.leadingAnchor, constant: 8).isActive = true
//        tableView.trailingAnchor.constraint(equalTo: view.safeAreaLayoutGuide.trailingAnchor, constant: -8).isActive = true
        dataTableView.topAnchor.constraint(equalTo: memoryViewModeControl.bottomAnchor, constant: 16).isActive = true
        codeTableView.leadingAnchor.constraint(equalTo: dataTableView.leadingAnchor).isActive = true
        codeTableView.trailingAnchor.constraint(equalTo: dataTableView.trailingAnchor).isActive = true
        codeTableView.topAnchor.constraint(equalTo: dataTableView.topAnchor).isActive = true
        codeTableView.bottomAnchor.constraint(equalTo: dataTableView.bottomAnchor).isActive = true
//        tableView.bottomAnchor.constraint(equalTo: view.bottomAnchor).isActive = true
        dismissButton.trailingAnchor.constraint(equalTo: dataTableView.trailingAnchor, constant: -8).isActive = true
        dismissButton.topAnchor.constraint(equalTo: view.safeAreaLayoutGuide.topAnchor, constant: 8.0).isActive = true
        resumePauseEmulationButton.leadingAnchor.constraint(equalTo: view.safeAreaLayoutGuide.leadingAnchor, constant: 8).isActive = true
        resumePauseEmulationButton.topAnchor.constraint(equalTo: view.safeAreaLayoutGuide.topAnchor, constant: 8).isActive = true
        view.backgroundColor = .black
    }
    
    func setupTableView() {
        dataTableView.dataSource = self
        dataTableView.delegate = self
        dataTableView.register(DebugMemoryCell.self, forCellReuseIdentifier: DebugMemoryCell.identifier)
        codeTableView.dataSource = self
        codeTableView.delegate = self
        codeTableView.register(DebugMemoryCell.self, forCellReuseIdentifier: DebugMemoryCell.identifier)
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
        actionControllerTopToTableViewBottomConstraint = actionController.view.topAnchor.constraint(equalTo: dataTableView.bottomAnchor)
        tableViewTrailingToViewTrailingConstraint = dataTableView.trailingAnchor.constraint(equalTo: view.trailingAnchor, constant: -8)

        actionControllerWidthConstraint = actionController.view.widthAnchor.constraint(equalToConstant: 400)
        actionControllerTopToViewTopConstraint = actionController.view.topAnchor.constraint(equalTo: view.safeAreaLayoutGuide.topAnchor)
        actionControllerLeadingToTableViewTrailingConstraint = actionController.view.leadingAnchor.constraint(equalTo: dataTableView.trailingAnchor)
        tableViewToViewBottomConstraint = dataTableView.bottomAnchor.constraint(equalTo: view.safeAreaLayoutGuide.bottomAnchor)
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
    
    @objc func memoryViewModeControlChanged(_ sender: UISegmentedControl) {
        switch sender.selectedSegmentIndex {
        case 1:
            codeTableView.isHidden = false
            dataTableView.isHidden = true
        default:
            codeTableView.isHidden = true
            dataTableView.isHidden = false
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
            dataTableView.reloadData()
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
        memoryViewModeControlChanged(memoryViewModeControl)
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        memoryModel.refresh()
        setupActionControllerOrientationConstraints()
        resumePauseEmulationButton.isSelected = false
    }
    
    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)
        dataTableView.reloadData()
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
}

extension DebugMemoryViewController: UITableViewDataSource {
    func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        if tableView == codeTableView {
            return memoryModel.interpretedInstructions.count
        } else {
            let numItems = EmuMemoryMapSection(rawValue: section)!.numberOfItems
            var numRows = numItems / memoryModel.numToDisplayPerCell
            if numItems % memoryModel.numToDisplayPerCell > 0 {
                numRows += 1
            }
            return numRows
        }
    }
    
    func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        if tableView == codeTableView {
            let cell = tableView.dequeueReusableCell(withIdentifier: DebugMemoryCell.identifier, for: indexPath) as! DebugMemoryCell
            let instruction = memoryModel.interpretedInstructions[indexPath.row]
            let values = [
                instruction.instruction.mnemonic.description,
                instruction.instruction.addressingMode.description
            ]
            cell.updateWith(delegate: self, offset: Int(instruction.address), hexMemoryValues: values)
            return cell
        } else {
            let cell = tableView.dequeueReusableCell(withIdentifier: DebugMemoryCell.identifier, for: indexPath) as! DebugMemoryCell
            let memoryVals = memoryModel.hexStrings(for: indexPath)
            let offset = memoryModel.offset(for: indexPath)
            cell.updateWith(delegate: self, offset: offset, hexMemoryValues: memoryVals)
            return cell
        }
    }
    
    func numberOfSections(in tableView: UITableView) -> Int {
        if tableView == codeTableView {
            return 1
        } else {
            return EmuMemoryMapSection.applesoftStringData.rawValue + 1
        }
    }
    
    func tableView(_ tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        if tableView == dataTableView {
            return EmuMemoryMapSection.init(rawValue: section)!.title
        }
        return nil
    }
}

extension DebugMemoryViewController: UITableViewDelegate {
    func tableView(_ tableView: UITableView, viewForHeaderInSection section: Int) -> UIView? {
        if tableView == dataTableView {
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
        return nil
    }
}

extension DebugMemoryViewController: DebugMemoryCellDelegate {
    func updateSelection(to address: Int) {
        memoryModel.selectedAddress = address
        self.dataTableView.reloadData()
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

extension DebugMemoryViewController: DebugMemoryActionViewControllerDelegate {
    func updateMemory(at address: Int, with memory:UInt8) {
        memoryModel.setMemory(at: address, value: memory)
        self.dataTableView.reloadData()
    }
    
    func jump(to address: Int) {
        print("jumping to address: \(String(format: "%04X",address)) decimal: \(address)")
        let indexPath = memoryModel.indexPath(for: address)
        dataTableView.scrollToRow(at: indexPath, at: .middle, animated: true)
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
