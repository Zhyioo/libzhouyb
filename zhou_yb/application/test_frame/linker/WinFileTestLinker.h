//========================================================= 
/**@file WinFileTestLinker.h 
 * @brief Windows下文件设备连接器 
 * 
 * @date 2015-01-24   20:21:36 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_WINFILETESTLINKER_H_
#define _LIBZHOUYB_WINFILETESTLINKER_H_
//--------------------------------------------------------- 
#include "TestLinkerHelper.h"
#include "../../../include/Extension.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace test {
//--------------------------------------------------------- 
/**
 * @brief FileBaseDevice设备的连接器
 */
template<class TFileBaseDevice>
struct WinFileTestLinker : public TestLinker<TFileBaseDevice>
{
    /**
     * @brief 查找制定名称的设备并打开
     * @date 2016-04-30 14:23
     * 
     * @param [in] dev 
     * @param [in] arg 
     * - 参数
     *  - Name 需要打开的设备名
     *  - WaitTime [default:DEV_WAIT_TIMEOUT] 读取的超时时间
     *  - OperatorInterval [default:DEV_OPERATOR_INTERVAL] 每次读取的间隔
     *  - SendComand [default:""] 连接成功后需要发送的控制指令
     *  - RecvCommand [default:""] 发送控制指令后的回应码
     * .
     * @param [in] printer 文本输出器
     */
    virtual bool Link(TFileBaseDevice& dev, IArgParser<string, string>& arg, TextPrinter& printer)
    {
        string devName = devName = arg["Name"].To<string>();;
        if(!dev.Open(devName.c_str()))
            return false;
        
        if(!TestLinkerHelper::LinkTimeoutBehavior(dev, arg, printer))
            return false;
        if(!TestLinkerHelper::LinkCommand(dev, arg, printer))
            return false;

        return true;
    }
    /// 关闭设备 
    virtual bool UnLink(TFileBaseDevice& dev, TextPrinter& )
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
#endif // _LIBZHOUYB_WINFILETESTLINKER_H_
//========================================================= 