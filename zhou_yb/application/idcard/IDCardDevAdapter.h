//========================================================= 
/**@file IDCardDevAdapter.h
 * @brief HID二代证适配器(在发送读信息指令后等待一定延时) 
 * 
 * @date 2015-08-18   18:54:14
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../include/Base.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace idcard {
//--------------------------------------------------------- 
/// 在发送读信息指令后等待一定延时获取信息 
class IDCardDevAdapter : 
    public IInteractiveTrans,
    public BaseDevAdapterBehavior<IInteractiveTrans>,
    public TimeoutBehavior,
    public RefObject
{
protected:
    //----------------------------------------------------- 
    /// 初始化 
    inline void _init()
    {
        _waitInterval = 50;
        _waitTimeout = 1000;
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    IDCardDevAdapter() : BaseDevAdapterBehavior<IInteractiveTrans>() { _init(); }
    //----------------------------------------------------- 
    /// 发送数据 
    virtual bool Write(const ByteArray& data)
    {
        if(_pDev.IsNull())
            return false;

        bool bRet = _pDev->Write(data);
        if(bRet)
        {
            Timer::Wait(_waitInterval);
            if(data.GetLength() > 8 && data[7] == 0x30 && data[8] == 0x01)
            {
                Timer::Wait(_waitTimeout);
            }
        }

        return bRet;
    }
    /// 接收数据 
    virtual bool Read(ByteBuilder& data)
    {
        if(_pDev.IsNull())
            return false;

        return _pDev->Read(data);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace idcard
} // namespace application 
} // namespace zhou_yb 
//========================================================= 