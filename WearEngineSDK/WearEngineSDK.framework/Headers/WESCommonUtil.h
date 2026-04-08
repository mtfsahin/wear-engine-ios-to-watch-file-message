//
//  WESCommonUtil.h
//  WearEngine通用工具类
//
//  Created by chendian on 2024/3/26.
//  Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
//

#ifndef WESCommonUtil_h
#define WESCommonUtil_h

#import <Foundation/Foundation.h>
#import "WESP2PMessage.h"
@class WESDevice;
NS_ASSUME_NONNULL_BEGIN

@interface WESCommonUtil : NSObject

/// 8位校验
/// @param val 8进制数据
+ (NSData *)WESdataFromUInt8:(UInt8)val;

/// 16位校验
/// @param val 16进制数据
+ (NSData *)WESdataFromUInt16:(UInt16)val;

/// 32位校验
/// @param val 32进制数据
+ (NSData *)WESdataFromUInt32:(UInt32)val;

/// 64位校验
/// @param val 64进制数据
+ (NSData *)WESdataFromUInt64:(UInt64)val;

/// 生成Sha256加密字符串
/// @param inputStr 需要进行Sha256加密的字符串
+ (NSString *)generatesSha256String:(NSString *)inputStr;

/// 解析Byte数据
/// @param bytes Byte数据
/// @param length 要解析的数据长度
int parseByteDataAndLength(Byte *bytes, int *length);

/// 字节到int转换
int parseBytesToInt(Byte *bytes, int iLength);

/// 字节到int64转换
UInt64 parseBytesToUInt64(Byte *bytes, int iLength);

/// 大小端转换
int exchangeBytesToInt(Byte *bytes, int iLength);
/// type convert method,not involved，reserve the type
int  valueToPackageSize(NSUInteger value);

/// 通用字符串转字典
+ (NSDictionary *)dictionaryWithJsonStringWearEngine:(NSString *)jsonString;

/// 数组转JSON字符串
+ (NSString *)objArrayToJSON:(NSMutableArray *)array;

/// 获取当前实际 格式 "yyyy-MM-dd HH:mm:ss.SSS"
+ (NSString *)getNowTime;

/// 获取当前时间戳
+ (NSNumber *)formatTimeStrToTimestamp:(NSString *)formatTime;

/// 获取设备实时的连接状态
+ (WearEngineServerErrorCode)checkDeviceConnectState:(WESDevice *)device;

@end

NS_ASSUME_NONNULL_END

#endif
