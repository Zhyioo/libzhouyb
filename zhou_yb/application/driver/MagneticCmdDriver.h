//========================================================= 
/**@file MagneticCmdDriver.h
 * @brief ������������
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
/// ������������
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
        _lastErr.Select(_magAdapter, "Adapter");
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
        _Registe("WaitMagCard", (*this), &MagneticCmdDriver::WaitMagCard);
    }
    LC_CMD_ADAPTER(IInteractiveTrans, _adapter);
    LC_CMD_LOGGER(_logInvoker);
    LC_CMD_LASTERR(_lastErr);
    LC_CMD_INTERRUPT(_interruptInvoker);
    /**
     * @brief ���ô�������
     * @date 2016-06-12 17:17
     * 
     * @param [in] 
     */
    LC_CMD_METHOD(MagSetting)
    {
        return true;
    }
    /**
     * @brief ������
     * @date 2016-06-12 17:13
     * 
     * @param [in] Timeout : uint �ȴ�ˢ���ĳ�ʱʱ��(ms)
     * @param [in] Mode : string ����ģʽ("23", "2", "123")
     * - �Ӳ���:
     *  - 1:��Ҫ��һ�ŵ�
     *  - 2:��Ҫ�����ŵ�
     *  - 3:��Ҫ�����ŵ�
     * .
     * @param [in] IsCheck : bool �Ƿ�Ӳ��У��
     * 
     * @retval Tr1 : string һ�ŵ�����
     * @retval Tr2 : string ���ŵ�����
     * @retval Tr3 : string ���ŵ�����
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
     * @brief д����
     * @date 2016-06-12 17:15
     * 
     * @param [in] Timeout : uint �ȴ�ˢ���ĳ�ʱʱ��(ms)
     * @param [in] Mode : string ����ģʽ("23", "2", "123")
     * - �Ӳ���:
     *  - 1:��Ҫдһ�ŵ�
     *  - 2:��Ҫд���ŵ�
     *  - 3:��Ҫд���ŵ�
     * .
     * @param [in] Tr1 : string ��Ҫд���һ�ŵ�����
     * @param [in] Tr2 : string ��Ҫд��Ķ��ŵ�����
     * @param [in] Tr3 : string ��Ҫд������ŵ�����
     * @param [in] IsCheck : bool �Ƿ�Ӳ��У��
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
     * @brief ��ѯ�ȴ�������
     * @date 2016-06-12 17:18
     *
     * @param [in] Timeout : uint �ȴ�ˢ���ĳ�ʱʱ��(ms)
     * 
     * @retval Tr1 : string һ�ŵ�����
     * @retval Tr2 : string ���ŵ�����
     * @retval Tr3 : string ���ŵ�����
     */
    LC_CMD_METHOD(WaitMagCard)
    {
        uint timeoutMs = arg["Timeout"].To<uint>(DEV_WAIT_TIMEOUT);
        return true;
    }
};
//--------------------------------------------------------- 
} // nemespace driver
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_MAGNETICCMDDRIVER_H_
//========================================================= 