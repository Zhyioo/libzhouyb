//========================================================= 
/**@file TestLinkerHelper.h
 * @brief 测试连接器辅助操作(对特定的接口赋值)
 * 
 * @date 2016-04-30   14:05:58
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_TESTLINKERHELPER_H_
#define _LIBZHOUYB_TESTLINKERHELPER_H_
//--------------------------------------------------------- 
#include "../TestFrame.h"
#include "../../../include/BaseDevice.h"
#include "../../../include/Device.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace test {
//--------------------------------------------------------- 
/// 带格式控制字节的HID设备
typedef TestDevice<HidDevice, HidFixedCmdAdapter<HidDevice> > FixedHidTestDevice;
//--------------------------------------------------------- 
/// 带命令控制的测试连接器
class TestLinkerHelper
{
protected:
    TestLinkerHelper() {}
public:
    /**
     * @brief 设置启动时交互的指令
     * @date 2016-04-30 14:21
     * 
     * @param [in] dev 需要操作的设备
     * @param [in] arg 配置项参数
     * - 参数
     *  - SendComand [default:""] 连接成功后需要发送的控制指令
     *  - RecvCommand [default:""] 发送控制指令后的回应码
     * .
     * @param [in] printer 文本输出器
     */
    static bool LinkCommand(IInteractiveTrans& dev, IArgParser<string, string>& arg, TextPrinter& printer)
    {
        string cmd = arg["SendCommand"].To<string>();
        ByteBuilder cmdHex(8);
        DevCommand::FromAscii(cmd.c_str(), cmdHex);
        if(!cmdHex.IsEmpty())
        {
            if(dev.Write(cmdHex))
            {
                cmd = arg["RecvCommand"].To<string>();
                cmdHex.Clear();
                DevCommand::FromAscii(cmd.c_str(), cmdHex);
                if(!cmdHex.IsEmpty())
                {
                    // 比较接收的数据是否一致
                    ByteBuilder recv(8);
                    if(!dev.Read(recv))
                    {
                        LOGGER(printer.TextPrint(TextPrinter::TextLogger, "接收回应数据失败"));
                        return false;
                    }
                    if(!StringConvert::Compare(recv, cmdHex))
                    {
                        LOGGER(printer.TextPrint(TextPrinter::TextLogger, "回应数据不匹配"));
                        return false;
                    }
                }
            }
        }
        return true;
    }
    /**
     * @brief 设置超时时间和操作间隔
     * @date 2016-04-30 14:19
     * 
     * @param [in] dev 需要操作的设备
     * @param [in] arg 配置项参数 
     * - 参数
     *  - WaitTime [default:DEV_WAIT_TIMEOUT] 读取的超时时间
     *  - OperatorInterval [default:DEV_OPERATOR_INTERVAL] 每次读取的间隔
     * .
     * @param [in] printer 文本输出器
     */
    static bool LinkTimeoutBehavior(ITimeoutBehavior& dev, IArgParser<string, string>& arg, TextPrinter& printer)
    {
        uint waitTimeout = arg["WaitTime"].To<uint>(DEV_WAIT_TIMEOUT);
        uint operatorInterval = arg["OperatorInterval"].To<uint>(DEV_OPERATOR_INTERVAL);

        dev.SetOperatorInterval(operatorInterval);
        dev.SetWaitTimeout(waitTimeout);
        return true;
    }
};
//--------------------------------------------------------- 
} // namespace test 
} // namespace application 
} // namespace zhou_yb 
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_TESTLINKERHELPER_H_
//========================================================= 