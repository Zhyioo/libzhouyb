//========================================================= 
/**@file WinHidTestLinker.h 
 * @brief Window下HID设备连接器 
 * 
 * @date 2015-01-24   20:21:47 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_WINHIDTESTLINKER_H_
#define _LIBZHOUYB_WINHIDTESTLINKER_H_
//--------------------------------------------------------- 
#include "TestLinkerHelper.h"
#include "../../../include/Extension.h"
#include "../container/HidICC_TestContainer.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace test {
//--------------------------------------------------------- 
/**
 * @brief HID设备的连接器  
 */ 
struct WinHidTestLinker : public TestLinker<HidDevice>, public TestLinker<FixedHidTestDevice>
{
    /**
     * @brief 查找指定名称的设备并打开
     * @date 2016-04-30 10:30
     * 
     * @param [in] dev 需要操作的设备
     * @param [in] arg 配置项参数
     * - 参数
     *  - Name 需要操作的设备名称 
     *  - WaitTime [default:DEV_WAIT_TIMEOUT] 读取的超时时间
     *  - OperatorInterval [default:DEV_OPERATOR_INTERVAL] 每次读取的间隔
     *  - TransmitMode [default:ControlTransmit] HID传输方式
     *  - SendComand [default:""] 连接成功后需要发送的控制指令
     *  - RecvCommand [default:""] 发送控制指令后的回应码
     * .
     * @param [in] printer 文本输出器
     */
    virtual bool Link(HidDevice& dev, IArgParser<string, string>& arg, TextPrinter& printer)
    {
        bool bLink = false;
        string hidName = arg["Name"].To<string>();
        if(HidDeviceHelper::OpenDevice<HidDevice>(dev, hidName.c_str()) == DevHelper::EnumSUCCESS)
        {
            HidDevice::TransmitMode transmitMode = HidDevice::StringToMode(arg["TransmitMode"].To<string>().c_str());
            dev.SetTransmitMode(transmitMode);

            if(!TestLinkerHelper::LinkTimeoutBehavior(dev, arg, printer))
                return false;
            if(!TestLinkerHelper::LinkCommand(dev, arg, printer))
                return false;

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

    /// 连接设备,增加FixedInput,FixedOutput参数配置项
    virtual bool Link(FixedHidTestDevice& dev, IArgParser<string, string>& arg, TextPrinter& printer)
    {
        dev.FixedInput = arg["FixedInput"].To<size_t>(0);
        dev.FixedOutput = arg["FixedOutput"].To<size_t>(0);
        return Link(dev.Base(), arg, printer);
    }
    /// 关闭设备 
    virtual bool UnLink(FixedHidTestDevice& dev, TextPrinter& printer)
    {
        return UnLink(dev.Base(), printer);
    }
};
//--------------------------------------------------------- 
} // namespace test 
} // namespace application 
} // namespace zhou_yb 
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_WINHIDTESTLINKER_H_
//========================================================= 