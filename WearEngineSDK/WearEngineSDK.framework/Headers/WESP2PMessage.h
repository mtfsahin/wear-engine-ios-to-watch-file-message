//
//  WESP2PMessage.h
//  公共头文件
//
//  Created by chendian on 2024/3/27.
//  Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
//

#import <Foundation/Foundation.h>
@class WESP2PData;
@class WESP2PMessage;
@class WESDevice;
/// P2P消息类型
typedef NS_ENUM(NSUInteger, WESP2PMessageType) {
    /// 默认类型
    WESP2PMessageTypeDefault = 0,
    /// 数据类型
    WESP2PMessageTypeData,
    /// 文件类型（暂不支持）
    WESP2PMessageTypeFile
};

/// 设备状态枚举
typedef NS_ENUM(NSInteger, WESDeviceConnectState) {
    /// 默认状态
    WESDeviceStateUnknown         = 0,
    /// 连接中
    WESDeviceStateConnecting      = 1,
    /// 已连接
    WESDeviceStateConnected       = 2,
    /// 连接失败-检测设备状态是否开机是否在手机附近后，仍失败请前往运动健康重新连接
    WESDeviceStateDisconnected    = 3,
    /// 连接失败-需要去运动健康重新连接场景
    WESDeviceStateConnectFailed   = 4,
};

/// 设备能力集状态枚举
typedef NS_ENUM(NSInteger, WESCapabilityStatus) {
    WESCapabilityStatusSupport       = 0,
    WESCapabilityStatusUnsupported   = 1,
    WESCapabilityStatusUnknown       = 2,
};

