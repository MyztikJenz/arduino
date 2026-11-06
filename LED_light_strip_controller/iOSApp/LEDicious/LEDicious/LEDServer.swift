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
    private var effects = ["One", "Amazing", "Another one", "wild", "Boring"]
    
    var body: some View {
        VStack {
            HStack {
                Spacer()
                ColorPicker(selection: $ledServer.color, supportsOpacity: false) {
                }
                .scaleEffect(4.0)
                .frame(width: 1, height: 128) // without a frame, it won't render right. I have no idea why. Height needs to be close to accurate or layout will overlap the picker well.
                Spacer()
            }
            List(effects, id: \.self) { effect in 
                Text(effect)
            }
            Spacer()
        }
        .onAppear {
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
