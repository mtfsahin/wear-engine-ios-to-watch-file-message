//
//  WESMonitorClient.h
//  提供用于穿戴设备事件上报和状态查询相关接口的封装类，例如：设备连接状态。
//
//  Created by chendian on 2024/3/29.
//  Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "WESP2PMessage.h"
@class WESDevice;

/// WESMonitorClient of WearEngine，用于提供查询、订阅及解订阅相关能力。
@interface WESMonitorClient : NSObject

/// WESMonitorClient单例
+ (WESMonitorClient *)sharedInstance;

/// 订阅
/// @param device  设备
/// @param callback  回调
- (void)subscribeStatusOfDevice:(WESDevice *)device callback:(WESConnectCallback)callback;

/// 解订阅
/// @param device 设备
/// @param callback 回调
- (void)unsubscribeStatusOfDevice:(WESDevice *)device callback:(WESConnectCallback)callback;

@end
