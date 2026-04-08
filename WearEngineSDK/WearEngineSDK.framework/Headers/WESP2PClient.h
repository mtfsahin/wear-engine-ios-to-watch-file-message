//
//  WESP2PClient.h
//  提供用于穿戴设备发送消息通信、文件，接收消息，文件等功能的封装类.
//
//  Created by chendian on 2024/3/27.
//  Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "WESP2PMessage.h"

#pragma mark - WESP2PClient

@interface WESP2PClient : NSObject


/// WESP2PClient单例
+ (WESP2PClient *)sharedInstance;

/// 检测需要通信的穿戴设备侧应用是否在线
/// - Parameters:
///   - device: 三方应用指定的需要通信的设备对象
///   - srcPkgName: 发消息源APP包名
///   - destPkgName: 三方应用指定的需要通信的设备侧应用包名
///   - callback: ping消息的结果回调
- (void)ping:(WESDevice *)device
  srcPkgName:(NSString *)srcPkgName
 destPkgName:(NSString *)destPkgName
pingCallback:(WESPingCallback)callback;

/// 向指定的设备上的指定应用注册一个接收消息或文件的回调对象
/// @param device 三方应用指定的需要通信的设备对象
/// @param srcInfo 发消息源APP信息
/// @param destInfo 三方应用指定的需要通信的设备侧应用信息
/// @param callback 接收消息或文件的回调对象
- (void)registerReceiver:(WESDevice *)device
                 srcInfo:(WESIdentityInfo *)srcInfo
                destInfo:(WESIdentityInfo *)destInfo
        receiverCallback:(WESReceiverCallback)callback;

/// 向需要通信的穿戴设备侧应用发送通信消息
/// - Parameters:
///   - device: 三方应用指定的需要通信的设备对象
///   - message: 三方应用指定的需要发送的消息
///   - srcInfo: 发消息源APP信息
///   - destInfo: 三方应用指定的需要通信的设备侧应用信息
///   - callback: 发送消息的结果回调对象
- (void)send:(WESDevice *)device
     message:(WESP2PMessage *)message
     srcInfo:(WESIdentityInfo *)srcInfo
    destInfo:(WESIdentityInfo *)destInfo
sendCallback:(WESSendCallback)callback;

/// 注销一个已经被注册的接收消息或文件的回调对象
/// @param device 三方应用指定的需要通信的设备对象
/// @param srcInfo 发消息源APP信息
/// @param destInfo 三方应用指定的需要通信的设备侧应用信息
/// @param callback 为空时注销全部的对应设备包名的全部callback
- (void)unRegisterReceiver:(WESDevice *)device
                 srcInfo:(WESIdentityInfo *)srcInfo
                destInfo:(WESIdentityInfo *)destInfo
        receiverCallback:(WESReceiverCallback)callback;

/// 查看是否安装指定的设备应用
/// @param device 三方应用指定的需要通信的设备对象
/// @param targetPkgName targetPkgName 设备应用包名
/// @param srcPkgName 发消息源APP包名
/// @param callback 设备信息回调
- (void)isAppInstalled:(WESDevice *)device
         targetPkgName:(NSString *)targetPkgName
            srcPkgName:(NSString *)srcPkgName
              callback:(WESDataCallback)callback;

/// 查看安装的指定设备应用版本号,未安装则返回-1
/// @param device 三方应用指定的需要通信的设备对象
/// @param targetPkgName 设备应用包名
/// @param srcPkgName 发消息源APP包名
/// @param callback 设备信息回调
- (void)getAppVersion:(WESDevice *)device
        targetPkgName:(NSString *)targetPkgName
           srcPkgName:(NSString *)srcPkgName
             callback:(WESDataCallback)callback;

@end
