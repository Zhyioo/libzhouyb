//========================================================= 
/**@file WinFileTestLinker.h 
 * @brief Windows下文件设备连接器 
 * 
 * @date 2015-01-24   20:21:36 
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
 * @brief FileBaseDevice设备的连接器
 * @param [in] TFileBaseDevice 基于FileBaseDevice的子类设备 
 * @param [in] waitTimeout 读取的超时时间
 * @param [in] operatorInterval 每次读取的间隔
 */
template<class TFileBaseDevice, uint waitTimeout = DEV_WAIT_TIMEOUT, uint operatorInterval = DEV_OPERATOR_INTERVAL>
struct WinFileTestLinker : public TestLinker<TFileBaseDevice>
{
    /// 查找制定名称的设备并打开 
    virtual bool Link(TFileBaseDevice& dev, const char* sArg, TextPrinter& )
    {
        if(dev.Open(sArg))
        {
            dev.SetOperatorInterval(operatorInterval);
            dev.SetWaitTimeout(waitTimeout);

            return true;
        }
        return false;
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
//========================================================= 