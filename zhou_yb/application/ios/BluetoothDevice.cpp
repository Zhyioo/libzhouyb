//========================================================= 
/**@file BluetoothDevice.cpp
 * @brief IOS下蓝牙设备
 * 
 * @date 2016-06-09   10:22:27
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_BLUETOOTHDEVICE_CPP_
#define _LIBZHOUYB_BLUETOOTHDEVICE_CPP_
//--------------------------------------------------------- 
#import "BluetoothDevice.h"

@implementation BluetoothDevice{
    CBCentralManager* activeCentralManager;
    CBPeripheral* activePeripheral;
    NSString* activeName;
    NSString* sUuid;
    NSString* rUuid;
    dispatch_queue_t waitQueue;
    CBCharacteristic* sendCharacteristic;
    CBCharacteristic* recvCharacteristic;
    NSMutableData* cacheBuff;
    NSLock* cacheLock;
    BOOL sendFlag;
    BOOL recvFlag;
}

-(id)init{
    if(self = [super init]) {
        waitQueue = dispatch_queue_create("com.waitqueue", DISPATCH_QUEUE_CONCURRENT);
        cacheBuff = [[NSMutableData alloc] init];
        cacheLock = [[NSLock alloc] init];
    }
    return self;
}

-(void)Connect:(NSString*)devName, (NSString*)sUUID, (NSString*)rUUID{
    NSLog(@"Connect[%@]...", devName);

    [self Disconnect];

    activeName = devName;
    sUuid = sUUID;
    rUuid = rUUID;
    activeCentralManager = [[CBCentralManager alloc] initWithDelegate:self queue : waitQueue];
}

-(void)Clean{
    [cacheLock lock];
    [cacheBuff setLength : 0];
    [cacheLock unlock];
}

-(BOOL)IsConnected{
    return activePeripheral != nil && sendCharacteristic != nil && recvCharacteristic != nil;
}

-(void)Write:(NSData*)data{
    sendFlag = NO;
    if(sendCharacteristic != nil) {
        [activePeripheral writeValue : data forCharacteristic : sendCharacteristic type : CBCharacteristicWriteWithResponse];

        NSData* sendData = data;
        Byte* pData = (Byte*)[sendData bytes];
        NSMutableString* sendAscii = [[NSMutableString alloc] init];
        for(int i = 0;i < sendData.length; ++i) {
            [sendAscii appendFormat : @"%02X", pData[i]];
        }
        NSLog(@"%@ SEND:%@", sendCharacteristic.UUID.description, sendAscii);
    }
}

-(BOOL)IsWrited{
    return sendFlag;
}

-(void)Read{
    [cacheLock lock];
    recvFlag = NO;
    [cacheBuff setLength : 0];
    [cacheLock unlock];
}

-(BOOL)IsReaded:(NSMutableData*)data{
    // 当前有缓存数据
    [cacheLock lock];
    BOOL hasData = [cacheBuff length];
    if(hasData) {
        [data appendData : cacheBuff];
        [cacheBuff setLength : 0];
        recvFlag = NO;
        [cacheLock unlock];

        return YES;
    }
    recvFlag = NO;
    [cacheLock unlock];

    return NO;
}

-(void)Disconnect{
    [cacheLock lock];
    [cacheBuff setLength : 0];
    [cacheLock unlock];
    if(activeCentralManager == nil) {
        return;
    }

    if(activePeripheral != nil) {
        CBPeripheral* peripheral = activePeripheral;
        activePeripheral = nil;
        [activeCentralManager cancelPeripheralConnection : peripheral];
    }
    if([activeCentralManager isScanning]) {
        [activeCentralManager stopScan];
        activeCentralManager = nil;
    }
    sendCharacteristic = nil;
    recvCharacteristic = nil;
}

-(BOOL)IsDisconnected{
    return activeCentralManager == nil;
}

/* CBCentralManager委托 */
-(void)centralManagerDidUpdateState:(CBCentralManager *)central{
    NSLog(@"Delegate UpdateState...");

    if([central state] == CBCentralManagerStatePoweredOn) {
        NSLog(@"ScanForPeripheral...");
        [activeCentralManager scanForPeripheralsWithServices : nil options : 0];
    }
}

