//========================================================= 
/**@file WinComTestLinker.h
 * @brief Windows下串口设备连接器 
 * 
 * @date 2015-06-24   19:56:44
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../TestFrame.h"

#include "../../../include/BaseDevice.h"
#include "../../../application/tools/ParamHelper.h"
using zhou_yb::application::tools::ParamHelper;
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
template<class TComDevice = ComDevice, uint waitTimeout = DEV_WAIT_TIMEOUT, uint operatorInterval = DEV_OPERATOR_INTERVAL>
struct WinComTestLinker : TestLinker < TComDevice >
{
    /// 查找制定名称的串口设备并打开 
    virtual bool Link(TComDevice& dev, const char* sArg, TextPrinter&)
    {
        list<string> args;
        StringHelper::Split(sArg, args, ',');

        // 查找串口 
        uint port = 0;
        const char* pParam = NULL;
        if(args.size() > 0)
            pParam = args.front().c_str();
        else
            pParam = "AUTO";

        port = ParamHelper::ParseCOM(pParam);

        // 波特率 
        uint baud = 9600;
        size_t index = 1;
        if(args.size() > index)
        {
            string sBaud = (*list_helper<string>::index_of(args, index));
            if(sBaud.length() > 0)
            {
                baud = ArgConvert::FromString<uint>(sBaud.c_str());
            }
        }

        if(dev.Open(port, baud))
        {
            dev.SetOperatorInterval(operatorInterval);
            dev.SetWaitTimeout(waitTimeout);

            return true;
        }
        return false;
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
//========================================================= 