//========================================================= 
/**@file ICBC_PrinterCmdAdapter.h
 * @brief 
 * 
 * @date 2016-06-12   13:37:00
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_ICBC_PRINTERCMDADAPTER_H_
#define _LIBZHOUYB_ICBC_PRINTERCMDADAPTER_H_
//--------------------------------------------------------- 
#include "../../include/Base.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace printer {
//--------------------------------------------------------- 
class ICBC_PrinterCmdAdapter :
    public IInteractiveTrans,
    public BaseDevAdapterBehavior<IInteractiveTrans>,
    public RefObject
{
protected:
    ByteBuilder _tmpBuffer;
public:
    /// 写数据 
    virtual bool Write(const ByteArray& data)
    {
        if(!BaseDevAdapterBehavior<IInteractiveTrans>::IsValid())
            return false;

        _tmpBuffer.Clear();

        ByteConvert::Expand(data, _tmpBuffer, 0x30);
        bool bRet = _pDev->Write(_tmpBuffer);
        Timer::Wait(DEV_OPERATOR_INTERVAL);
        return bRet;
    }
    /// 读数据 
    virtual bool Read(ByteBuilder& data)
    {
        if(!BaseDevAdapterBehavior<IInteractiveTrans>::IsValid())
            return false;
        _tmpBuffer.Clear();
        bool bRet = _pDev->Read(data);

        ByteConvert::Fold(_tmpBuffer, data);

        return bRet;
    }
};
//--------------------------------------------------------- 
} // namespace printer
} // namespace application 
} // namespace zhou_yb 
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_ICBC_PRINTERCMDADAPTER_H_
//========================================================= 