//
//  NetworkDoodad.swift
//  LEDicious
//
//  Created by Jim Turner on 8/22/20.
//

import Foundation
import Combine
import Network
//import SwiftUI

class NetworkDoodad {
    private var client:BonjourClient
    private var queue = DispatchQueue(label: "NetworkDoodadQueue")
    private var subscriber: AnyCancellable? = nil
//    private var serviceEndpoint = NWEndpoint.hostPort(host: .ipv4(IPv4Address.init("192.168.174.199")!), port: NWEndpoint.Port.init(rawValue: 11234)!)
    private var conn: NWConnection? = nil
    
    init(client: BonjourClient) {
        self.client = client

//        subscriber = self.client.server.$hexValue.receive(on: queue).sink(receiveValue: { (hexString:String) in
//            print("NetworkDoodad received hex: \(hexString)")
//            self.sendString(hexString)
//        })
        
//        conn = NWConnection(to: self.client.endpoint, using: .tcp)
//        conn?.stateUpdateHandler = stateDidChange(to:)
//        self.setupReceive()
//        conn?.start(queue: queue)
    }
        
    func startConnection() {
        subscriber = client.server.$hexValue.receive(on: queue).sink(receiveValue: { (hexString:String) in
            print("NetworkDoodad received hex: \(hexString)")
            self.sendString(hexString)
        })
//        conn = NWConnection(to: self.client.endpoint, using: .tcp)
//        conn?.stateUpdateHandler = stateDidChange(to:)
//        self.setupReceive()
//        conn?.start(queue: queue)
        
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
