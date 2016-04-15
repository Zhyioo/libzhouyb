//========================================================= 
/**@file H002_ReaderDevAdapter.h
 * @brief 
 * 
 * @date 2016-03-11   21:15:24
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_H002_READERDEVADAPTER_H_
#define _LIBZHOUYB_H002_READERDEVADAPTER_H_
//--------------------------------------------------------- 
#include "../../include/Base.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace device {
//--------------------------------------------------------- 
/// H002设备控制器
class H002_ReaderDevAdapter : public DevAdapterBehavior<IInteractiveTrans>
{
public:
    /// Led模式
    enum LedMode
    {
        /// 打开
        LedOn,
        /// 关闭
        LedOff,
        /// 闪烁
        LedLight
    };
    /// Led模块
    enum LedModule
    {
        /// 磁条Led
        LedMagnetic,
        /// 接触式IC Led
        LedContact,
        /// 非接触式IC Led
        LedContactless
    };
    /// 生成Led控制字节
    static void MakeLed(LedModule module, LedMode mode, byte& ledMode)
    {
        switch(mode)
        {
        case LedOn:
            BitConvert::Set(ledMode, module, 1);
            BitConvert::Set(ledMode, module + 3, 0);
            break;
        case LedOff:
            BitConvert::Set(ledMode, module, 0);
            BitConvert::Set(ledMode, module + 3, 0);
            break;
        case LedLight:
            BitConvert::Set(ledMode, module, 1);
            BitConvert::Set(ledMode, module + 3, 1);
            break;
        }
    }
    /// Led控制器
    bool LedControl(byte ledMode)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();
        LOGGER(_log << "LedMode:<" << _bit(ledMode) << ">\n");
        ByteBuilder cmd(4);
        DevCommand::FromAscii("1B 50", cmd);
        cmd += ledMode;
        ASSERT_FuncErr(_pDev->Write(cmd), DeviceError::SendErr);

        return _logRetValue(true);
    }
};
//--------------------------------------------------------- 
} // namespace device
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_H002_READERDEVADAPTER_H_
//========================================================= 