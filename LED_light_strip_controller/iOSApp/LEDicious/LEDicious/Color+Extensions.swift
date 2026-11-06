//
//  Color+Extensions.swift
//  LEDicious
//
//  Created by Jim Turner on 8/22/20.
//

import SwiftUI

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
}

