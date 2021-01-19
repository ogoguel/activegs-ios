//
//  DebugMemoryViewController.swift
//  ActiveGS
//
//  Created by Yoshi Sugawara on 1/17/21.
//

import Foundation
import UIKit

class EmuMemoryModel {
    // make this static for now
    let numToDisplayPerCell = 8
    let maxMemorySize = 128 * 1024
    
    let slowMemory = EmuWrapper.slowMemory()
    
    func offset(for indexPath: IndexPath) -> Int {
        return indexPath.row * numToDisplayPerCell
    }
    
    func hexStrings(for indexPath: IndexPath) -> [String] {
        let startIndex = offset(for: indexPath)
        let endIndex = min(maxMemorySize, startIndex + numToDisplayPerCell)
        var row = [String]()
        guard let slowMemory = slowMemory else {
            return row
        }
        for i in startIndex..<endIndex {
            row.append(String(format: "%02X", slowMemory[i]))
        }
        return row
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

class DebugMemoryCell: UITableViewCell {
    static let identifier = "DebugMemoryCell"
    
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
    
    func updateWith(offset: Int, hexMemoryValues: [String]) {
        self.offset = offset
        stackView.arrangedSubviews.forEach{ $0.removeFromSuperview() }
        addressLabel.text = String(format: "%04X:", offset)
        stackView.addArrangedSubview(addressLabel)
        stackView.setCustomSpacing(3, after: addressLabel)
        for (index, hexValue) in hexMemoryValues.enumerated() {
            let button = UIButton(type: .custom)
            button.setTitle(hexValue, for: .normal)
            button.titleLabel?.font = UIFont(name: "Print Char 21", size: 16)
            button.setTitleColor(.green, for: .normal)
            button.tag = index
            button.addTarget(self, action: #selector(didTapOnButton(_:)), for: .touchUpInside)
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
    
    let titleLabel: UILabel = {
        let label = UILabel(frame: .zero)
        label.translatesAutoresizingMaskIntoConstraints = false
        label.font = UIFont(name: "Print Char 21", size: 14)
        label.textColor = .white
        label.text = "Memory Debugger"
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
        titleLabel.centerXAnchor.constraint(equalTo: view.centerXAnchor).isActive = true
        titleLabel.topAnchor.constraint(equalTo: view.safeAreaLayoutGuide.topAnchor, constant: 16).isActive = true
        tableView.leadingAnchor.constraint(equalTo: view.safeAreaLayoutGuide.leadingAnchor, constant: 8).isActive = true
        tableView.trailingAnchor.constraint(equalTo: view.safeAreaLayoutGuide.trailingAnchor, constant: -8).isActive = true
        tableView.topAnchor.constraint(equalTo: titleLabel.bottomAnchor, constant: 16).isActive = true
        tableView.bottomAnchor.constraint(equalTo: view.bottomAnchor).isActive = true
        dismissButton.trailingAnchor.constraint(equalTo: view.safeAreaLayoutGuide.trailingAnchor, constant: -8).isActive = true
        dismissButton.topAnchor.constraint(equalTo: view.safeAreaLayoutGuide.topAnchor, constant: 8.0).isActive = true
        view.backgroundColor = .black
    }
    
    func setupTableView() {
        tableView.dataSource = self
        tableView.register(DebugMemoryCell.self, forCellReuseIdentifier: DebugMemoryCell.identifier)
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        setupView()
        setupTableView()
    }
    
    @objc func closeTapped(_ sender: UIButton) {
        dismiss(animated: true) {
            EmuWrapper.resume()
        }
    }
}

extension DebugMemoryViewController: UITableViewDataSource {
    func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        var numRows = memoryModel.maxMemorySize / memoryModel.numToDisplayPerCell
        if memoryModel.maxMemorySize % memoryModel.numToDisplayPerCell > 0 {
            numRows += 1
        }
        return numRows
    }
    
    func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = tableView.dequeueReusableCell(withIdentifier: DebugMemoryCell.identifier, for: indexPath) as! DebugMemoryCell
        let memoryVals = memoryModel.hexStrings(for: indexPath)
        cell.updateWith(offset: memoryModel.offset(for: indexPath), hexMemoryValues: memoryVals)
        return cell
    }
    
    
}
