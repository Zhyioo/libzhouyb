//========================================================= 
/**@file WinHidTestLinker.h 
 * @brief Window下HID设备连接器 
 * 
 * @date 2015-01-24   20:21:47 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../TestFrame.h"

#include "../../../include/Extension.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace test {
//--------------------------------------------------------- 
/**
 * @brief HID设备的连接器  
 * @param [in] waitTimeout [default:DEV_WAIT_TIMEOUT] 读取的超时时间 
 * @param [in] operatorInterval [default:DEV_OPERATOR_INTERVAL] 每次读取的间隔 
 * @param [in] transmitMode HID传输方式 
 */ 
template<uint waitTimeout = DEV_WAIT_TIMEOUT, 
    uint operatorInterval = DEV_OPERATOR_INTERVAL,
    HidDevice::TransmitMode transmitMode = HidDevice::ControlTransmit>
struct WinHidTestLinker : public TestLinker < HidDevice >
{
    /// 查找指定名称的设备并打开 
    virtual bool Link(HidDevice& dev, const char* sArg, TextPrinter& printer)
    {
        bool bLink = false;
        if(HidDeviceHelper::OpenDevice(dev, sArg) == DevHelper::EnumSUCCESS)
        {
            dev.SetOperatorInterval(operatorInterval);
            dev.SetWaitTimeout(waitTimeout);
            dev.SetTransmitMode(transmitMode);

            bLink = true;
        }
        return bLink;
    }
    /// 关闭设备 
    virtual bool UnLink(HidDevice& dev, TextPrinter& printer)
    {
        dev.Close();
        return true;
    }
};
//--------------------------------------------------------- 
} // namespace test 
} // namespace application 
} // namespace zhou_yb 
//========================================================= 