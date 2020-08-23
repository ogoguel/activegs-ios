//
//  EmulatorKeyboard.swift
//  ActiveGS
//
//  Created by Yoshi Sugawara on 7/30/20.
//

// TODO: shift key should change the label of the keys to uppercase (need callback mechanism?)
// pan gesture to outer edges of keyboard view for better dragging
// double tap modifier to toggle
// taller keys?
// alt key: add shift to left

import Foundation
import UIKit

class KeyboardButton: UIButton {
    let key: KeyCoded
    var toggleState = false
    
    // MARK: - Functions
    override func point(inside point: CGPoint, with event: UIEvent?) -> Bool {
        let newArea = CGRect(
            x: self.bounds.origin.x - 5.0,
            y: self.bounds.origin.y - 5.0,
            width: self.bounds.size.width + 20.0,
            height: self.bounds.size.height + 20.0
        )
        return newArea.contains(point)
    }
    
    override open var isHighlighted: Bool {
        didSet {
            // this is the problem - the isHighlighted sets to false for the touchesEnded callback
            // need to look ask something else
//            let shouldHighlight = isModifierToggle ? toggleState : isHighlighted
            if !isHighlighted && toggleState {
                // don't update the highlught
            } else {
                backgroundColor = isHighlighted ? .white : .clear
            }
        }
    }
    
    override open var isSelected: Bool {
        didSet {
            let shouldHighlight = key.isModifier ? toggleState : isSelected
            backgroundColor = shouldHighlight ? .red : .clear
        }
    }
    
    required init(key: KeyCoded) {
        self.key = key
        super.init(frame: .zero)
    }
    required init?(coder aDecoder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
}

@objc protocol EmulatorKeyboardKeyPressedDelegate: class {
    func keyDown(_ key: KeyCoded)
    func keyUp(_ key: KeyCoded)
}

@objc protocol EmulatorKeyboardModifierPressedDelegate: class {
    func modifierPressedWithKey(_ key: KeyCoded, enable: Bool)
    func isModifierEnabled(key: KeyCoded) -> Bool
}

protocol EmulatorKeyboardViewDelegate: class {
    func toggleAlternateKeys()
    func refreshModifierStates()
}

class EmulatorKeyboardView: UIView {
    
    var viewModel = EmulatorKeyboardViewModel(keys: [[KeyCoded]]()) {
        didSet {
            setupWithModel(viewModel)
        }
    }
    var modifierButtons = Set<KeyboardButton>()
    
    weak var delegate: EmulatorKeyboardViewDelegate?
    
    private lazy var keyRowsStackView: UIStackView = {
       let stackView = UIStackView()
       stackView.translatesAutoresizingMaskIntoConstraints = false
       stackView.axis = .vertical
       stackView.distribution = .equalCentering
       stackView.spacing = 12
       return stackView
    }()
    
    private lazy var alternateKeyRowsStackView: UIStackView = {
        let stackView = UIStackView()
        stackView.translatesAutoresizingMaskIntoConstraints = false
        stackView.axis = .vertical
        stackView.distribution = .equalCentering
        stackView.spacing = 12
        stackView.isHidden = true
        return stackView
    }()
    
    let dragMeView: UILabel = {
        let label = UILabel(frame: .zero)
        label.translatesAutoresizingMaskIntoConstraints = false
        label.text = "DRAG ME!"
        label.textColor = UIColor.systemRed
        return label
    }()
    
    private var pressedKeyLabels = [String: UILabel]()
    
    convenience init() {
        self.init(frame: CGRect.zero)
    }

    override init(frame: CGRect) {
        super.init(frame: frame)
        commonInit()
    }

    required init?(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder)
        commonInit()
    }

