//
//  LEDServer.swift
//  LEDicious
//
//  Created by Jim Turner on 8/22/20.
//

import Foundation
import SwiftUI

enum Effect : String, CaseIterable {
    case none
    case trails
    case somethingElse
    
//    func toString() -> String {
//        return "\(self)"
//    }
    
    static func allEffects() -> [String] {
        return Effect.allCases.map { $0.rawValue }
    }
}

class LEDServer : ObservableObject {
    @Published var color: Color = Color(.sRGB, red: 1.0, green: 0.0, blue: 0.0, opacity: 1.0) {
        didSet {
            self.hexValue = self.color.hexValue
        }
    }
    @Published var hexValue: String = "0xFF0000"
    @Published var currentEffect: Effect = .none
    @Published var brightness: Double = 0.5 // 0.0 <-> 1.0
}
