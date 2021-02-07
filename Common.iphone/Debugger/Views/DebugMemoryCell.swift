//
//  DebugMemoryCell.swift
//  ActiveGS
//
//  Created by Yoshi Sugawara on 2/4/21.
//

import Foundation

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
    
    func updateWith(
        delegate: DebugMemoryCellDelegate,
        offset: Int,
        hexMemoryValues: [String]
    ) {
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
