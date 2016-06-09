//========================================================= 
/**@file IOS_H002Driver.h
 * @brief IOSÏÂH002ÃüÁîÇý¶¯
 * 
 * @date 2016-06-09   11:27:18
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_IOS_H002DRIVER_H_
#define _LIBZHOUYB_IOS_H002DRIVER_H_
//--------------------------------------------------------- 
#import "BluetoothDevice.h"
//--------------------------------------------------------- 
@interface H002Driver : NSObject
/// À¶ÑÀ²Ù×÷
@property(nonatomic, retain) id<BluetoothDeviceDelegate> delegate;
-(BOOL)TransmitCommand:(NSString*)sCmd, (NSString*)sArg, (NSMutableString*)sRecv;
@end
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_IOS_H002DRIVER_H_
//========================================================= 