    private func commonInit() {
        backgroundColor = .clear
        layer.borderColor = UIColor.white.cgColor
        layer.borderWidth = 1.0
        layer.cornerRadius = 15.0
        layoutMargins = UIEdgeInsets(top: 16, left: 4, bottom: 16, right: 4)
        insetsLayoutMarginsFromSafeArea = false
        addSubview(keyRowsStackView)
        keyRowsStackView.topAnchor.constraint(equalTo: layoutMarginsGuide.topAnchor).isActive = true
        keyRowsStackView.leadingAnchor.constraint(equalTo: layoutMarginsGuide.leadingAnchor, constant: 4.0).isActive = true
        keyRowsStackView.trailingAnchor.constraint(equalTo: layoutMarginsGuide.trailingAnchor, constant: -4.0).isActive = true
        addSubview(alternateKeyRowsStackView)
        alternateKeyRowsStackView.topAnchor.constraint(equalTo: layoutMarginsGuide.topAnchor).isActive = true
        alternateKeyRowsStackView.leadingAnchor.constraint(equalTo: layoutMarginsGuide.leadingAnchor, constant: 4.0).isActive = true
        alternateKeyRowsStackView.trailingAnchor.constraint(equalTo: layoutMarginsGuide.trailingAnchor, constant: -4.0).isActive = true
        addSubview(dragMeView)
        dragMeView.centerXAnchor.constraint(equalTo: centerXAnchor).isActive = true
        dragMeView.bottomAnchor.constraint(equalTo: bottomAnchor, constant: -8.0).isActive = true
    }
    
    
    @objc private func keyPressed(_ sender: KeyboardButton) {
        if sender.key.keyCode == AppleKeyboardKey.KEY_SPECIAL_TOGGLE.rawValue {
            return
        }
        if !sender.key.isModifier {
            let label = UILabel(frame: .zero)
            label.text = sender.titleLabel?.text
            // hmm need to convert frame
            let converted = sender.convert(sender.bounds, to: self)
            print("sender frame: \(sender.frame), bounds: \(sender.bounds), convertedBounds = \(converted)")
            var labelFrame = converted.offsetBy(dx: 0, dy: -60)
            labelFrame = CGRect(x: labelFrame.origin.x, y: labelFrame.origin.y, width: labelFrame.width * 2, height: labelFrame.height * 2)
            label.backgroundColor = .purple
            label.textColor = .green
            label.frame = labelFrame
            addSubview(label)
            pressedKeyLabels[label.text ?? "😭"] = label
        }
        viewModel.keyPressed(sender.key)
    }
    
    @objc private func keyReleased(_ sender: KeyboardButton) {
        if sender.key.keyCode == AppleKeyboardKey.KEY_SPECIAL_TOGGLE.rawValue {
            delegate?.toggleAlternateKeys()
            return
        }
        let title = sender.titleLabel?.text ?? "😭"
        if let label = pressedKeyLabels[title] {
            label.removeFromSuperview()
            pressedKeyLabels.removeValue(forKey: title)
        }
        let modifierState = viewModel.modifierKeyToggleStateForKey(sender.key)
        sender.toggleState = modifierState
        sender.isSelected = modifierState
        viewModel.keyReleased(sender.key)
        self.delegate?.refreshModifierStates()
    }
    
    func setupWithModel(_ model: EmulatorKeyboardViewModel) {
        for row in model.keys {
            let keysInRow = createKeyRow(keys: row)
            keyRowsStackView.addArrangedSubview(keysInRow)
        }
        if let altKeys = model.alternateKeys {
            for row in altKeys {
                let keysInRow = createKeyRow(keys: row)
                alternateKeyRowsStackView.addArrangedSubview(keysInRow)
            }
        }
    }
    
    func toggleKeysStackView() {
        if viewModel.alternateKeys != nil {
            keyRowsStackView.isHidden.toggle()
            alternateKeyRowsStackView.isHidden.toggle()
            refreshModifierStates()
        }
    }
    
    func refreshModifierStates() {
        modifierButtons.forEach{ button in
            button.toggleState = viewModel.modifierKeyToggleStateForKey(button.key)
            button.isSelected = button.toggleState
        }
    }
    
