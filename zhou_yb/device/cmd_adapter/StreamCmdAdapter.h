//========================================================= 
/**@file StreamCmdAdapter.h 
 * @brief 流指令适配器 
 * 
 * @date 2013-08-28   23:50:43 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_STREAMCMDADAPTER_H_
#define _LIBZHOUYB_STREAMCMDADAPTER_H_
//--------------------------------------------------------- 
#include "../../include/Base.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace device {
namespace cmd_adapter {
//--------------------------------------------------------- 
/**
 * @brief 可以将一个底层设备已经读写完毕的指令在一个适配器上再次发送 
 * 
 * @attention 还可以用来搭配CmdAdapter解析指令 
 */  
class StreamCmdAdapter : public IInteractiveTrans, public RefObject
{
public:
    //----------------------------------------------------- 
    /// 接收缓冲区 
    ByteArray InputStream;
    /// 发送缓冲区 
    ByteBuilder OutputStream;
    //----------------------------------------------------- 
    /// 读数据 
    virtual bool Read(ByteBuilder& data)
    {
        data += InputStream;
        return true;
    }
    /// 取数据 
    virtual bool Write(const ByteArray& data)
    {
        OutputStream += data;
        return true;
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace cmd_adapter
} // namespace device 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_STREAMCMDADAPTER_H_
//========================================================= 