import Foundation
import Combine
import WearEngineSDK

protocol WearEngineManagerDelegate: AnyObject {
    func didLog(_ text: String)
    func didUpdateDeviceList(_ devices: [WearEngineManager.WearableDeviceInfo])
    func didReceiveMessage(_ text: String)
}

final class WearEngineManager: ObservableObject {
    private let authClient = WESAuthClient.sharedInstance()
    private let deviceClient = WESDeviceClient.sharedInstance()
    private let p2pClient = WESP2PClient.sharedInstance()
    private let monitorClient = WESMonitorClient.sharedInstance()
    
    private var receiverCallback: ((WearEngineServerErrorCode, WESP2PMessage?) -> Void)?

    @Published private(set) var isAuthorized = false
    @Published private(set) var devices: [WearableDeviceInfo] = []
    @Published private(set) var connectedDevices: [WearableDeviceInfo] = []
    @Published private(set) var currentDeviceCached: WearableDeviceInfo?

    weak var delegate: WearEngineManagerDelegate?


    private let clientId = "YOUR_CLIENT_ID"
    private let clientSecret = "YOUR_SECRET_ID"
    private let schemeSecret = "YOUR_SECRET_SCHEME"
    private let authCallbackScheme = "YOUR_AUTH_CALLBACK_SCHEME"
    private let srcPkgName = "YOUR_SOURCE_PACKAGE_NAME"
    private let srcFingerprint = "YOUR_SOURCE_FINGERPRINT_NAME"
    

    private let destPkgName = "YOUR_DESTINATION_PACKAGE_NAME"
    private let destFingerprint = "YOUR_DESTINATION_FINGERPRINT"

    private let autoConnectIfSingleDevice = true
    private var subscribedDeviceUUIDs = Set<String>()

    struct WearableDeviceInfo: Identifiable, Hashable {
        let id: String
        let name: String
        let isConnected: Bool
        let originalDevice: WESDevice?
    }

    init() { }

    func requestAuthorization(completion: @escaping (Bool, String) -> Void) {
        delegate?.didLog("Auth: start")

        guard !clientId.isEmpty, !clientSecret.isEmpty, !schemeSecret.isEmpty, !authCallbackScheme.isEmpty else {
            completion(false, "Missing config (clientId/clientSecret/schemeSecret/callbackScheme)")
            return
        }
        guard let authClient else {
            completion(false, "Auth client unavailable")
            return
        }

        authClient.auth(
            withClientId: clientId,
            scheme: authCallbackScheme,
            schemeSecret: schemeSecret,
            clientSecret: clientSecret,
            srcPkgName: srcPkgName,
            destPkgName: destPkgName
        ) { [weak self] isSuccess, errorCode in
            guard let self = self else { return }
            Task { @MainActor in
                self.isAuthorized = isSuccess
                self.delegate?.didLog("Auth callback: success=\(isSuccess) code=\(errorCode)")
                if isSuccess {
                    self.refreshDevicesAndResubscribe()
                    completion(true, "Authorization successful")
                } else {
                    completion(false, "Authorization failed: \(errorCode)")
                }
            }
        }
    }

    func handleOpenURL(_ url: URL) {
        print("url", url)
            authClient?.processAuthResult(with: url)
    }

    func refreshDevicesAndResubscribe() {
        getAvailableDevicesWithRetry(retries: 2, delay: 0.6) { [weak self] _, _ in
            guard let self = self else { return }
            self.verifyCurrentDevice()
            self.resubscribeToDevices()
        }
    }

    func resubscribeToDevices() {
        let currentUUIDs = Set(devices.map { $0.id })
        let toUnsub = subscribedDeviceUUIDs.subtracting(currentUUIDs)
        let toSub = currentUUIDs.subtracting(subscribedDeviceUUIDs)

        for uuid in toUnsub {
            if let dev = devices.first(where: { $0.id == uuid })?.originalDevice {
                unsubscribeStatus(of: dev)
            } else {
                subscribedDeviceUUIDs.remove(uuid)
            }
        }
        for uuid in toSub {
            if let dev = devices.first(where: { $0.id == uuid })?.originalDevice {
                subscribeStatus(of: dev)
            }
        }
    }

