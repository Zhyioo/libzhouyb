//========================================================= 
/**@file CMB_IC_DevUpdater.h
 * @brief 招行集成键盘IC卡读卡器固件升级程序
 * 
 * @date 2015-04-21   22:04:35
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../../include/Extension.h"

#include "CCID_DevUpdater.h"
#include "HID_IC_DevUpdater.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace updater {
//--------------------------------------------------------- 
/// 招行集成键盘IC卡读卡器固件升级程序 
class CMB_IC_DevUpdater : public CCID_DevUpdater
{
public:
    //----------------------------------------------------- 
    /**
     * @brief 连接设备
     * @param [in] sArg 读卡器名称 
     */ 
    virtual bool Link(const char* sArg = NULL)
    {
        HID_IC_DevUpdater cibUpdater;

        list<string> arglist;
        list<string>::iterator itr;

        string hidArg;
        string ccidArg;

        StringHelper::Split(sArg, arglist, ',');
        if(arglist.size() < 1)
        {
            _logErr(DeviceError::ArgRangeErr, "参数过少");
            return _logRetValue(false);
        }

        hidArg = arglist.front();
        itr = list_helper<string>::index_of(arglist, 1);
        ccidArg = (*itr);

        cibUpdater.Link(hidArg.c_str());
        return CCID_DevUpdater::Link(ccidArg.c_str());
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace updater
} // namespace application
} // namespace zhou_yb
//========================================================= 