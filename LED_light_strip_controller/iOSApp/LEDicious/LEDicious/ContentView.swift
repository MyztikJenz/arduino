//
//  ContentView.swift
//  LEDicious
//
//  Created by Jim Turner on 8/9/20.
//

import SwiftUI

struct ContentView: View {
    @ObservedObject var browser = BonjourManager.sharedManager

    var body: some View {
        NavigationView {
            List(browser.clients, id: \.id) { client in
                NavigationLink(destination: LEDServerDetail(client: client)) {
                    HStack {
                        Text(client.name)
                        Spacer()
                    }
                }
            }
        }
        .navigationBarTitle("LEDicious")
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