    private func subscribeStatus(of device: WESDevice) {
        guard let monitorClient else { return }
        let name = device.deviceName ?? "-"
        let uuid = device.uuid ?? "-"
        delegate?.didLog("Monitor: subscribe -> \(name) (\(uuid))")

        monitorClient.subscribeStatus(of: device) { [weak self] (status: WearEngineServerErrorCode, state: WESDeviceConnectState) in
            guard let self = self else { return }
            Task { @MainActor in
                self.delegate?.didLog("Monitor cb: status=\(status.rawValue) device=\(name) state=\(state.rawValue)")
                self.getAvailableDevicesWithRetry(retries: 1, delay: 0.4) { _, _ in
                    self.verifyCurrentDevice()
                }
            }
        }
        subscribedDeviceUUIDs.insert(uuid)
    }

    private func unsubscribeStatus(of device: WESDevice) {
        guard let monitorClient else { return }
        let name = device.deviceName ?? "-"
        let uuid = device.uuid ?? "-"
        delegate?.didLog("Monitor: unsubscribe -> \(name) (\(uuid))")

        monitorClient.unsubscribeStatus(of: device) { [weak self] (status: WearEngineServerErrorCode, state: WESDeviceConnectState) in
            guard let self = self else { return }
            Task { @MainActor in
                self.delegate?.didLog("Monitor unsubs: status=\(status.rawValue) device=\(name) state=\(state.rawValue)")
                self.subscribedDeviceUUIDs.remove(uuid)
            }
        }
    }

    func preloadConnectedIfAny(completion: @escaping (WearableDeviceInfo?) -> Void) {
        getAvailableDevicesWithRetry(retries: 2, delay: 0.6) { [weak self] list, _ in
            guard let self = self else { completion(nil); return }

            if let d = self.getCurrentDeviceFromSDK(),
               d.isConnected() || (d.connectState.rawValue == 2) {
                let info = WearableDeviceInfo(
                    id: d.uuid ?? "current",
                    name: d.deviceName ?? "Unknown",
                    isConnected: true,
                    originalDevice: d
                )
                self.currentDeviceCached = info
                completion(info)
                return
            }

            if let already = list.first(where: { $0.isConnected }) {
                self.currentDeviceCached = already
                completion(already)
                return
            }

            if self.autoConnectIfSingleDevice, list.count == 1, let only = list.first {
                self.currentDeviceCached = only
                completion(only)
                return
            }

            self.currentDeviceCached = nil
            completion(nil)
        }
    }

    func getAvailableDevicesWithRetry(
        retries: Int = 2,
        delay: TimeInterval = 0.6,
        completion: @escaping ([WearableDeviceInfo], String?) -> Void
    ) {
        getAvailableDevices { [weak self] list, err in
            guard let self = self else { return }
            if !list.isEmpty || retries <= 0 {
                completion(list, err)
                return
            }
            self.delegate?.didLog("Devices empty, retrying in \(delay)s (left: \(retries))")
            DispatchQueue.main.asyncAfter(deadline: .now() + delay) {
                self.getAvailableDevicesWithRetry(
                    retries: retries - 1,
                    delay: delay,
                    completion: completion
                )
            }
        }
    }

