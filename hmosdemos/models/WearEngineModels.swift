import Foundation

struct WearableDevice: Identifiable, Hashable {
    let id: String
    let name: String
    let isConnected: Bool
}

struct P2PMessage: Identifiable {
    let id = UUID() 
    let content: String
    let timestamp: Date
    let isOutgoing: Bool
}
