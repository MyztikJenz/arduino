//
//  LEDServer.swift
//  LEDicious
//
//  Created by Jim Turner on 8/22/20.
//

import Foundation
import SwiftUI

class LEDServer : ObservableObject {
//    static let sharedModel: Model = {
//        return Model()
//    }()
    @Published var color: Color = Color(.sRGB, red: 1.0, green: 0.0, blue: 0.0, opacity: 1.0) {
        didSet {
            self.hexValue = self.color.hexValue
        }
    }
    @Published var hexValue: String = "0xFF0000"
    
}
