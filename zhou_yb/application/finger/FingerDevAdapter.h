//========================================================= 
/**@file FingerDevAdapter.h
 * @brief 维尔指纹仪驱动
 * 
 * @date 2016-05-11   12:25:31
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_FINGERDEVADAPTER_H_
#define _LIBZHOUYB_FINGERDEVADAPTER_H_
//--------------------------------------------------------- 
#include "../../include/Base.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace finger {
//--------------------------------------------------------- 
class WE_FingerDevAdapter : public DevAdapterBehavior<IInteractiveTrans>
{
protected:
    /// 接收数据
    bool _TransCommand(const ByteArray& cmd, ByteBuilder& recv)
    {
        ASSERT_FuncErr(_pDev->Write(cmd), DeviceError::SendErr);
        ByteBuilder tmp(32);
        ASSERT_FuncErr(ComICCardCmdAdapter::RecvByFormat(_pDev, tmp), DeviceError::RecvErr);
        LOGGER(_log.WriteLine("RecvByFormat:").WriteLine(tmp));
        ByteBuilder data(32);
        ByteConvert::Fold(tmp.SubArray(5, tmp.GetLength() - 8), data);
        recv += data;
        return true;
    }
public:
    /// 取版本信息
    bool GetVersion(ByteBuilder& version)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        return _logRetValue(true);
    }
    /// 取特征
    bool GetFeature(ByteBuilder& data)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        ByteBuilder cmd(8);
        DevCommand::FromAscii("02 30 30 30 34 30 3C 30 30 30 30 30 30 30 38 03", cmd);
        ASSERT_FuncErrRet(_TransCommand(cmd, data), DeviceError::TransceiveErr);
        return _logRetValue(true);
    }
    /// 取模板
    bool GetTemplate(ByteBuilder& data)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        ByteBuilder cmd(8);
        DevCommand::FromAscii("02 30 30 30 34 30 3B 30 30 30 30 30 30 30 3F 03", cmd);
        ASSERT_FuncErrRet(_TransCommand(cmd, data), DeviceError::TransceiveErr);
        return _logRetValue(true);
    }
    /// 取指纹图片
    bool GetImage(ByteBuilder& image)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();
        return _logRetValue(true);
    }
};
//--------------------------------------------------------- 
} // namespace finger
} // namespace application 
} // namespace zhou_yb 
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_FINGERDEVADAPTER_H_
//========================================================= 