    func getAvailableDevices(completion: @escaping ([WearableDeviceInfo], String?) -> Void) {
        delegate?.didLog("Devices: getCommonDevices")
        guard let deviceClient else {
            completion([], "Device client unavailable")
            return
        }

        deviceClient.getCommonDevices { [weak self] status, deviceArr in
            guard let self = self else { return }
            Task { @MainActor in
                guard status == .WESErrorCodeSuccess else {
                    self.delegate?.didLog("getCommonDevices failed: \(status.rawValue)")
                    completion([], "Device query failed: \(status.rawValue)")
                    return
                }
                guard let arr = deviceArr, !arr.isEmpty else {
                    self.delegate?.didLog("Device list empty")
                    self.devices = []
                    self.connectedDevices = []
                    self.delegate?.didUpdateDeviceList([])
                    completion([], nil)
                    return
                }

                let list: [WearableDeviceInfo] = arr.compactMap { dev in
                    guard let uuid = dev.uuid, !uuid.isEmpty else { return nil }
                    let isConn = dev.isConnected() || (dev.connectState.rawValue == 2)
                    return WearableDeviceInfo(
                        id: uuid,
                        name: dev.deviceName ?? "Unknown",
                        isConnected: isConn,
                        originalDevice: dev
                    )
                }

                self.devices = list
                self.connectedDevices = list.filter { $0.isConnected }
                self.delegate?.didUpdateDeviceList(list)
                self.delegate?.didLog("Devices count: \(list.count)")
                completion(list, nil)
            }
        }
    }
    func connectToDevice(_ device: WearableDeviceInfo, completion: @escaping (Bool, String) -> Void) {
        delegate?.didLog("Connect: requesting \(device.name)")
        guard let deviceClient else {
            completion(false, "Device client unavailable")
            return
        }
        guard let wesDevice = device.originalDevice else {
            completion(false, "Invalid device ref")
            return
        }

        deviceClient.connect(wesDevice) { [weak self] status, connectState in
            guard let self else { return }
            Task { @MainActor in
                self.delegate?.didLog("Connect callback: status=\(status.rawValue) state=\(connectState.rawValue)")
                guard status == .WESErrorCodeSuccess else {
                    completion(false, "Connect failed: \(status.rawValue)")
                    return
                }
                if connectState.rawValue == 2 {
                    DispatchQueue.main.asyncAfter(deadline: .now() + 1.0) { [weak self] in
                        guard let self else { return }
                        self.verifyCurrentDevice()
                    }
                    completion(true, "Connected")
                } else {
                    completion(false, "Not connected (state=\(connectState.rawValue))")
                }
            }
        }
    }


    func getCurrentDeviceFromSDK() -> WESDevice? {
        let dev = deviceClient?.getCurrentDevices()
        if let d = dev {
            delegate?.didLog("Current: \(d.deviceName ?? "Unknown") state=\(d.connectState.rawValue) isConn=\(d.isConnected())")
        } else {
            delegate?.didLog("Current: nil")
        }
        return dev
    }

    func verifyCurrentDevice() {
        if let d = getCurrentDeviceFromSDK() {
            let connected = d.isConnected() || (d.connectState.rawValue == 2)
            currentDeviceCached = WearableDeviceInfo(
                id: d.uuid ?? "current",
                name: d.deviceName ?? "Unknown",
                isConnected: connected,
                originalDevice: d
            )
        } else {
            currentDeviceCached = nil
        }
    }

    private func ensureReadyDevice() -> WESDevice? {
        guard let device = WESDeviceClient.sharedInstance()?.getCurrentDevices() else {
            delegate?.didLog("No current device from SDK")
            return nil
        }
        let connected = device.isConnected() || (device.connectState.rawValue == 2)
        guard connected else {
            delegate?.didLog("Device not connected: state=\(device.connectState.rawValue)")
            return nil
        }
        return device
    }

    private func makeSenderIdentity() -> (WESIdentityInfo, WESIdentityInfo) {
        let src = WESIdentityInfo()
        src.mPackageName = srcPkgName
        src.mFingerPrint = srcFingerprint

        let dest = WESIdentityInfo()
        dest.mPackageName = destPkgName
        dest.mFingerPrint = destFingerprint
        return (src, dest)
    }

    private func makeReceiverIdentity() -> (WESIdentityInfo, WESIdentityInfo) {
        let src = WESIdentityInfo()
        src.mPackageName = srcPkgName
        src.mFingerPrint = srcFingerprint
        let dest = WESIdentityInfo()
        dest.mPackageName = destPkgName
        dest.mFingerPrint = destFingerprint
        return (src, dest)
    }

    private func makeMessage(_ text: String) -> WESP2PMessage {
        let msg = WESP2PMessage()
        msg.type = .data

        let p2p = WESP2PData()
        p2p.isTLVFormat = false
        p2p.payloadData = text.data(using: .utf8)

        p2p.srcPkgName = srcPkgName
        p2p.srcFingerPrint = srcFingerprint
        p2p.destPkgName = destPkgName
        p2p.destFingerPrint = destFingerprint

        msg.p2pData = p2p
        return msg
    }

