//========================================================= 
/**@file ICCardAppAdapter.h 
 * @brief IC卡应用适配器基本功能封装 
 * 
 * @date 2012-08-30   15:18:41 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_ICCARDAPPADAPTER_H_
#define _LIBZHOUYB_ICCARDAPPADAPTER_H_
//--------------------------------------------------------- 
#include "../../../include/Base.h"
#include "ICCardLibrary.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace device {
namespace iccard {
namespace base {
//--------------------------------------------------------- 
/**
 * @brief IC卡适配器,负责处理Apdu交互,并提供基本的SW状态记录支持 
 */
class ICCardAppAdapter : public DevAdapterBehavior<ITransceiveTrans>, public RefObject
{
protected:
    //----------------------------------------------------- 
    /// 上次交换APDU的SW状态码 
    ushort _lastSW;
    //-----------------------------------------------------
    /// 初始化数据成员 
    inline void _init()
    {
        _lastSW = ICCardLibrary::UnValidSW; 
    }
    /**
     * @brief 交换APDU，判断SW是否成功，并且把状态码去除 
     * @param [in] send 需要发送的APDU指令 
     * @param [out] recv 接收到的APDU数据  
     * @param [in] pIcDev [default:NULL] 需要操作的IC卡读卡器(为NULL表示操作当前选择的设备) 
     */ 
    inline bool _apdu(const ByteArray& send, ByteBuilder& recv, ITransceiveTrans* pIcDev = NULL)
    {
        return ICCardLibrary::IsSuccessSW(_apdu_s(send, recv, pIcDev));
    }
    /**
     * @brief 交换APDU，去除状态码，返回SW值，发送失败则返回 UnValidSW 
     * @param [in] send 需要发送的APDU指令 
     * @param [out] recv 接收到的APDU数据  
     * @param [in] pIcDev [default:NULL] 需要操作的IC卡读卡器(为NULL表示操作当前选择的设备) 
     */ 
    ushort _apdu_s(const ByteArray& send, ByteBuilder& recv, ITransceiveTrans* pIcDev = NULL)
    {
        recv.Clear();

        _lastSW = ICCardLibrary::UnValidSW;
        if(pIcDev == NULL)
            pIcDev = &(*_pDev);
        /* 交换APDU */
        if(ICCardDevice::AutoApdu(*pIcDev, send, recv) == false)
        {
            _logErr(DeviceError::TransceiveErr);
            return _lastSW;
        }

        _lastSW = ICCardLibrary::GetSW(recv);
        // 删除状态字
        ICCardLibrary::RemoveSW(recv);

        if(!ICCardLibrary::IsSuccessSW(_lastSW))
        {
            string swStr = _hex(_lastSW);
            _logErr(DeviceError::OperatorStatusErr, swStr.c_str());
            LOGGER(_log<<"SW:<"<<swStr<<"> "<<ICCardLibrary::GetSW_MSG(_lastSW)<<"\n");
        }

        return _lastSW;
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    ICCardAppAdapter() : DevAdapterBehavior<ITransceiveTrans>() { _init(); }
    //----------------------------------------------------- 
    /// 获取上次SW值 
    inline ushort GetSW() const
    {
        return _lastSW;
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace base
} // namesapce iccard 
} // namespace device 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_ICCARDAPPADAPTER_H_
//========================================================= 