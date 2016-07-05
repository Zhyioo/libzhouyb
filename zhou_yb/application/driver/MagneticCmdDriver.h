//========================================================= 
/**@file MagneticCmdDriver.h
 * @brief 磁条命令驱动
 * 
 * @date 2016-06-12   17:08:47
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_MAGNETICCMDDRIVER_H_
#define _LIBZHOUYB_MAGNETICCMDDRIVER_H_
//--------------------------------------------------------- 
#include "CommonCmdDriver.h"

#include "../../device/magnetic/MagneticDevAdapter.h"
using zhou_yb::device::magnetic::MagneticDevAdapter;

#include "../../extension/ability/TimerDevAdapter.h"
using zhou_yb::extension::ability::TimerDevAdapter;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace driver {
//--------------------------------------------------------- 
/// 磁条命令驱动
class MagneticCmdDriver :
    public DevAdapterBehavior<IInteractiveTrans>,
    public CommandCollection,
    public InterruptBehavior,
    public RefObject
{
protected:
    LastErrInvoker _objErr;
    LastErrExtractor _lastErr;
    LoggerInvoker _logInvoker;
    InterruptInvoker _interruptInvoker;
    DevAdapterInvoker<IInteractiveTrans> _adapter;

    TimerDevAdapter _timerAdapter;
    MagneticDevAdapter _magAdapter;
public:
    MagneticCmdDriver()
    {
        _objErr.Invoke(_lasterr, _errinfo);

        _lastErr.IsFormatMSG = false;
        _lastErr.IsLayerMSG = true;
        _lastErr.Select(_timerAdapter, "Reader");
        _lastErr.Select(_magAdapter, "MagAdapter");
        _lastErr.Select(_objErr);

        _interruptInvoker.select(_timerAdapter);
        select_helper<LoggerInvoker::SelecterType>::select(_logInvoker),
            _timerAdapter, _magAdapter;
        select_helper<DevAdapterInvoker<IInteractiveTrans>::SelecterType>::select(_adapter),
            _timerAdapter;

        _timerAdapter.IsTryWrite = false;
        _magAdapter.SelectDevice(_timerAdapter);

        _Registe("MagSetting", (*this), &MagneticCmdDriver::MagSetting);
        _Registe("MagRead", (*this), &MagneticCmdDriver::MagRead);
        _Registe("MagWrite", (*this), &MagneticCmdDriver::MagWrite);

        Ref<Command> clearCmd = Command::Make((*this), &MagneticCmdDriver::MagClear);
        _Registe("MagClear", clearCmd);
        _Registe("WaitMagCard", (*this), &MagneticCmdDriver::WaitMagCard)->Bind(clearCmd, ComplexCommand::RunOnFinal);
    }
    LC_CMD_ADAPTER(IInteractiveTrans, _adapter);
    LC_CMD_LOGGER(_logInvoker);
    LC_CMD_LASTERR(_lastErr);
    LC_CMD_INTERRUPT(_interruptInvoker);
    /**
     * @brief 设置磁条参数
     * @date 2016-06-12 17:17
     * 
     * @param [in] 
     */
    LC_CMD_METHOD(MagSetting)
    {
        return true;
    }
    /**
     * @brief 读磁条
     * @date 2016-06-12 17:13
     * 
     * @param [in] Timeout : uint 等待刷卡的超时时间(ms)
     * @param [in] Mode : string 磁条模式("23", "2", "123")
     * - 子参数:
     *  - 1:需要读一磁道
     *  - 2:需要读二磁道
     *  - 3:需要读三磁道
     * .
     * @param [in] IsCheck : bool 是否硬件校验
     * 
     * @retval Tr1 : string 一磁道数据
     * @retval Tr2 : string 二磁道数据
     * @retval Tr3 : string 三磁道数据
     */
    LC_CMD_METHOD(MagRead)
    {
        uint timeoutMs = arg["Timeout"].To<uint>(DEV_WAIT_TIMEOUT);
        string mode = arg["Mode"].To<string>("2");
        bool isCheck = arg["IsCheck"].To<bool>(true);
        
        _timerAdapter.SetWaitTimeout(timeoutMs);
        _magAdapter.IsCheckRetStatus = isCheck;

        ByteBuilder tr1(128);
        ByteBuilder tr2(128);
        ByteBuilder tr3(128);
        if(MagneticDevAdapterHelper::ReadMagneticCard(_magAdapter, mode.c_str(), &tr1, &tr2, &tr3) != DevHelper::EnumSUCCESS)
            return false;

        rlt.PushValue("Tr1", tr1.GetString());
        rlt.PushValue("Tr2", tr2.GetString());
        rlt.PushValue("Tr3", tr3.GetString());
        return true;
    }
    /**
     * @brief 写磁条
     * @date 2016-06-12 17:15
     * 
     * @param [in] Timeout : uint 等待刷卡的超时时间(ms)
     * @param [in] Mode : string 磁条模式("23", "2", "123")
     * - 子参数:
     *  - 1:需要写一磁道
     *  - 2:需要写二磁道
     *  - 3:需要写三磁道
     * .
     * @param [in] Tr1 : string 需要写入的一磁道数据
     * @param [in] Tr2 : string 需要写入的二磁道数据
     * @param [in] Tr3 : string 需要写入的三磁道数据
     * @param [in] IsCheck : bool 是否硬件校验
     */
    LC_CMD_METHOD(MagWrite)
    {
        uint timeoutMs = arg["Timeout"].To<uint>(DEV_WAIT_TIMEOUT);
        string mode;
        if(!arg["Mode"].Get<string>(mode))
        {
            _logErr(DeviceError::ArgFormatErr);
            return false;
        }
        bool isCheck = arg["IsCheck"].To<bool>(true);
        string tr1 = arg["Tr1"].To<string>();
        string tr2 = arg["Tr2"].To<string>();
        string tr3 = arg["Tr3"].To<string>();

        _timerAdapter.SetWaitTimeout(timeoutMs);
        _magAdapter.IsCheckRetStatus = isCheck;
        if(MagneticDevAdapterHelper::WriteMagneticCard(
            _magAdapter, mode.c_str(), tr1.c_str(), tr2.c_str(), tr3.c_str()) != DevHelper::EnumSUCCESS)
        {
            return false;
        }
        return true;
    }
    /**
     * @brief 轮询等待读磁条
     * @date 2016-06-12 17:18
     *
     * @param [in] Timeout : uint 等待刷卡的超时时间(ms)
     * 
     * @retval Number : string 卡号(截取二磁道数据中第一个等号前的数据)
     * @retval Tr1 : string 一磁道数据
     * @retval Tr2 : string 二磁道数据
     * @retval Tr3 : string 三磁道数据
     */
    LC_CMD_METHOD(WaitMagCard)
    {
        uint timeoutMs = arg["Timeout"].To<uint>(DEV_WAIT_TIMEOUT);
        if(!_pDev->Write(DevCommand::FromAscii("1B 5D")))
            return false;
        byte magCmd[2] = { 0x1B, 'j' };
        ByteBuilder buff(256);
        Timer timer;
        bool bRead = false;
        while(timer.Elapsed() < timeoutMs)
        {
            if(InterruptBehavior::Implement(*this))
            {
                _logErr(DeviceError::OperatorInterruptErr);
                return false;
            }

            if(!_pDev->Write(ByteArray(magCmd, 2)))
                return false;
            buff.Clear();
            while(buff.GetLength() < 2)
            {
                if(!_pDev->Read(buff))
                    return false;
            }
            // 有磁条数据
            if(buff[0] == 0x1B)
            {
                if(buff[1] == 0x31)
                {
                    buff.RemoveFront(2);
                    bRead = true;
                    break;
                }
                else if(buff[1] == 0x73)
                {
                    bRead = true;
                    break;
                }
            }
        }
        ASSERT_FuncErrInfo(bRead, DeviceError::WaitTimeOutErr, "等待刷磁超时");
        bRead = MagneticDevAdapter::RecvByFormat(_pDev, buff);
        ASSERT_FuncErr(bRead, DeviceError::RecvFormatErr);

        ByteBuilder tr1(128);
        ByteBuilder tr2(128);
        ByteBuilder tr3(128);

        MagneticDevAdapter::MagneticMode mode = MagneticDevAdapter::UnpackRecvCmd(buff, &tr1, &tr2, &tr3);
        if(mode == MagneticDevAdapter::UnKnownMode)
        {
            _logErr(DeviceError::RecvFormatErr, "磁条数据格式不正确");
            return false;
        }
        size_t index = StringConvert::IndexOfAny(tr2, "=\'");
        char CardNumberID[] = "Number";
        // 没有找到分隔符
        if(index == SIZE_EOF)
        {
            rlt.PushValue(CardNumberID, tr2.GetString());
        }
        else
        {
            ByteBuilder cardNumber(20);
            cardNumber = tr2.SubArray(0, index);
            rlt.PushValue(CardNumberID, cardNumber.GetString());
        }
        rlt.PushValue("Tr1", tr1.GetString());
        rlt.PushValue("Tr2", tr2.GetString());
        rlt.PushValue("Tr3", tr3.GetString());

        return true;
    }
    /// 清除磁条刷卡状态
    LC_CMD_METHOD(MagClear)
    {
        return _pDev->Write(DevCommand::FromAscii("1B 30"));
    }
};
//--------------------------------------------------------- 
} // nemespace driver
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_MAGNETICCMDDRIVER_H_
//========================================================= 