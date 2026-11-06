//
//  LEDServer.swift
//  LEDicious
//
//  Created by Jim Turner on 8/22/20.
//

import SwiftUI

struct LEDServerDetail: View {
    var conn: NetworkDoodad? = nil
    @ObservedObject var ledServer:LEDServer
    
    var body: some View {
        VStack {
            HStack {
                Spacer()
                VStack {
                    Button("On") {
                        print("on")
                    }
                    .padding()
                    Button("Off") {
                        print("off")
                    }
                    .padding()
                }
                Spacer()
                ColorPicker(selection: $ledServer.color, supportsOpacity: false) {
                }
                .scaleEffect(4.0)
                .frame(width: 1, height: 128) // without a frame, it won't render right. I have no idea why. Height needs to be close to accurate or layout will overlap the picker well.
                Spacer()
            }
            HStack {
                Text("Brightness")
                Slider(value: $ledServer.brightness, in: 0.0...1.0, step:0.05)
            }
            .padding()
            List(Effect.allEffects(), id: \.self) { effect in 
                HStack {
                    Text(effect)
                    Spacer()
                    Image(systemName:ledServer.currentEffect.rawValue == effect ? "checkmark.circle.fill" : "circle")
                }
                .background(Color.white) // Cannot tap the entire cell without this
                .onTapGesture {
                    ledServer.currentEffect = Effect(rawValue: effect)!
                }
            }
            Spacer()
        }
        .onAppear {
            NSLog("onAppear")
            conn?.startConnection()
        }
        .onDisappear {
            conn?.stop()
        }
    }

    init(client: BonjourClient) {
        self.ledServer = client.server
        self.conn = NetworkDoodad(client: client)
    }
}

//struct LEDServerDetail_Previews: PreviewProvider {
//    static var previews: some View {
//        let s = LEDServer()
//        LEDServerDetail(client: s)
//    }
//}
