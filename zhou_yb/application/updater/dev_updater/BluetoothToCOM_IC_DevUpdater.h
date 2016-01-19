//========================================================= 
/**@file BluetoothToCOM_IC_DevUpdater.h
 * @brief 蓝牙转串口IC卡设备固件升级程序 
 * 
 * @date 2015-06-14   17:43:57
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "COM_IC_DevUpdater.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace updater {
//--------------------------------------------------------- 
/// 串口IC卡读卡器固件升级程序 
class BluetoothToCOM_IC_DevUpdater : public COM_IC_DevUpdater
{
public:
    //----------------------------------------------------- 
    /**
     * @brief 升级每一行数据
     */
    virtual bool UpdateLine(const ByteArray& bin)
    {
        // 将随机数与hex数据异或 
        ByteBuilder tmpLine(20);
        tmpLine += static_cast<byte>(bin.GetLength());
        tmpLine += bin;

        ByteConvert::Xor(_random, tmpLine.SubArray(1));

        // 累加到一个缓冲区中进行发送 
        _tmpBuffer += tmpLine;

        ++_updateCount;
        // 判断状态码 
        if((_updateCount == _swCount) || DevUpdaterConvert::IsEOF(tmpLine))
        {
            // 直接发送数据包 
            if(!_dev.Write(_tmpBuffer))
                return false;

            _tmpBuffer.Clear();
            return _WaitSW();
        }

        return true;
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace updater
} // namespace application
} // namespace zhou_yb
//========================================================= 