/// WearEngine错误码枚举
typedef NS_ENUM(NSInteger, WearEngineServerErrorCode) {
    /// 成功
    WESErrorCodeSuccess                          = 0,
    /// 通用错误，当前未具体定义
    WESErrorCodeGeneric                          = 1,
    /// 运动健康未登录
    WESErrorCodeAPPLogout                        = 3,
    /// 运动健康无绑定设备（没有任何设备绑定中）
    WESErrorCodeNoBoundDevices                   = 4,
    /// 非法参数错误
    WESErrorCodeInvalidArgument                  = 5,
    /// 用户未同意运动健康用户协议、隐私声明
    WESErrorCodeUserUnauthorized                 = 7,
    /// 未通过Scope权限校验错误码
    WESErrorCodeScopeUnauthorized                = 8,
    /// 未通过WearEngine用户授权设备管理权限
    WESErrorCodeUserUnauthorizedWearEngine       = 9,
    /// 非法文件
    WESErrorCodeInvalidFile                      = 10,
    /// Server端过多的Receiver来接收穿戴设备端的消息错误码
    WESErrorCodeOvermuchReceiver                 = 11,
    /// 内部错误
    WESErrorCodeInternalError                    = 12,
    /// 穿戴设备版本不支持
    WESErrorCodeDeviceVersionLow                 = 13,
    /// 运动健康版本不支持
    WESErrorCodeAPPVersionLow                    = 14,
    /// 设备未连接
    WESErrorCodeDeviceDisconnected               = 16,
    /// 由于用户重新绑定设备，导致的UUID失效
    WESErrorCodeUUIDInvalid                      = 17,
    /// 接口不支持调用
    WESErrorCodeAPINotSupport                    = 18,
    /// 系统蓝牙关闭
    WESErrorCodeSystemBluetoothSwitchOff         = 21,
    /// 系统蓝牙未授权
    WESErrorCodeSystemBluetoothUnauthorized      = 22,
    /// 手机ping手表，手表应用未安装错误码
    WESErrorCodeP2PWatchAPPNotExit               = 200,
    /// 手机ping手表，手表应用已安装未启动错误码
    WESErrorCodeP2PWatchAPPNotRunning            = 201,
    /// 手机ping手表，手表应用已安装已启动提示码
    WESErrorCodeP2PWatchAPPRunning               = 202,
    /// P2P通信，其他错误码
    WESErrorCodeP2POtherError                    = 203,
    /// 手表ping手机，手机应用未安装错误码
    WESErrorCodeP2PPhoneAPPNotExit               = 204,
    /// 手表ping手机，手机应用已安装提示码
    WESErrorCodeP2PPhoneAPPExit                  = 205,
    /// 业务通信失败错误码
    WESErrorCodeCommFail                         = 206,
    /// 业务通信成功码
    WESErrorCodeCommSuccess                      = 207,
    /// 业务离线消息通信成功码
    WESErrorCodeOfflineMsgSuccess                = 209,
    /// 授权码获取成功
    WESErrorCodeAuthCodeSucc                     = 210,
    /// 请求授权码失败，设备未连接
    WESErrorCodeAuthCodeNoDevice                 = 211,
    /// 请求授权码失败，运动健康app未安装
    WESErrorCodeAuthCodeAppNotInstall            = 212,
    /// 请求授权码失败，URL解析失败
    WESErrorCodeAuthCodeUrlInvalid               = 213,
    /// 请求授权码失败，三方app合法性MCP校验失败
    WESErrorCodeAuthCodeMcpError                 = 214,
    /// 请求授权码失败，未登录运动健康app
    WESErrorCodeAuthCodeAppLogout                = 215,
    /// 请求授权码失败，三方app为非法签名
    WESErrorCodeAuthCodeSignInvalid              = 216,
    /// 请求授权码失败，合法权限列表为空
    WESErrorCodeAuthCodePermissionEmpty          = 217,
    /// 请求授权码失败，网络请求失败
    WESErrorCodeAuthCodeNetworkError             = 218,
    /// 取消授权码
    WESErrorCodeAuthCodeDeauthorize              = 219,
    /// 已授权，拉起运动健康APP获取设备信息成功
    WESErrorCodeAuthCodeAppShouldLaunch          = 220,
    /// 拉起运动健康APP授权成功，未获取设备信息
    WESErrorCodeAuthCodeNoDevicesInfo            = 221,
    /// 设备未佩戴
    WESErrorCodeWatchWearOff                     = 300,
    /// 设备导联脱落错误码 导联脱落就是手指没有放在上面
    WESErrorCodeSensorWatchLeadOff               = 301,
    /// 设备侧主动关闭传感器
    WESErrorCodeWatchUserStop                    = 302,
    /// 传感器已被占用
    WESErrorCodeSensorBusy                       = 303,
    /// 设备不支持当前传感器
    WESErrorCodeSensorNotSupport                 = 304,
};

#pragma mark - 回调Blocks定义

/// 用于关闭Sensor过程中接收回调。
typedef void(^WESStopSensorCallback)(WearEngineServerErrorCode status);
/// 通过String扩展，后续支撑JSON。
typedef void(^WESDataCallback)(WearEngineServerErrorCode status, NSString *data);

/// 获取ping的数据回调
typedef void(^WESPingCallback)(WearEngineServerErrorCode status);

/// 发送数据回调
typedef void(^WESSendCallback)(WearEngineServerErrorCode status, int sendProgress);

/// 取消发送文件结果回调
typedef void(^WESCancelFileTransferCallback)(int status);

/// 通信回调接口
typedef void(^WESReceiverCallback)(WearEngineServerErrorCode status, WESP2PMessage *message);

/// 获取已连接设备回调接口
typedef void(^WESGetDeviceCallback)(WearEngineServerErrorCode status, NSArray<WESDevice *>* deviceArr);

/// 连接回调接口
typedef void(^WESConnectCallback)(WearEngineServerErrorCode status, WESDeviceConnectState connectStatus);

#pragma mark P2P消息对象
@interface WESP2PMessage : NSObject

