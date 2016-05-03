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
struct WinHidTestLinker : public TestLinker<HidDevice>, public TestLinker<TestDevice<HidDevice, HidFixedCmdAdapter<HidDevice> > >
{
    /**
     * @brief 查找指定名称的设备并打开
     * @date 2016-04-30 10:30
     * 
     * @param [in] dev 需要操作的设备
     * @param [in] sArg 配置项参数
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
    virtual bool Link(HidDevice& dev, const char* sArg, TextPrinter& printer)
    {
        bool bLink = false;
        ArgParser cfg;
        string hidName = sArg;
        size_t count = cfg.Parse(sArg);
        if(count > 0)
        {
            hidName = cfg["Name"].To<string>();
        }
        if(HidDeviceHelper::OpenDevice<HidDevice>(dev, hidName.c_str()) == DevHelper::EnumSUCCESS)
        {
            HidDevice::TransmitMode transmitMode = HidDevice::StringToMode(cfg["TransmitMode"].To<string>().c_str());
            dev.SetTransmitMode(transmitMode);

            if(count > 0)
            {
                if(!TestLinkerHelper::LinkTimeoutBehavior(dev, cfg, printer))
                    return false;
                if(!TestLinkerHelper::LinkCommand(dev, cfg, printer))
                    return false;
            }

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
    virtual bool Link(TestDevice<HidDevice, HidFixedCmdAdapter<HidDevice> >& dev, const char* sArg, TextPrinter& printer)
    {
        ArgParser cfg;
        if(cfg.Parse(sArg))
        {
            dev.FixedInput = cfg["FixedInput"].To<size_t>(0);
            dev.FixedOutput = cfg["FixedOutput"].To<size_t>(0);
        }
        return Link(dev.Base(), sArg, printer);
    }
    /// 关闭设备 
    virtual bool UnLink(TestDevice<HidDevice, HidFixedCmdAdapter<HidDevice> >& dev, TextPrinter& printer)
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