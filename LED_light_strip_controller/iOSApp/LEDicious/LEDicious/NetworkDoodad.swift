//
//  NetworkDoodad.swift
//  LEDicious
//
//  Created by Jim Turner on 8/22/20.
//

import Foundation
import Combine
import Network
import SwiftUI

class NetworkDoodad {
    private var client:BonjourClient
    private var queue = DispatchQueue(label: "NetworkDoodadQueue")
    private var colorSubscriber: AnyCancellable? = nil
    private var effectSubscriber: AnyCancellable? = nil
    private var brightnessSubscriber: AnyCancellable? = nil
    private var conn: NWConnection? = nil
    private var currentConfigReceived = false // Prevent the initial load from the server from round-tripping back to the server as an update
    
    init(client: BonjourClient) {
        self.client = client
    }
        
    func startConnection() {
        currentConfigReceived = false
        colorSubscriber = client.server.$hexValue.receive(on: queue).removeDuplicates().sink(receiveValue: { (hexString:String) in
            print("NetworkDoodad received hex: \(hexString)")
            if self.currentConfigReceived {
                self.sendString("setColor:\(hexString)\r")
            }
        })

        effectSubscriber = client.server.$currentEffect.receive(on: queue).sink(receiveValue: { (effect:Effect) in
            print("NetworkDoodad received effect: \(effect.rawValue)")
            if self.currentConfigReceived {
                self.sendString("effect:\(effect.rawValue)\r")
            }
        })
        
        brightnessSubscriber = client.server.$brightness.receive(on: queue).removeDuplicates().sink(receiveValue: { (brightness:Double) in

            print("NetworkDoodad received brightness: \(brightness)")
            if self.currentConfigReceived {
                let uNormValue = Int(brightness * 255)
                self.sendString("brightness:\(uNormValue)\r")
            }
        })
        

        conn = NWConnection(to: client.endpoint, using: .tcp)
        conn?.stateUpdateHandler = stateDidChange(to:)
        self.setupReceive()
        conn?.start(queue: queue)
        
        queue.async {
            NSLog("asking for current config")
            self.sendString("currentConfig")
        }
    }
    
    private func setupReceive() {
        conn?.receive(minimumIncompleteLength: 1, maximumLength: 65565, completion: { (data, context, isComplete, error) in
            if let data = data, !data.isEmpty {
                if let _msg = String(data: data, encoding: .utf8) {
                    var msg = _msg.trimmingCharacters(in: .newlines)
                    NSLog("Received message: \(msg)")
                    if msg.hasPrefix(">currentConfig:") {
                        msg.removeSubrange(msg.startIndex..<msg.index(msg.startIndex, offsetBy: 15))
                        let parts = msg.split(separator: "|")
                        
                        DispatchQueue.main.async {
                            if let color = Color(from: String(parts[0])), self.client.server.color != color {
                                self.client.server.color = color
                            }
                            
                            if let effect = Effect(rawValue: String(parts[1])), self.client.server.currentEffect != effect {
                                self.client.server.currentEffect = effect
                            }
                            
                            if let brightness = Double(parts[2]) {
                                let normValue = brightness / 255.0
                                if self.client.server.brightness != normValue {
                                    self.client.server.brightness = normValue
                                }
                            }

                            self.currentConfigReceived = true
                        }
                    }
                }
                else {
                    print("Unable to create string from data: \(data)")
                }
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
        conn?.send(content: data, contentContext: .defaultMessage, isComplete: true, completion: .contentProcessed({ (error) in
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
            NSLog("Client connection ready")
        case .failed(let error):
            connDidFail(error: error)
        default:
//            print("Did not handle: \(state)")
            break
        }
    }
    
    private func connDidFail(error: Error) {
        print("Failed with errror: \(error)")
        stop()
    }
    
    func stop() {
        colorSubscriber?.cancel()
        effectSubscriber?.cancel()
        brightnessSubscriber?.cancel()
        
        currentConfigReceived = false
        conn?.stateUpdateHandler = nil
        conn?.cancel()
    }
}
