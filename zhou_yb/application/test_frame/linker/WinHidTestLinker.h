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
#include "../TestFrame.h"

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
     *  - WaitTimeout [default:DEV_WAIT_TIMEOUT] 读取的超时时间
     *  - OperatorInterval [default:DEV_OPERATOR_INTERVAL] 每次读取的间隔
     *  - TransmitMode [default:ControlTransmit] HID传输方式
     * .
     * @param [in] printer 日志输出器
     * 
     * @return bool 
     */
    virtual bool Link(HidDevice& dev, const char* sArg, TextPrinter& printer)
    {
        bool bLink = false;
        ArgParser cfg;
        string hidName = sArg;
        if(cfg.Parse(sArg))
        {
            hidName = cfg["Name"].To<string>();
        }
        if(HidDeviceHelper::OpenDevice(dev, hidName.c_str()) == DevHelper::EnumSUCCESS)
        {
            uint waitTimeout = cfg["WaitTimeout"].To<uint>(DEV_WAIT_TIMEOUT);
            uint operatorInterval = cfg["OperatorInterval"].To<uint>(DEV_OPERATOR_INTERVAL);
            HidDevice::TransmitMode transmitMode = static_cast<HidDevice::TransmitMode>(cfg["TransmitMode"].To<int>(HidDevice::ControlTransmit));

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

    /// 连接设备,增加FixedLength参数配置项
    virtual bool Link(TestDevice<HidDevice, HidFixedCmdAdapter<HidDevice> >& dev, const char* sArg, TextPrinter& printer)
    {
        ArgParser cfg;
        if(cfg.Parse(sArg))
        {
            dev.FixedLength = cfg["FixedLength"].To<size_t>();
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