//========================================================= 
/**@file CommandFilter.h
 * @brief 指令过滤器 
 * 
 * @date 2015-07-20   17:17:28
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_COMMANDFILTER_H_
#define _LIBZHOUYB_COMMANDFILTER_H_
//--------------------------------------------------------- 
#include "../../include/BaseDevice.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace device {
namespace cmd_adapter {
//--------------------------------------------------------- 
/// 按照字节截取数据,过滤其他的数据 
class CommandFilter :
    public IInteractiveTrans,
    public BaseDevAdapterBehavior<IInteractiveTrans>,
    public RefObject
{
protected:
    //----------------------------------------------------- 
    /// 接收字节 
    byte _sTx;
    /// 截止字节 
    byte _eTx;
    /// 临时缓冲区 
    ByteBuilder _tmpBuffer;
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    CommandFilter() : BaseDevAdapterBehavior<IInteractiveTrans>() 
    {
        IsMultiSTX = false;
    }
    //----------------------------------------------------- 
    /// 写数据 
    virtual bool Write(const ByteArray& data)
    {
        if(!BaseDevAdapterBehavior<IInteractiveTrans>::IsValid())
            return false;
        return _pDev->Write(data);
    }
    /// 读数据 
    virtual bool Read(ByteBuilder& data)
    {
        if(!BaseDevAdapterBehavior<IInteractiveTrans>::IsValid())
            return false;
        bool bRet = false;
        bool isContinue = true;
        bool hasSTX = false;
        size_t lastLen = data.GetLength();
        do
        {
            _tmpBuffer.Clear();
            bRet = _pDev->Read(_tmpBuffer);
            if(!bRet) break;

            for(size_t i = 0;i < _tmpBuffer.GetLength(); ++i)
            {
                // 查找STX 
                if(!hasSTX)
                {
                    if(_tmpBuffer[i] == _sTx)
                    {
                        data += _tmpBuffer[i];
                        hasSTX = true;
                        continue;
                    }
                }
                else
                {
                    if(IsMultiSTX && _tmpBuffer[i] == _sTx)
                    {
                        data.RemoveTail(data.GetLength() - lastLen);
                    }
                    data += _tmpBuffer[i];
                    if(_tmpBuffer[i] == _eTx)
                    {
                        isContinue = false;
                        break;
                    }
                }
            }
        } while(isContinue);
        return bRet;
    }
    //----------------------------------------------------- 
    /**
     * @brief 是否过滤多个STX
     *
     * true 只处理最后一个STX
     * false 第一个为STX,后续的为数据
     *
     * @date 20160713 15:05
     */
    bool IsMultiSTX;
    /// 设置读取标志 
    inline void SetReadTX(byte sTx, byte eTx)
    {
        _sTx = sTx;
        _eTx = eTx;
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace cmd_adapter
} // namespace device 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_COMMANDFILTER_H_
//========================================================= 