-(void)centralManager:(CBCentralManager *)central didDiscoverPeripheral : (CBPeripheral *)peripheral advertisementData : (NSDictionary<NSString *, id> *)advertisementData RSSI : (NSNumber *)RSSI{
    NSLog(@"Delegate Discover...");
    NSLog(@"Find:%@", peripheral.name);

    if([peripheral.name isEqualToString : activeName]) {
        NSLog(@"Find Device!");

        [activeCentralManager stopScan];

        activePeripheral = peripheral;
        [activeCentralManager connectPeripheral : activePeripheral options : nil];
    }
}

-(void)centralManager:(CBCentralManager *)central didConnectPeripheral : (CBPeripheral *)peripheral{
    NSLog(@"Delegate Connect...");
    peripheral.delegate = self;
    [peripheral discoverServices : nil];
}

-(void)centralManager:(CBCentralManager *)central didFailToConnectPeripheral : (CBPeripheral *)peripheral error : (NSError *)error{
    NSLog(@"Delgate Connect Failed...");
}

-(void)centralManager : (CBCentralManager *)central didDisconnectPeripheral : (CBPeripheral *)peripheral error : (NSError *)error{
    NSLog(@"Delegate Disconnect...");

    // 断开重连
    if(activePeripheral != nil) {
        [activeCentralManager connectPeripheral : activePeripheral options : nil];
    }
}

/* CBPeripheral委托 */
-(void)peripheral:(CBPeripheral *)peripheral didDiscoverServices : (NSError *)error{
    if(!error) {
        NSLog(@"DiscoverServices...");

        for(CBService* s in peripheral.services) {
            [peripheral discoverCharacteristics : nil forService : s];
        }
    }
}

-(void)peripheral:(CBPeripheral *)peripheral didDiscoverCharacteristicsForService : (CBService *)service error : (NSError *)error{
    NSLog(@"DiscoverCharacteristicsForService[%@]...", service.UUID.description);
    if(!error) {
        for(CBCharacteristic* c in service.characteristics) {
            [peripheral discoverDescriptorsForCharacteristic : c];

            if(recvCharacteristic == nil &&[c.UUID.description isEqualToString : rUuid]) {
                [peripheral readValueForCharacteristic : c];
                [peripheral setNotifyValue : YES forCharacteristic : c];
                recvCharacteristic = c;
            }
            if(sendCharacteristic == nil &&[c.UUID.description isEqualToString : sUuid]) {
                [peripheral setNotifyValue : YES forCharacteristic : c];
                sendCharacteristic = c;
            }
        }
    }
}

-(void)peripheral:(CBPeripheral *)peripheral didDiscoverDescriptorsForCharacteristic : (CBCharacteristic *)characteristic error : (NSError *)error{
    NSLog(@"DiscoverDescriptorsForCharacteristic[%@]...", characteristic.UUID.description);
}

-(void)peripheral : (CBPeripheral *)peripheral didUpdateValueForCharacteristic : (CBCharacteristic *)characteristic error : (NSError *)error{
    NSLog(@"Delegate UpdateValueForCharacteristic...");

    if(!error) {
        NSData* recvData = characteristic.value;
        Byte* pData = (Byte*)[recvData bytes];

        NSMutableString* recvAscii = [[NSMutableString alloc] init];
        for(int i = 0;i < recvData.length; ++i) {
            [recvAscii appendFormat : @"%02X", pData[i]];
        }
        NSLog(@"%@ RECV[%lu]:%@", characteristic.UUID.description, (unsigned long)recvData.length, recvAscii);

        if([recvData length] == 1 && pData[0] == 0x00) {
            return;
        }

        [cacheLock lock];
        [cacheBuff appendData : recvData];
        recvFlag = YES;
        [cacheLock unlock];
    }
}

-(void)peripheral:(CBPeripheral *)peripheral didWriteValueForCharacteristic : (CBCharacteristic *)characteristic error : (NSError *)error{
    NSLog(@"Delegate WriteValueForCharacteristic...");

    sendFlag = YES;
}

@end
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_BLUETOOTHDEVICE_CPP_
//========================================================= 