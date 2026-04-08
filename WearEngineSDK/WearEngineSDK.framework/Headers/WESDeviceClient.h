//
//  WESDeviceClient.h
//  用于操作穿戴设备相关的封装，例如：获取穿戴设备列表等。
//
//  Created by chendian on 2024/3/26.
//  Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "WESP2PMessage.h"
@class WESDevice;

@interface WESDeviceClient : NSObject

/// WESDeviceClient单例
+ (WESDeviceClient *)sharedInstance;

/// 获取当前连接的设备
- (WESDevice *)getCurrentDevices;

/// 获取设备列表
/// - Parameter callback: 设备列表结果回调
- (void)getCommonDevices:(WESGetDeviceCallback)callback;

/// 连接设备
/// - Parameters:
///   - device: 需要连接的设备对象
///   - callback: 连接状态回调
- (void)connectDevice:(WESDevice *)device connectCallback:(WESConnectCallback)callback;

@end
