//========================================================= 
/**@file IOS_BluetoothInvoker.h
 * @brief IOSÏÂÀ¶ÑÀ»¥²Ù×÷Àà
 * 
 * @date 2016-06-09   11:35:14
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_IOS_BLUETOOTHINVOKER_H_
#define _LIBZHOUYB_IOS_BLUETOOTHINVOKER_H_
//--------------------------------------------------------- 
#import "BluetoothDevice.h"
//--------------------------------------------------------- 
class BluetoothDeviceInvoker :
    public IInteractiveTrans,
    public TimeoutBehavior,
    public DeviceBehavior,
    public RefObject
{
protected:
    id<BluetoothDeviceDelegate> _dev;
public:
    BluetoothDeviceInvoker()
    {
        _dev = nil;
    }
    inline void SelectDevice(id<BluetoothDeviceDelegate> dev)
    {
        _dev = dev;
    }
    bool IsValid() const
    {
        return _dev != nil;
    }
    virtual bool Read(ByteBuilder& data)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();
        LOGGER(_log << "Invoker RECV:" << data << endl);

        NSMutableData* rBuff = [[NSMutableData alloc] init];

        Timer timer;
        while(timer.Elapsed() < _waitTimeout)
        {
            if([_dev IsReaded : rBuff] == YES)
            {
                byte* ptr = (byte*)[rBuff bytes];
                data.Append(ByteArray(ptr, [rBuff length]));
                return _logRetValue(true);
            }

            Timer::Wait(_waitInterval);
        }

        _logErr(DeviceError::RecvErr);
        return _logRetValue(false);
    }
    virtual bool Write(const ByteArray& data)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();
        LOGGER(_log << "Invoker SEND:" << data << endl);

        NSData* sBuff = [[NSData alloc] initWithBytes:data.GetBuffer() length : data.GetLength()];
        [_dev Clean];
        [_dev Write : sBuff];
        Timer timer;
        while(timer.Elapsed() < _waitTimeout)
        {
            if([_dev IsWrited] == YES)
            {
                return _logRetValue(true);
            }
            Timer::Wait(_waitInterval);
        }

        _logErr(DeviceError::SendErr);
        return _logRetValue(false);
    }
};
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_IOS_BLUETOOTHINVOKER_H_
//========================================================= 