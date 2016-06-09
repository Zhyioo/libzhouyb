//========================================================= 
/**@file BluetoothDevice.h
 * @brief IOS下的蓝牙设备
 * 
 * @date 2016-06-09   10:22:04
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_BLUETOOTHDEVICE_H_
#define _LIBZHOUYB_BLUETOOTHDEVICE_H_
//--------------------------------------------------------- 
#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>
//--------------------------------------------------------- 
@protocol BluetoothDeviceDelegate

-(void)Connect:(NSString*)devName, (NSString*)sUUID, (NSString*)rUUID;
-(BOOL)IsConnected;
-(void)Write:(NSData*)data;
-(BOOL)IsWrited;
-(void)Read;
-(BOOL)IsReaded:(NSMutableData*)data;
-(void)Clean;
-(void)Disconnect;
-(BOOL)IsDisconnected;

@end
//--------------------------------------------------------- 
@interface BluetoothDevice : NSObject<CBCentralManagerDelegate, CBPeripheralDelegate, BluetoothDeviceDelegate>

@end
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_BLUETOOTHDEVICE_H_
//========================================================= 