    func checkAppInstallation(completion: @escaping (Bool, String) -> Void) {
        guard let dev = ensureReadyDevice() else {
            completion(false, "No device connected")
            return
        }
        guard let p2pClient else {
            completion(false, "P2P client unavailable")
            return
        }
        p2pClient.isAppInstalled(dev, targetPkgName: destPkgName, srcPkgName: srcPkgName) { [weak self] status, data in
            guard let self = self else { return }
            Task { @MainActor in
                if status == .WESErrorCodeSuccess {
                    let raw = data ?? ""
                    let normalized = raw.trimmingCharacters(in: .whitespacesAndNewlines).lowercased()
                    let installed =
                        normalized == "true" ||
                        normalized == "1" ||
                        normalized == "yes" ||
                        normalized.contains("yes")
                    self.delegate?.didLog("isAppInstalled result -> status=\(status.rawValue) raw='\(raw)' normalized='\(normalized)' installed=\(installed)")
                    completion(installed, installed ? "Installed" : "Not installed")
                } else {
                    self.delegate?.didLog("isAppInstalled failed: \(status.rawValue) data=\(data ?? "nil")")
                    completion(false, "Check failed: \(status.rawValue)")
                }
            }
        }
    }

    func getAppVersion(completion: @escaping (String?, String?) -> Void) {
        guard let dev = ensureReadyDevice() else {
            completion(nil, "No device connected")
            return
        }
        guard let p2pClient else {
            completion(nil, "P2P client unavailable")
            return
        }
        p2pClient.getAppVersion(dev, targetPkgName: destPkgName, srcPkgName: srcPkgName) { [weak self] status, data in
            guard let self = self else { return }
            Task { @MainActor in
                if status == .WESErrorCodeSuccess {
                    let version = (data != "-1" && data != nil) ? data : "Not installed"
                    completion(version, nil)
                } else {
                    self.delegate?.didLog("getAppVersion failed: \(status.rawValue)")
                    completion(nil, "Version check failed: \(status.rawValue)")
                }
            }
        }
    }

    func pingDevice(completion: @escaping (Bool, String) -> Void) {
        guard let currentDevice = ensureReadyDevice() else {
            completion(false, "No device connected")
            return
        }
        guard let p2pClient else {
            completion(false, "P2P client unavailable")
            return
        }
        p2pClient.ping(currentDevice, srcPkgName: srcPkgName, destPkgName: destPkgName) { [weak self] status in
            guard self != nil else { return }
            Task { @MainActor in
                if status == .WESErrorCodeP2PWatchAPPRunning || status == .WESErrorCodeSuccess {
                    completion(true, "Ping OK")
                } else if status == .WESErrorCodeP2PWatchAPPNotExit {
                    completion(false, "Watch app not installed (200)")
                } else if status == .WESErrorCodeP2PWatchAPPNotRunning {
                    completion(false, "Watch app not running (201)")
                } else {
                    completion(false, "Ping failed: \(status.rawValue)")
                }
            }
        }
    }

    func registerMessageListener() {
        guard let currentDevice = ensureReadyDevice() else {
            delegate?.didLog("No device connected")
            return
        }
        
        let srcInfo = WESIdentityInfo()
        srcInfo.mPackageName = srcPkgName
        srcInfo.mFingerPrint = srcFingerprint
        
        let destInfo = WESIdentityInfo()
        destInfo.mPackageName = destPkgName
        destInfo.mFingerPrint = destFingerprint
        
        receiverCallback = { [weak self] (status: WearEngineServerErrorCode, message: WESP2PMessage?) in
            guard let self = self else { return }
            if status != .WESErrorCodeSuccess {
                print("registerReceiver failed: \(status.rawValue)")
                self.delegate?.didLog("registerReceiver failed: \(status.rawValue)")
                return
            }
            if status == .WESErrorCodeSuccess && message == nil {
                print("Receiver registered")
                self.delegate?.didLog("Receiver registered. pkg=\(destInfo.mPackageName)")
                return
            }
            if (message != nil) {
                let response = message?.data
                let responseString :String = String.init(data: response ?? Data() , encoding: .utf8) ?? ""
                if (responseString.count > 0) {
                    self.delegate?.didLog("P2P received responseString > 0: \(responseString )")
                    self.delegate?.didReceiveMessage(responseString)
                } else {
                    let string = message?.data.toHexString() ?? ""
                    self.delegate?.didLog("P2P received: \(string)")
                    self.delegate?.didReceiveMessage(string)
                }
            }
        }
        
        HiWear.getP2PClient().registerReceiver(
            currentDevice,
            srcInfo: srcInfo,
            destInfo: destInfo,
            receiverCallback: receiverCallback
        )
    }
    
    

