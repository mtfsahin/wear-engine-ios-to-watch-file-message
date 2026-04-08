//
//  HiWear.h
//  提供用于Wear Engine功能的入口类封装。
//  主要用于获取使用WearEngine相关功能的实例对象，例如：WESDeviceClient、WESP2PClient等。
//
//  Created by chendian on 2024/3/26.
//  Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "WESDeviceClient.h"
#import "WESP2PClient.h"
#import "WESAuthClient.h"
#import "WESMonitorClient.h"

@interface HiWear : NSObject

// SDK版本，内部使用
extern NSString *_Nonnull const SDK_VERSION_NUM;
// SDK版本号，外部使用
extern NSString *_Nonnull const SDK_VERSION_NANE;

/// 获取WESDeviceClient实例
+ (WESDeviceClient *)getDeviceClient;

/// 获取WESP2PClient实例
+ (WESP2PClient *)getP2PClient;

/// 获取WESMonitorClient实例
+ (WESMonitorClient *)getMonitorClient;

/// 获取WESAuthClient实例
+ (WESAuthClient *)getAuthClient;

/// 接收WearEngineSDK的log信息
/// - Parameter logBlock: 打印log的回调block
+ (void)startLogWithBlock:(void(^)(NSString *log))logBlock;

/// 停止输出log,会释放logBlock
+ (void)stopLog;

@end
