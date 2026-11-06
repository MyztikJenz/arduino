//
//  ContentView.swift
//  test
//
//  Created by Jim Turner on 8/14/20.
//  Copyright © 2020 Apple. All rights reserved.
//

import SwiftUI
import Combine
import Network

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

class Model : ObservableObject {
    static let sharedModel: Model = {
        return Model()
    }()
    @Published var color: Color = Color(.sRGB, red: 1.0, green: 0.0, blue: 0.0, opacity: 1.0) {
        didSet {
            self.hexValue = self.color.hexValue
        }
    }
    @Published var hexValue: String = "0xFF0000"
    
}

class NetworkDoodad {
    @ObservedObject var model = Model.sharedModel
    private var queue = DispatchQueue(label: "NetworkDoodadQueue")
    private var subscriber: AnyCancellable? = nil
    private var serviceEndpoint = NWEndpoint.hostPort(host: .ipv4(IPv4Address.init("192.168.174.199")!), port: NWEndpoint.Port.init(rawValue: 11234)!)
    private var conn: NWConnection? = nil
    
    init() {
        subscriber = model.$hexValue.receive(on: queue).sink(receiveValue: { (hexString:String) in
            print("NetworkDoodad received hex: \(hexString)")
            self.sendString(hexString)
        })
        
        conn = NWConnection(to: serviceEndpoint, using: .tcp)
        conn?.stateUpdateHandler = stateDidChange(to:)
        self.setupReceive()
        conn?.start(queue: queue)
    }
    
    private func setupReceive() {
        conn?.receive(minimumIncompleteLength: 1, maximumLength: 65565, completion: { (data, context, isComplete, error) in
            if let data = data, !data.isEmpty {
                let msg = String(data: data, encoding: .utf8)
                print("Received message: \(msg!)")
            }
            if isComplete {
                self.stop()
            }
            else if let error = error {
                self.connDidFail(error: error)
            }
            else {
                self.setupReceive()
            }
        })
    }

    func sendData(_ data: Data) {
        conn?.send(content: data, completion: .contentProcessed({ (error) in
            if let error = error {
                self.connDidFail(error: error)
            }
            else {
                print("sent data \(data)")
            }
        }))
    }
    
    func sendString(_ str: String) {
        if let d = str.data(using: .utf8) {
            sendData(d)
        }
        else {
            print("Drud, can't create date from string: \(str)")
        }
    }
    
    private func stateDidChange(to state: NWConnection.State) {
        switch state {
        case .waiting(let error):
            connDidFail(error: error)
        case .ready:
            print("Client connection ready")
        case .failed(let error):
            connDidFail(error: error)
        default:
            print("Did not handle: \(state)")
            break
        }
    }
    
    private func connDidFail(error: Error) {
        print("Failed with errror: \(error)")
        stop()
    }
    
    func stop() {
        conn?.stateUpdateHandler = nil
        conn?.cancel()
    }
}

struct ContentView: View {
    @ObservedObject var model = Model.sharedModel
    @ObservedObject var browser = BonjourManager.sharedManager

    var body: some View {
        var test = model.$color
        VStack {
//            Text(model.hexValue)
//            ColorPicker(selection: $model.color, supportsOpacity: false) {
//                Label("Another doodadxx", image: "")
//                    .multilineTextAlignment(.trailing)
//            }
//            Spacer()
            List(browser.clients, id: \.id) { client in
                HStack {
                    Text(client.name)
                    Spacer()
                    ColorPicker(selection: $model.color, supportsOpacity: false) {
                    }
                    .frame(width: 35, height: 30)
                }
            }
        }
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
