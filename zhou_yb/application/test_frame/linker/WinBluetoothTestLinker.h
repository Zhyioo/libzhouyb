//========================================================= 
/**@file WinBluetoothTestLinker.h
 * @brief 
 * 
 * @date 2016-01-23   17:10:13
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../TestFrame.h"

#include "../../../include/BaseDevice.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace test {
//--------------------------------------------------------- 
/// 查找是否有指定地址的远程蓝牙设备
template<class TBluetoothDevice>
struct WinBluetoothAddressTestLinker : public TestLinker < BluetoothDevice >
{
    /**
     * @brief 根据地址查找是否有指定的蓝牙设备
     * 
     * @param [in] dev 需要操作的设备
     * @param [in] devArg 需要匹配的蓝牙地址,格式为:"XX:XX:XX:XX:XX:XX"
     * @param [in] printer 信息输出器
     */
    virtual bool Link(BluetoothDevice& dev, const char* devArg, TextPrinter& printer)
    {
        // 枚举本地蓝牙 
        list<BluetoothDevice::device_info> localdevlist;
        if(dev.EnumLocalDevice(localdevlist, true) < 1)
        {
            printer.TextPrint(TextPrinter::TextError, "没有识别到本地蓝牙设备");
            return false;
        }
        // 枚举所有的远程设备 
        list<BluetoothDevice::device_info> devlist;
        // 没有枚举到设备 
        if(dev.EnumRemoteDevice(devlist) < 1)
        {
            printer.TextPrint(TextPrinter::TextError, "没有识别到远程蓝牙设备");
            return false;
        }

        list<BluetoothDevice::device_info>::iterator itr;
        ByteArray bthAddress(devArg);
        for(itr = devlist.begin();itr != devlist.end(); ++itr)
        {
            string adr = BluetoothDevice::AddressToString(itr->Address);
            if(StringConvert::Contains(ByteArray(adr.c_str(), adr.length()), bthAddress, true))
            {
                return true;
            }
        }
        return false;
    }
};
//--------------------------------------------------------- 
} // test
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
//========================================================= 