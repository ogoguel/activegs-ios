//
//  PreviewUI.swift
//  activegs
//
//  Created by Yoshi Sugawara on 1/9/21.
//

import SwiftUI

struct ContentView: View {
    var body: some View {
        //Text("Hello, World!")
        IntegratedController()
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        Group {
            ContentView().previewLayout(.fixed(width: 568, height: 320))
        }
    }
}

struct IntegratedController: UIViewControllerRepresentable {
    
    func makeUIViewController(context: UIViewControllerRepresentableContext<IntegratedController>) -> EmulatorKeyboardController {
        let controller = EmulatorKeyboardController()
        controller.view.backgroundColor = .black
        return controller
    }
    
    func updateUIViewController(_ uiViewController: EmulatorKeyboardController, context: UIViewControllerRepresentableContext<IntegratedController>) {
        
    }
}

