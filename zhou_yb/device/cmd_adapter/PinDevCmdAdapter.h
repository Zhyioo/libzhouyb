//========================================================= 
/**@file PinDevCmdAdapter.h 
 * @brief 密码键盘指令适配器 
 * 
 * @date 2014-11-02   15:34:13 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_PINDEVCMDADAPTER_H_
#define _LIBZHOUYB_PINDEVCMDADAPTER_H_
//--------------------------------------------------------- 
#include "../../include/Base.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace device {
namespace cmd_adapter {
//--------------------------------------------------------- 
/**
 * @brief 负责按照密码键盘所定义的指令集处理数据
 * 
 * - 格式:
 *  - 发送:直接发送 
 *  - 接收:判断 STX OK ETX 或者 STX DATA(拆字) ETX 为成功,如有数据进行解析, STX ER ETX 为失败 
 * .
 */ 
class PinDevCmdAdapter : 
    public IInteractiveTrans, 
    public DevAdapterBehavior<IInteractiveTrans>,
    public RefObject
{
protected:
    //----------------------------------------------------- 
    /// 临时缓冲区 
    ByteBuilder _tmpBuff;
    //----------------------------------------------------- 
    inline void _init()
    {
        IsAllowFold = true;
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    PinDevCmdAdapter() : DevAdapterBehavior<IInteractiveTrans>() { _init(); }
    //----------------------------------------------------- 
    /// 处理状态码,并解析数据 
    /**
     * @brief 处理状态码,并解析数据 
     * @warning 在失败时,data里面仍然有数据,存放错误码
     * @date 20160715 11:49
     */
    virtual bool Read(ByteBuilder& data) 
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        const byte STX = 0x02;
        const byte ETX = 0x03;

        _tmpBuff.Clear();

        // 读取失败 
        bool bRet = false;
        do
        {
            bRet = _pDev->Read(_tmpBuff);
            size_t len = _tmpBuff.GetLength();
            if(bRet && len > 0 && _tmpBuff[len - 1] == ETX)
                break;
        } while(bRet);
        ASSERT_FuncErrRet(bRet, DeviceError::RecvErr);
        ASSERT_FuncErrInfoRet(_tmpBuff.GetLength() > 2, DeviceError::RecvFormatErr, "长度错误");
        // 格式必须符合02,03
        byte tmpETX = _tmpBuff[_tmpBuff.GetLength() - 1];
        ASSERT_FuncErrRet(_tmpBuff[0] == STX && tmpETX == ETX, DeviceError::RecvFormatErr);

        // 长度<3则直接返回数据 
        if(_tmpBuff.GetLength() < 4)
        {
            data += static_cast<byte>(_tmpBuff[1]);
            return _logRetValue(true);
        }
        // +1跳过STX,长度-2跳过STX和ETX 
        ByteArray subArray = _tmpBuff.SubArray(1, _tmpBuff.GetLength() - 2);
        // 格式为: 02 ER DATA 03 的格式
        if(StringConvert::StartWith(subArray, "E"))
        {
            data += subArray;
            return _logRetValue(false);
        }
        // 格式为: 02 OK DATA 03 的格式
        if(StringConvert::StartWith(subArray, "OK"))
            subArray = subArray.SubArray(2);
        if(IsAllowFold)
        {
            // 拆字还原数据
            ByteConvert::Fold(subArray, data);
        }
        else
        {
            data.Append(subArray);
        }

        return _logRetValue(true);
    }
    /// 发送数据(直接发送不做任何处理)
    virtual bool Write(const ByteArray& data) 
    {
        if(!IsValid())
            return false;
        return _pDev->Write(data);
    }
    //----------------------------------------------------- 
    /// 是否允许拆字(不允许则直接返回所有数据) 
    bool IsAllowFold;
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace cmd_adapter
} // namespace device 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_PINDEVCMDADAPTER_H_
//========================================================= 