//
//  BonjourManager.swift
//  test
//
//  Created by Jim Turner on 8/17/20.
//  Copyright © 2020 Apple. All rights reserved.
//

import Network
import Combine
import Foundation.NSUUID

struct BonjourClient: Identifiable {
    let id = UUID()
    let name: String
    let endpoint: NWEndpoint
    let server: LEDServer
}

class BonjourManager: ObservableObject {
    static let sharedManager: BonjourManager = {
        return BonjourManager()
    }()

    private var browser: NWBrowser? = nil
    private var queue = DispatchQueue(label: "BonjourManager")
    @Published var clients = [BonjourClient]()

    init() {
//        let bonjourType = "_ledicious._tcp"
        let bonjourType = "_http._tcp"
        browser = NWBrowser(for: .bonjour(type: bonjourType, domain: nil), using: .tcp)
        browser?.browseResultsChangedHandler = { (results, changes) in 
            self.processChanges(changes: changes)
        }
        browser?.start(queue: queue)
    }
    
    private func processChanges(changes: Set<NWBrowser.Result.Change>) {
        for chng in changes {
            switch chng {
            case .added(let browserResult):
                print("added \(browserResult)")
                switch browserResult.endpoint {
                case .service(name: let name, type: _, domain: _, interface: _):
                    let c = BonjourClient(name: name, endpoint: browserResult.endpoint, server: LEDServer())
                    clients.append(c)
                default:
                    break
                }
                print("")
            case .removed(let browserResult):
                print("removed \(browserResult.endpoint)")
                switch browserResult.endpoint {
                case .service(name: let name, type: _, domain: _, interface: _):
                    clients.removeAll { $0.name == name }
                default:
                    break
                }
            case .changed(old: let old, new: let new, flags: let flags):
                print("change: \(old) \(new) \(flags)")
            case .identical:
                print("identical... why does this exist?")
            default:
                print("processChanges got unknown change: \(chng)")
                
            }
        }
    }
}