@property (nonatomic, assign) WESP2PMessageType type;
@property (nonatomic, copy) NSString *fileName;
@property (nonatomic, copy) NSString *filePath;
@property (nonatomic, strong) NSData *data;
@property (nonatomic, assign) BOOL enableEncrypt;
/// P2P模型
@property (nonatomic, strong) WESP2PData *p2pData;

/// 获取Message类型：0表示默认类型，1表示数据类型，2表示文件类型
- (int)getType;

/// 返回是否加密传输   Yes 加密传输 No 不加密传输
- (BOOL)isEnableEncrypt;

/// 组装message对象
/// - Parameter payloadString: 要发送的数据
+ (instancetype)initWithPayload:(NSString *)payloadString;

@end

#pragma mark 设备对象
@interface WESDevice : NSObject

/// 设备名称
@property (nonatomic, copy) NSString *deviceName;

/// 设备标识，即：uuid
@property (nonatomic, copy) NSString *uuid;

/// 设备产品编号
@property (nonatomic, assign) NSInteger productType;
/// 设备产品类型（0: 运动表）
@property (nonatomic, assign) NSInteger deviceType;
/// 设备版本
@property (nonatomic, copy) NSString *softwareVersion;
/// 设备型号
@property (nonatomic, copy) NSString *deviceModel;

/// 设备电量
@property (nonatomic, assign) NSInteger batteryLevel;

/// 连接状态
@property (nonatomic, assign) WESDeviceConnectState connectState;

/// 保留字段
@property (nonatomic, copy) NSString *reversedness;

/// 设备是否连接中
- (BOOL)isConnected;

@end

#pragma mark P2P数据
@interface WESP2PData : NSObject
/// 子命令
@property (nonatomic, assign) int subCmd;
/// 消息序号
@property (nonatomic, assign) NSInteger subSeqId;
/// 包名
@property (nonatomic, copy) NSString *srcPkgName;
/// 指纹信息
@property (nonatomic, copy) NSString *srcFingerPrint;
/// 包名
@property (nonatomic, copy) NSString *destPkgName;
/// 指纹信息
@property (nonatomic, copy) NSString *destFingerPrint;
/// 初始化不需赋值，由设备上报后返回
@property (nonatomic, strong) NSData *response;
/// 状态码
@property (nonatomic, assign) NSInteger responseErrorCode;
/// 发送数据异常时，返回的错误码  例如：参数错误100007
@property (nonatomic, assign) NSInteger exceptionErrorCode;
/// 是否为TLV格式。默认YES
@property (nonatomic, assign) BOOL isTLVFormat;
/// payload和payloadData只需赋值一个即可
@property (nonatomic, strong) NSMutableArray *payload;
/// 可以是json 格式，和TLV格式
@property (nonatomic, strong) NSData *payloadData;

@end

#pragma mark  TLVData 组装TLV
@interface TLVDataCmm : NSObject
/// 类型
@property  (atomic, assign) Byte type;
/// 长度//limited return ,reserve the type
@property  (atomic, assign) NSUInteger length;
/// 取值，对于含有子结点的对象，value为nil
@property  (atomic, retain) NSMutableData *value;

/// 对于叶子级的TLV的初始化接口
- (id)init:(int)type  value:(NSData *)data;

/// 对于嵌套关系的TLV的父级TL初始化专用接口,组包时默认value为nil；
- (id)initWithType:(int)type  withLen:(int)length;

/// 组包单个TLV对象
- (NSData*)TLVToData;

- (int)getTotalSize;//limited return ,reserve the type

@end

#pragma mark 身份识别对象
@interface WESIdentityInfo : NSObject
/// 包名
@property (nonatomic, copy) NSString *mPackageName;
/// 签名信息
@property (nonatomic, copy) NSString *mFingerPrint;

/// 检查是否为空
+ (BOOL)checkNullOrEmpty:(WESIdentityInfo *)info;
/// 检查包名长度是否超过255
+ (BOOL)checkOverLong:(WESIdentityInfo *)info;

@end
