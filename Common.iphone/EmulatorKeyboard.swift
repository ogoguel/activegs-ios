//
//  EmulatorKeyboard.swift
//  ActiveGS
//
//  Created by Yoshi Sugawara on 7/30/20.
//

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
                backgroundColor = isHighlighted ? .white : .black
            }
        }
    }
    
    override open var isSelected: Bool {
        didSet {
            let shouldHighlight = key.isModifier ? toggleState : isSelected
            backgroundColor = shouldHighlight ? .red : .black
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

class EmulatorKeyboardView: UIView {
    
    var viewModel = EmulatorKeyboardViewModel(keys: [[KeyCoded]]()) {
        didSet {
            setupWithModel(viewModel)
        }
    }
    
    private lazy var keyRowsStackView: UIStackView = {
       let stackView = UIStackView()
       stackView.translatesAutoresizingMaskIntoConstraints = false
       stackView.axis = .vertical
       stackView.distribution = .equalCentering
       stackView.spacing = 16
       return stackView
    }()
    
    private lazy var alternateKeyRowsStackView: UIStackView = {
        let stackView = UIStackView()
        stackView.translatesAutoresizingMaskIntoConstraints = false
        stackView.axis = .vertical
        stackView.distribution = .equalCentering
        stackView.spacing = 16
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
        keyRowsStackView.leadingAnchor.constraint(equalTo: layoutMarginsGuide.leadingAnchor).isActive = true
        keyRowsStackView.trailingAnchor.constraint(equalTo: layoutMarginsGuide.trailingAnchor).isActive = true
        addSubview(dragMeView)
        dragMeView.centerXAnchor.constraint(equalTo: centerXAnchor).isActive = true
        dragMeView.bottomAnchor.constraint(equalTo: bottomAnchor, constant: -8.0).isActive = true
    }
    
    
    @objc private func keyPressed(_ sender: KeyboardButton) {
        if viewModel.shouldShowKeyPressedVisualFeedbackForKey(sender.key) {
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
        let title = sender.titleLabel?.text ?? "😭"
        if let label = pressedKeyLabels[title] {
            label.removeFromSuperview()
            pressedKeyLabels.removeValue(forKey: title)
        }
        let modifierState = viewModel.modifierKeyToggleStateForKey(sender.key)
        sender.toggleState = modifierState
        sender.isSelected = modifierState
        viewModel.keyReleased(sender.key)
    }
    
    func setupWithModel(_ model: EmulatorKeyboardViewModel) {
        for row in model.keys {
            let keysInRow = createKeyRow(keys: row)
            keyRowsStackView.addArrangedSubview(keysInRow)
        }
    }
    
    private func createKey(_ keyCoded: KeyCoded) -> UIButton {
        let testKey = KeyboardButton(key: keyCoded)
        testKey.setTitle(keyCoded.keyLabel, for: .normal)
        testKey.titleLabel?.font = UIFont.systemFont(ofSize: 12.0)
        testKey.setTitleColor(.white, for: .normal)
        testKey.setTitleColor(.black, for: .highlighted)
        testKey.translatesAutoresizingMaskIntoConstraints = false
        testKey.widthAnchor.constraint(equalToConstant: (25 * CGFloat(keyCoded.keySize.rawValue))).isActive = true
        testKey.heightAnchor.constraint(equalToConstant: 25).isActive = true
        testKey.layer.borderWidth = 1.0
        testKey.layer.borderColor = UIColor.white.cgColor
        testKey.addTarget(self, action: #selector(keyPressed(_:)), for: .touchDown)
        testKey.addTarget(self, action: #selector(keyReleased(_:)), for: .touchUpInside)
        testKey.addTarget(self, action: #selector(keyReleased(_:)), for: .touchUpOutside)
        return testKey
    }

    private func createKeyRow(keys: [KeyCoded]) -> UIStackView {
        let spacer = UIView()
        var subviews: [UIView] = keys.enumerated().map { index, keyCoded -> UIView in
            createKey(keyCoded)
        }
        subviews.append(spacer)
        let stack = UIStackView(arrangedSubviews: subviews)
        stack.axis = .horizontal
        stack.distribution = .fill
        stack.spacing = 8
        return stack
    }
}

// represents a key that has an underlying code that gets sent to the emulator
@objc enum KeySize: Int {
    case standard = 1, wide, wider
}

@objc protocol KeyCoded: AnyObject {
    var keyLabel: String { get }
    var keyCode: Int { get }
    var keySize: KeySize { get }
    var isModifier: Bool { get }
}

protocol KeyRowsDataSource {
    func keyForPositionAt(_ position: KeyPosition) -> KeyCoded?
}

@objc class AppleIIKey: NSObject, KeyCoded {
    let keyLabel: String
    let keyCode: Int
    let keySize: KeySize
    let isModifier: Bool
    
    override var description: String {
        return String(format: "\(keyLabel) (%02X)", keyCode)
    }
    init(label: String, code: Int, keySize: KeySize, isModifier: Bool = false)  {
        self.keyLabel = label
        self.keyCode = code
        self.keySize = keySize
        self.isModifier = isModifier
    }
    convenience init(label: String, code: Int) {
        self.init(label: label, code: code, keySize: .standard)
    }
}

struct KeyPosition {
    let row: Int
    let column: Int
}

@objc class EmulatorKeyboardViewModel: NSObject, KeyRowsDataSource {
    var keys = [[KeyCoded]]()
    var alternateKeys: [[KeyCoded]]?
    var modifierState: Int16 = 0
    
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
    
    func shouldShowKeyPressedVisualFeedbackForKey(_ key: KeyCoded) -> Bool {
        return !key.isModifier
    }
    
    func modifierKeyToggleStateForKey(_ key: KeyCoded) -> Bool {
        return key.isModifier && (modifierDelegate?.isModifierEnabled(key: key) ?? false)
    }
    
    func keyPressed(_ key: KeyCoded) {
        if key.isModifier {
            let isPressed = modifierDelegate?.isModifierEnabled(key: key) ?? false
            modifierDelegate?.modifierPressedWithKey(key, enable: !isPressed)
        }
        delegate?.keyDown(key)
    }
    
    func keyReleased(_ key: KeyCoded) {
        delegate?.keyUp(key)

    }
    
    // KeyCoded can support a shifted key label
    // view can update with shifted key labels?
    // cluster can support alternate keys and view can swap them out?
}

@objc class EmulatorKeyboardController: UIViewController {
    @objc lazy var leftKeyboardView: EmulatorKeyboardView = {
        leftKeyboardModel.createView()
    }()
    @objc lazy var rightKeyboardView: EmulatorKeyboardView = {
        rightKeyboardModel.createView()
    }()
    var keyboardConstraints = [NSLayoutConstraint]()
    
    // uses bitwise masks for the state of shift keys, control, open-apple keys, etc
    @objc var modifierState: Int16 = 0
    
    @objc let leftKeyboardModel = EmulatorKeyboardViewModel(keys:
        [
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
                AppleIIKey(label: "SHIFT", code: AppleKeyboardKey.KEY_SHIFT.rawValue, keySize: .wide, isModifier: true),
                AppleIIKey(label: "SPACE", code: AppleKeyboardKey.KEY_SPACE.rawValue, keySize: .wider)
            ]
        ]
    )
    @objc let rightKeyboardModel = EmulatorKeyboardViewModel(keys:
        [
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
                AppleIIKey(label: "/", code: AppleKeyboardKey.KEY_FSLASH.rawValue)
            ],
            [
                AppleIIKey(label: "RETURN", code: AppleKeyboardKey.KEY_RETURN.rawValue, keySize: .wider)
            ]
        ]
    )
    var arrowKeysModel = [[AppleIIKey]]()
    
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
        // assume a size of 200x180 for now
        // since we don't know the frame of this view yet until layout time,
        // assume it's taking the full screen
        let screenFrame = UIScreen.main.bounds
        let keyboardHeight: CGFloat = 180.0
        let keyboardWidth: CGFloat = 200.0
        let bottomLeftFrame = CGRect(
            x: self.view.safeAreaInsets.left,
            y: screenFrame.size.height - self.view.safeAreaInsets.bottom - keyboardHeight - 20,
            width: keyboardWidth, height: keyboardHeight)
        let bottomRightFrame = CGRect(
            x: screenFrame.size.width - self.view.safeAreaInsets.right - keyboardWidth,
            y:screenFrame.size.height - self.view.safeAreaInsets.bottom - keyboardHeight - 20,
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