    func unregisterMessageListener() {
        guard let dev = ensureReadyDevice() else { return }
        guard let p2pClient else { return }
        let (src, dest) = makeReceiverIdentity()
        p2pClient.unRegisterReceiver(dev, srcInfo: src, destInfo: dest) { [weak self] status, _ in
            guard let self = self else { return }
            Task { @MainActor in
                let ok = (status == .WESErrorCodeSuccess)
                self.delegate?.didLog(ok ? "unregisterReceiver OK" : "unregisterReceiver failed \(status.rawValue)")
            }
        }
    }

    func sendMessage(_ text: String, completion: @escaping (Bool, String) -> Void) {
        guard let deviceClient = WESDeviceClient.sharedInstance(),
              let p2pClient = WESP2PClient.sharedInstance(),
              let dev = deviceClient.getCurrentDevices(),
              dev.isConnected() || dev.connectState.rawValue == 2
        else {
            completion(false, "No device connected")
            return
        }

        let srcInfo = WESIdentityInfo()
        srcInfo.mPackageName = srcPkgName
        srcInfo.mFingerPrint = srcFingerprint

        let destInfo = WESIdentityInfo()
        destInfo.mPackageName = destPkgName
        destInfo.mFingerPrint = destFingerprint
        
        let message = WESP2PMessage();
        message.type = WESP2PMessageType.data;
     
        let wesP2PData = WESP2PData()
        wesP2PData.isTLVFormat = false
     
        let unitIsOpen = text
        let tlv = TLVDataCmm.init(0, value: unitIsOpen.data(using: .utf8))
        let tlvData = tlv?.value ?? NSMutableData()
     
        let rootCmm = TLVDataCmm(type: 0x07, withLen: Int32(tlvData.count))
        let rootCmmData = rootCmm?.tlvToData() ?? Data()
     
        let tmpPayloadData = NSMutableData.init(data: rootCmmData)
        tmpPayloadData.append(tlvData as Data)
        wesP2PData.payloadData = tmpPayloadData.copy() as? Data
        message.p2pData = wesP2PData.copy() as? WESP2PData

        p2pClient.send(dev, message: message, srcInfo: srcInfo, destInfo: destInfo) { status, progress in
            if status == .WESErrorCodeCommSuccess || status == .WESErrorCodeSuccess {
                print("[P2P] Send OK (status=\(status.rawValue), progress=\(progress))")
                completion(true, "Sent")
            } else {
                print("[P2P] Send FAIL (status=\(status.rawValue), progress=\(progress))")
                completion(false, "Send failed: \(status.rawValue)")
            }
        
        }
    }
    
    func sendLongMessage(
        size: Int = 50 * 1024,
        progress: @escaping (_ sent: Int, _ total: Int) -> Void,
        completion: @escaping (Bool, String) -> Void
    ) {
        guard ensureReadyDevice() != nil else {
            completion(false, "No device connected")
            return
        }
        let chunkSize = 900
        let total = max(1, size / chunkSize)
        var sent = 0
        var failed = 0

        func next() {
            guard sent < total else {
                completion(
                    failed == 0,
                    failed == 0 ? "Long message sent" : "Completed with failures: \(failed)"
                )
                return
            }
            let payload = String(repeating: "x", count: chunkSize)
            sendMessage("Chunk \(sent + 1)/\(total): \(payload)") { ok, _ in
                sent += 1
                if !ok { failed += 1 }
                progress(sent, total) 
                DispatchQueue.main.asyncAfter(deadline: .now() + 0.05) { next() }
            }
        }
        next()
    }
}
private extension Data {
    func toHexString() -> String {
        map { String(format: "%02x", $0) }.joined()
    }
}
