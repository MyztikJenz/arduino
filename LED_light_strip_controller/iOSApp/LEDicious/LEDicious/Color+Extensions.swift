//
//  Color+Extensions.swift
//  LEDicious
//
//  Created by Jim Turner on 8/22/20.
//

import SwiftUI
import Foundation

extension Color {
    var hexValue: String {
        if let comps = self.components {
            return String(format:"0x%02X%02X%02X", Int(comps.red * 255), Int(comps.green * 255), Int(comps.blue * 255))
        }
        else {
            return "0x000000"
        }
    }
    
    var components: (red: CGFloat, green: CGFloat, blue: CGFloat, alpha: CGFloat)? {
        var r: CGFloat = 0
        var g: CGFloat = 0
        var b: CGFloat = 0
        var a: CGFloat = 0
        
        guard UIColor(self).getRed(&r, green: &g, blue: &b, alpha: &a) else {
            return nil
        }
        // Normalize to 0.0 -> 1.0
        func _mm(_ v:CGFloat) -> CGFloat {
            return min(max(0.0, v), 1.0)
        }
        return (_mm(r), _mm(g) ,_mm(b) , _mm(a))
    }
    
    /// This expects a 24bit hex value. Alpha is not accounted for
    public init?(from hexValue: String) {
        var r = 0.0
        var g = 0.0
        var b = 0.0
        
        let substring:Substring
        if hexValue.hasPrefix("0x") {
            substring = hexValue[hexValue.index(hexValue.startIndex, offsetBy: 2) ..< hexValue.endIndex]
        }
        else if hexValue.hasPrefix("#") {
            substring = hexValue[hexValue.index(hexValue.startIndex, offsetBy: 1) ..< hexValue.endIndex]
        }
        else {
            return nil
        }

        let scanner = Scanner(string: String(substring))
        var hexNumber : UInt64 = 0
        if scanner.scanHexInt64(&hexNumber) {
            r = Double((hexNumber & 0x00FF0000) >> 16) / 255.0
            g = Double((hexNumber & 0x0000FF00) >> 8) / 255.0
            b = Double((hexNumber & 0x000000FF) >> 0) / 255.0
        }
        else {
            return nil
        }
        
        self.init(.sRGB, red: r, green: g, blue: b, opacity: 1.0)
    }
}

