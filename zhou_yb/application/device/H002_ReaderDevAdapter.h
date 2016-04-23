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
#include "../../device/magnetic/MagneticDevAdapter.h"
using zhou_yb::device::magnetic::MagneticDevAdapter;
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
        /// 非接触式IC Led
        LedContactless,
        /// 接触式IC Led
        LedContact,
        /// 磁条Led
        LedMagnetic
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
    /**
     * @brief 等待磁条数据
     * @date 2016-04-23 23:08
     * 
     * @param [out] hasMag 是否有磁条数据
     * @param [out] tr1 获取到的一磁道信息
     * @param [out] tr2 获取到的二磁道信息
     * @param [out] tr3 获取到的三磁道信息
     */
    bool WaitForMagnetic(bool& hasMag, ByteBuilder* tr1 = NULL, ByteBuilder* tr2 = NULL, ByteBuilder* tr3 = NULL)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        ByteBuilder cmd(4);
        DevCommand::FromAscii("1B 50", cmd);
        ASSERT_FuncErr(_pDev->Write(cmd), DeviceError::SendErr);
        ByteBuilder recv(64);
        while(recv.GetLength() < 2)
        {
            ASSERT_FuncErr(_pDev->Read(recv), DeviceError::RecvErr);
        }
        ASSERT_FuncErr(recv[0] == 0x1B, DeviceError::RecvFormatErr);
        hasMag = (recv[1] == '1');
        if(!hasMag)
            return _logRetValue(true);
        // 继续接收磁条数据
        ASSERT_FuncErr(MagneticDevAdapter::RecvByFormat(_pDev, recv), DeviceError::RecvErr);
        MagneticDevAdapter::UnpackRecvCmd(recv.SubArray(2), tr1, tr2, tr3);

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