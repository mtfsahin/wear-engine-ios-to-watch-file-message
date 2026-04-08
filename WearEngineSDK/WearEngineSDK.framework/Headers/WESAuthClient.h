//
//  WESAuthClient.h
//  主要用于拉起APP，授权页等
//
//  Created by chendian on 2024/4/17.
//  Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
//
#import <Foundation/Foundation.h>

typedef void (^ WESAuthCallback)(BOOL isAuthSuccess, NSInteger errorCode);

@interface WESAuthClient : NSObject

+ (WESAuthClient *)sharedInstance;

/// 拉起运动健康app进行授权认证
/// - Parameters:
///   - clientId: 华为开发者联盟应用id
///   - scheme: 三方app scheme
///   - schemeSecret: 华为开发者联盟应用schemeSecret
///   - clientSecret: 华为开发者联盟应用clientSecret
///   - srcPkgName: 包名
///   - destPkgName: 应用包名
///   - callback: 回调
- (void)authWithClientId:(NSString *)clientId
                  scheme:(NSString *)scheme
            schemeSecret:(NSString *)schemeSecret
            clientSecret:(NSString *)clientSecret
              srcPkgName:(NSString *)srcPkgName
             destPkgName:(NSString *)destPkgName
                callback:(WESAuthCallback)callback;

/// 解析运动健康APP返回信息
- (void)processAuthResultWith:(NSURL *)url;

@end

