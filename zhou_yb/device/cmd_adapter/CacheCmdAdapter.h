//========================================================= 
/**@file CacheCmdAdapter.h
 * @brief 
 * 
 * @date 2016-03-31   15:31:27
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../include/Base.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace device {
namespace cmd_adapter {
//--------------------------------------------------------- 
/// 能够缓存收发指令的命令适配器
class CacheCmdAdapter : 
    public IInteractiveTrans, 
    public BaseDevAdapterBehavior<IInteractiveTrans>,
    public RefObject
{
public:
    //----------------------------------------------------- 
    CacheCmdAdapter() : BaseDevAdapterBehavior<IInteractiveTrans>() {}
    //----------------------------------------------------- 
    /// 接收缓冲区 
    ByteBuilder InputStream;
    /// 发送缓冲区 
    ByteBuilder OutputStream;
    //----------------------------------------------------- 
    /// 读数据 
    virtual bool Read(ByteBuilder& data)
    {
        if(!BaseDevAdapterBehavior<IInteractiveTrans>::IsValid())
            return false;
        if(_pDev->Read(data))
        {
            InputStream += data;
            return true;
        }
        return false;
    }
    /// 取数据 
    virtual bool Write(const ByteArray& data)
    {
        if(!BaseDevAdapterBehavior<IInteractiveTrans>::IsValid())
            return false;
        if(_pDev->Write(data))
        {
            OutputStream += data;
            return true;
        }
        return false;
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace cmd_adapter
} // namespace device
} // namespace zhou_yb
//========================================================= 