//========================================================= 
/**@file WinComTestLinker.h
 * @brief Windows下串口设备连接器 
 * 
 * @date 2015-06-24   19:56:44
 * @author Zhyioo 
 * @version 1.0
 */
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_WINCOMTESTLINKER_H_
#define _LIBZHOUYB_WINCOMTESTLINKER_H_
//--------------------------------------------------------- 
#include "TestLinkerHelper.h"
#include "../../../include/BaseDevice.h"
#include "../../../application/tools/ParamHelper.h"
using zhou_yb::application::tools::ParamHelper;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace test {
//--------------------------------------------------------- 
/**
 * @brief ComDevice设备的连接器
 */
template<class TComDevice>
struct WinComTestLinker : public TestLinker<TComDevice>
{
    /**
     * @brief 查找指定名称的串口设备并打开
     * @date 2016-04-30 14:38
     * 
     * @param [in] dev 需要操作的设备
     * @param [in] sArg 参数
     * - 参数
     *  - Name 设备名称
     *  - WaitTime [default:DEV_WAIT_TIMEOUT] 读取的超时时间
     *  - OperatorInterval [default:DEV_OPERATOR_INTERVAL] 每次读取的间隔
     *  - SendComand [default:""] 连接成功后需要发送的控制指令
     *  - RecvCommand [default:""] 发送控制指令后的回应码
     * .
     * @param [in] printer 文本输出器
     */
    virtual bool Link(TComDevice& dev, const char* sArg, TextPrinter& printer)
    {
        ArgParser cfg;
        size_t count = cfg.Parse(sArg);
        string devName = sArg;
        if(count > 0)
        {
            devName = cfg["Name"].To<string>("AUTO");
        }

        byte gate = 0x00;
        uint baud = CBR_9600;
        uint port = ParamHelper::ParseCOM(devName.c_str(), gate, baud);

        if(ComDeviceHelper::OpenDevice<TComDevice>(dev, port, baud) != DevHelper::EnumSUCCESS)
            return false;
        if(count > 0)
        {
            if(!TestLinkerHelper::LinkTimeoutBehavior(dev, cfg, printer))
                return false;
            if(!TestLinkerHelper::LinkCommand(dev, cfg, printer))
                return false;
        }
        return true;
    }
    /// 关闭设备 
    virtual bool UnLink(TComDevice& dev, TextPrinter&)
    {
        dev.Close();
        return true;
    }
};
//--------------------------------------------------------- 
} // namespace test 
} // namespace application 
} // namespace zhou_yb 
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_WINCOMTESTLINKER_H_
//========================================================= 