    private func createKey(_ keyCoded: KeyCoded) -> UIButton {
        let key = KeyboardButton(key: keyCoded)
        if let imageName = keyCoded.keyImageName {
            key.setImage(UIImage(systemName: imageName), for: .normal)
            if let highlightedImageName = keyCoded.keyImageNameHighlighted {
                key.setImage(UIImage(systemName: highlightedImageName), for: .highlighted)
                key.setImage(UIImage(systemName: highlightedImageName), for: .selected)
            }
        } else {
            key.setTitle(keyCoded.keyLabel, for: .normal)
            key.titleLabel?.font = UIFont.systemFont(ofSize: 12.0)
            key.setTitleColor(.white, for: .normal)
            key.setTitleColor(.black, for: .highlighted)
        }
        key.translatesAutoresizingMaskIntoConstraints = false
        key.widthAnchor.constraint(equalToConstant: (25 * CGFloat(keyCoded.keySize.rawValue))).isActive = true
        key.heightAnchor.constraint(equalToConstant: 25).isActive = true
        key.layer.borderWidth = 1.0
        key.layer.borderColor = UIColor.white.cgColor
        key.layer.cornerRadius = 6.0
        key.addTarget(self, action: #selector(keyPressed(_:)), for: .touchDown)
        key.addTarget(self, action: #selector(keyReleased(_:)), for: .touchUpInside)
        key.addTarget(self, action: #selector(keyReleased(_:)), for: .touchUpOutside)
        if keyCoded.isModifier {
            modifierButtons.update(with: key)
        }
        return key
    }

    private func createKeyRow(keys: [KeyCoded]) -> UIStackView {
        let subviews: [UIView] = keys.enumerated().map { index, keyCoded -> UIView in
            if keyCoded is SpacerKey {
                let spacer = UIView()
                spacer.widthAnchor.constraint(equalToConstant: 25.0 * CGFloat(keyCoded.keySize.rawValue)).isActive = true
                spacer.heightAnchor.constraint(equalToConstant: 25.0).isActive = true
                return spacer
            }
            return createKey(keyCoded)
        }
        let stack = UIStackView(arrangedSubviews: subviews)
        stack.axis = .horizontal
        stack.distribution = .fill
        stack.spacing = 8
        return stack
    }
}

@objc enum KeySize: Int {
    case standard = 1, wide, wider
}

// represents a key that has an underlying code that gets sent to the emulator
@objc protocol KeyCoded: AnyObject {
    var keyLabel: String { get }
    var keyImageName: String? { get }
    var keyImageNameHighlighted: String? { get }
    var keyCode: Int { get }
    var keySize: KeySize { get }
    var isModifier: Bool { get }
}

protocol KeyRowsDataSource {
    func keyForPositionAt(_ position: KeyPosition) -> KeyCoded?
}

@objc class AppleIIKey: NSObject, KeyCoded {
    let keyLabel: String
    var keyImageName: String?
    var keyImageNameHighlighted: String?
    let keyCode: Int
    let keySize: KeySize
    let isModifier: Bool
    
    override var description: String {
        return String(format: "\(keyLabel) (%02X)", keyCode)
    }
    init(label: String, code: Int, keySize: KeySize = .standard, isModifier: Bool = false, imageName: String? = nil, imageNameHighlighted: String? = nil)  {
        self.keyLabel = label
        self.keyCode = code
        self.keySize = keySize
        self.isModifier = isModifier
        self.keyImageName = imageName
        self.keyImageNameHighlighted = imageNameHighlighted
    }
}

class SpacerKey: KeyCoded {
    let keyLabel = ""
    let keyCode = 0
    let keySize: KeySize
    let isModifier = false
    let keyImageName: String? = nil
    let keyImageNameHighlighted: String? = nil
    init(keySize: KeySize = .standard) {
        self.keySize = keySize
    }
}

struct KeyPosition {
    let row: Int
    let column: Int
}

@objc class EmulatorKeyboardViewModel: NSObject, KeyRowsDataSource {
    var keys = [[KeyCoded]]()
    var alternateKeys: [[KeyCoded]]?
    var modifiers: [Int16: KeyCoded]?
    
    @objc weak var delegate: EmulatorKeyboardKeyPressedDelegate?
    @objc weak var modifierDelegate: EmulatorKeyboardModifierPressedDelegate?
    
    init(keys: [[KeyCoded]], alternateKeys: [[KeyCoded]]? = nil) {
        self.keys = keys
        self.alternateKeys = alternateKeys
    }
    
    func createView() -> EmulatorKeyboardView {
        let view = EmulatorKeyboardView()
        view.viewModel = self
        return view
    }
    
    func keyForPositionAt(_ position: KeyPosition) -> KeyCoded? {
        guard position.row < keys.count else {
            return nil
        }
        let row = keys[position.row]
        guard position.column < row.count else {
            return nil
        }
        return row[position.column]
    }
    
    func modifierKeyToggleStateForKey(_ key: KeyCoded) -> Bool {
        return key.isModifier && (modifierDelegate?.isModifierEnabled(key: key) ?? false)
    }
    
    func keyPressed(_ key: KeyCoded) {
        if key.isModifier {
            let isPressed = modifierDelegate?.isModifierEnabled(key: key) ?? false
            modifierDelegate?.modifierPressedWithKey(key, enable: !isPressed)
            return
        }
        delegate?.keyDown(key)
    }
    
    func keyReleased(_ key: KeyCoded) {
        if key.isModifier {
            return
        }
        delegate?.keyUp(key)
    }
    
    // KeyCoded can support a shifted key label
    // view can update with shifted key labels?
    // cluster can support alternate keys and view can swap them out?
}

@objc class EmulatorKeyboardController: UIViewController {
    @objc lazy var leftKeyboardView: EmulatorKeyboardView = {
        let view = leftKeyboardModel.createView()
        view.delegate = self
        return view
    }()
    @objc lazy var rightKeyboardView: EmulatorKeyboardView = {
        let view = rightKeyboardModel.createView()
        view.delegate = self
        return view
    }()
    var keyboardConstraints = [NSLayoutConstraint]()
    
    // Global states for all the keyboards
    // uses bitwise masks for the state of shift keys, control, open-apple keys, etc
    @objc var modifierState: Int16 = 0
    
    @objc let leftKeyboardModel = EmulatorKeyboardViewModel(
        keys:
        [
            [
                AppleIIKey(label: "esc", code: AppleKeyboardKey.KEY_ESC.rawValue),
                AppleIIKey(label: "tab", code: AppleKeyboardKey.KEY_TAB.rawValue, keySize: .wide),
                SpacerKey(),
                AppleIIKey(label: "CTRL", code: AppleKeyboardKey.KEY_CTRL.rawValue,
                           keySize: .standard, isModifier: true, imageName: "control")
            ],
            [
                AppleIIKey(label: "q", code: AppleKeyboardKey.KEY_Q.rawValue),
                AppleIIKey(label: "w", code: AppleKeyboardKey.KEY_W.rawValue),
                AppleIIKey(label: "e", code: AppleKeyboardKey.KEY_E.rawValue),
                AppleIIKey(label: "r", code: AppleKeyboardKey.KEY_R.rawValue),
                AppleIIKey(label: "t", code: AppleKeyboardKey.KEY_T.rawValue)
            ],
            [
                AppleIIKey(label: "a", code: AppleKeyboardKey.KEY_A.rawValue),
                AppleIIKey(label: "s", code: AppleKeyboardKey.KEY_S.rawValue),
                AppleIIKey(label: "d", code: AppleKeyboardKey.KEY_D.rawValue),
                AppleIIKey(label: "f", code: AppleKeyboardKey.KEY_F.rawValue),
                AppleIIKey(label: "g", code: AppleKeyboardKey.KEY_G.rawValue)
            ],
            [
                AppleIIKey(label: "z", code: AppleKeyboardKey.KEY_Z.rawValue),
                AppleIIKey(label: "x", code: AppleKeyboardKey.KEY_X.rawValue),
                AppleIIKey(label: "c", code: AppleKeyboardKey.KEY_C.rawValue),
                AppleIIKey(label: "v", code: AppleKeyboardKey.KEY_V.rawValue),
                AppleIIKey(label: "b", code: AppleKeyboardKey.KEY_B.rawValue)
            ],
            [
                AppleIIKey(label: "SHIFT", code: AppleKeyboardKey.KEY_SHIFT.rawValue,
                           keySize: .wide, isModifier: true, imageName: "shift", imageNameHighlighted: "shift.fill"),
                AppleIIKey(label: "123", code: AppleKeyboardKey.KEY_SPECIAL_TOGGLE.rawValue, keySize: .wide, imageName: "textformat.123"),
                AppleIIKey(label: "", code: AppleKeyboardKey.KEY_APPLE.rawValue,
                           keySize: .standard, isModifier: true)
            ],
            [
                AppleIIKey(label: "SPACE", code: AppleKeyboardKey.KEY_SPACE.rawValue)
            ]
        ],
        alternateKeys:
        [
            [],
            [
                AppleIIKey(label: "1", code: AppleKeyboardKey.KEY_1.rawValue),
                AppleIIKey(label: "2", code: AppleKeyboardKey.KEY_2.rawValue),
                AppleIIKey(label: "3", code: AppleKeyboardKey.KEY_3.rawValue),
                AppleIIKey(label: "4", code: AppleKeyboardKey.KEY_4.rawValue),
                AppleIIKey(label: "5", code: AppleKeyboardKey.KEY_5.rawValue),
            ],
            [
                AppleIIKey(label: "-", code: AppleKeyboardKey.KEY_MINUS.rawValue),
                AppleIIKey(label: "=", code: AppleKeyboardKey.KEY_EQUALS.rawValue),
                AppleIIKey(label: "/", code: AppleKeyboardKey.KEY_FSLASH.rawValue),
                AppleIIKey(label: "[", code: AppleKeyboardKey.KEY_LEFT_BRACKET.rawValue),
                AppleIIKey(label: "]", code: AppleKeyboardKey.KEY_RIGHT_BRACKET.rawValue),
            ],
            [
                AppleIIKey(label: ";", code: AppleKeyboardKey.KEY_SEMICOLON.rawValue),
                AppleIIKey(label: "~", code: AppleKeyboardKey.KEY_TILDE.rawValue)
            ],
            [
                AppleIIKey(label: "SHIFT", code: AppleKeyboardKey.KEY_SHIFT.rawValue,
                           keySize: .wide, isModifier: true, imageName: "shift", imageNameHighlighted: "shift.fill"),
                AppleIIKey(label: "ABC", code: AppleKeyboardKey.KEY_SPECIAL_TOGGLE.rawValue, keySize: .wide, imageName: "textformat.abc"),
                AppleIIKey(label: "OPT", code: AppleKeyboardKey.KEY_OPTION.rawValue,
                           keySize: .standard, isModifier: true)
            ],
            [
                AppleIIKey(label: "SPACE", code: AppleKeyboardKey.KEY_SPACE.rawValue)
            ]
        ]
    )
    @objc let rightKeyboardModel = EmulatorKeyboardViewModel(
        keys:
        [
            [
                SpacerKey(),
                SpacerKey(),
                SpacerKey(),
                AppleIIKey(label: "RESET", code: AppleKeyboardKey.KEY_RESET.rawValue, keySize: .wide)
            ],
            [
                AppleIIKey(label: "y", code: AppleKeyboardKey.KEY_Y.rawValue),
                AppleIIKey(label: "u", code: AppleKeyboardKey.KEY_U.rawValue),
                AppleIIKey(label: "i", code: AppleKeyboardKey.KEY_I.rawValue),
                AppleIIKey(label: "o", code: AppleKeyboardKey.KEY_O.rawValue),
                AppleIIKey(label: "p", code: AppleKeyboardKey.KEY_P.rawValue)
            ],
            [
                AppleIIKey(label: "h", code: AppleKeyboardKey.KEY_H.rawValue),
                AppleIIKey(label: "j", code: AppleKeyboardKey.KEY_J.rawValue),
                AppleIIKey(label: "k", code: AppleKeyboardKey.KEY_K.rawValue),
                AppleIIKey(label: "l", code: AppleKeyboardKey.KEY_L.rawValue),
                AppleIIKey(label: "'", code: AppleKeyboardKey.KEY_SQUOTE.rawValue)
            ],
            [
                AppleIIKey(label: "n", code: AppleKeyboardKey.KEY_N.rawValue),
                AppleIIKey(label: "m", code: AppleKeyboardKey.KEY_M.rawValue),
                AppleIIKey(label: ",", code: AppleKeyboardKey.KEY_COMMA.rawValue),
                AppleIIKey(label: ".", code: AppleKeyboardKey.KEY_PERIOD.rawValue),
                AppleIIKey(label: "DELETE", code: AppleKeyboardKey.KEY_DELETE.rawValue, imageName: "delete.left", imageNameHighlighted: "delete.left.fill")
            ],
            [
                AppleIIKey(label: "RETURN", code: AppleKeyboardKey.KEY_RETURN.rawValue, keySize: .wide)
            ]
        ],
        alternateKeys:
        [
            [
                AppleIIKey(label: "6", code: AppleKeyboardKey.KEY_6.rawValue),
                AppleIIKey(label: "7", code: AppleKeyboardKey.KEY_7.rawValue),
                AppleIIKey(label: "8", code: AppleKeyboardKey.KEY_8.rawValue),
                AppleIIKey(label: "9", code: AppleKeyboardKey.KEY_9.rawValue),
                AppleIIKey(label: "0", code: AppleKeyboardKey.KEY_0.rawValue),
            ],
            [
                SpacerKey(),
                SpacerKey(),
                AppleIIKey(label: "⬆", code: AppleKeyboardKey.KEY_UP_CURSOR.rawValue),
                SpacerKey(),
                SpacerKey()
            ],
            [
                SpacerKey(),
                AppleIIKey(label: "⬅", code: AppleKeyboardKey.KEY_LEFT_CURSOR.rawValue),
                AppleIIKey(label: "⬇", code: AppleKeyboardKey.KEY_DOWN_CURSOR.rawValue),
                AppleIIKey(label: "➡", code: AppleKeyboardKey.KEY_RIGHT_CURSOR.rawValue),
                SpacerKey()
            ],
            [
                AppleIIKey(label: "SPACE", code: AppleKeyboardKey.KEY_SPACE.rawValue, keySize: .wide),
                AppleIIKey(label: "SHIFT", code: AppleKeyboardKey.KEY_SHIFT.rawValue,
                           keySize: .standard, isModifier: true, imageName: "shift", imageNameHighlighted: "shift.fill"),
                AppleIIKey(label: "DELETE", code: AppleKeyboardKey.KEY_DELETE.rawValue, imageName: "delete.left", imageNameHighlighted: "delete.left.fill")
            ],
            [
                AppleIIKey(label: "RETURN", code: AppleKeyboardKey.KEY_RETURN.rawValue, keySize: .standard)
            ]
        ]
    )
    
    init() {
        super.init(nibName: nil, bundle: nil)
    }
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }

    override func viewDidLoad() {
        super.viewDidLoad()
//        setupView()
        setupViewFrames()
//        setupKeyModels()
        
        let panGesture = UIPanGestureRecognizer(target: self, action: #selector(draggedView(_:)))
        leftKeyboardView.dragMeView.isUserInteractionEnabled = true
        leftKeyboardView.dragMeView.addGestureRecognizer(panGesture)
        let panGestureRightKeyboard = UIPanGestureRecognizer(target: self, action: #selector(draggedView(_:)))
        rightKeyboardView.dragMeView.isUserInteractionEnabled = true
        rightKeyboardView.dragMeView.addGestureRecognizer(panGestureRightKeyboard)
    }
    
    func setupView() {
        NSLayoutConstraint.deactivate(keyboardConstraints)
        keyboardConstraints.removeAll()
        leftKeyboardView.translatesAutoresizingMaskIntoConstraints = false
        view.addSubview(leftKeyboardView)
        leftKeyboardView.heightAnchor.constraint(equalToConstant: 200).isActive = true
        leftKeyboardView.widthAnchor.constraint(equalToConstant: 180).isActive = true
        keyboardConstraints.append(contentsOf: [
            leftKeyboardView.leadingAnchor.constraint(equalTo: view.safeAreaLayoutGuide.leadingAnchor),
            leftKeyboardView.centerYAnchor.constraint(equalTo: view.centerYAnchor)
        ])
        rightKeyboardView.translatesAutoresizingMaskIntoConstraints = false
        view.addSubview(rightKeyboardView)
        keyboardConstraints.append(contentsOf: [
            rightKeyboardView.trailingAnchor.constraint(equalTo: view.safeAreaLayoutGuide.trailingAnchor),
            rightKeyboardView.centerYAnchor.constraint(equalTo: view.centerYAnchor),
        ])
        rightKeyboardView.heightAnchor.constraint(equalToConstant: 200).isActive = true
        rightKeyboardView.widthAnchor.constraint(equalToConstant: 180).isActive = true
        NSLayoutConstraint.activate(keyboardConstraints)
    }
    
    func setupViewFrames() {
        // initial placement on the bottom corners
        // since we don't know the frame of this view yet until layout time,
        // assume it's taking the full screen
        let screenFrame = UIScreen.main.bounds
        let keyboardHeight: CGFloat = 240.0
        let keyboardWidth: CGFloat = 173.0
        let bottomLeftFrame = CGRect(
            x: 20,
            y: screenFrame.size.height - 40 - keyboardHeight - 20,
            width: keyboardWidth, height: keyboardHeight)
        let bottomRightFrame = CGRect(
            x: screenFrame.size.width - 20 - keyboardWidth,
            y:screenFrame.size.height - 40 - keyboardHeight - 20,
            width: keyboardWidth, height: keyboardHeight
        )
        view.addSubview(leftKeyboardView)
        view.addSubview(rightKeyboardView)
        leftKeyboardView.frame = bottomLeftFrame
        rightKeyboardView.frame = bottomRightFrame
    }
    
    override func viewWillTransition(to size: CGSize, with coordinator: UIViewControllerTransitionCoordinator) {
        // get relative positions of frames to size
        for v in [leftKeyboardView, rightKeyboardView] {
            let xPercent = v.frame.origin.x / view.frame.size.width
            let yPercent = v.frame.origin.y / view.frame.size.height
            var newX = size.width * xPercent
            var newY = size.height * yPercent
            // mmm need to check if the views fit within the frame and adjust
            if newX + v.bounds.size.width > size.width {
                newX = size.width - v.bounds.size.width
            } else if newX < 0 {
                newX = 0
            }
            if newY + v.bounds.size.height > size.height {
                newY = size.height - v.bounds.size.height
            }
            let newFrame = CGRect(x: newX, y: newY, width: v.bounds.size.width, height: v.bounds.size.height)
            v.frame = newFrame
        }
    }
    
    func setupKeyModels() {
        leftKeyboardView.setupWithModel(leftKeyboardModel)
        rightKeyboardView.setupWithModel(rightKeyboardModel)
    }
    
    @objc func draggedView(_ sender:UIPanGestureRecognizer){
        guard let keyboardView = sender.view?.superview else {
            return
        }
//        NSLayoutConstraint.deactivate(keyboardConstraints)
//        self.view.bringSubviewToFront(keyboardView)
        let translation = sender.translation(in: self.view)
        keyboardView.center = CGPoint(x: keyboardView.center.x + translation.x, y: keyboardView.center.y + translation.y)
        sender.setTranslation(CGPoint.zero, in: self.view)
    }
}

extension EmulatorKeyboardController: EmulatorKeyboardViewDelegate {
    func toggleAlternateKeys() {
        for keyboard in [leftKeyboardView, rightKeyboardView] {
            keyboard.toggleKeysStackView()
        }
    }
    func refreshModifierStates() {
        for keyboard in [leftKeyboardView, rightKeyboardView] {
            keyboard.refreshModifierStates()
        }
    }
}
