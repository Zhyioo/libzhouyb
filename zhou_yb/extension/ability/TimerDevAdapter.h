//========================================================= 
/**@file TimerDevAdapter.h
 * @brief 
 * 
 * @date 2016-06-12   17:39:06
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_TIMERDEVADAPTER_H_
#define _LIBZHOUYB_TIMERDEVADAPTER_H_
//--------------------------------------------------------- 
#include "../../include/Base.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace ability {
//--------------------------------------------------------- 
/// ���г�ʱ��Ϊ���豸
class TimerDevAdapter : 
    public DevAdapterBehavior<IInteractiveTrans>,
    public IInteractiveTrans,
    public TimeoutBehavior,
    public InterruptBehavior,
    public RefObject
{
public:
    TimerDevAdapter() : DevAdapterBehavior()
    {
        IsTryRead = true;
        IsTryWrite = true;
    }
    virtual bool Read(ByteBuilder& data)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        if(!IsTryRead) 
            return _logRetValue(_pDev->Read(data));

        Timer timer;
        while(timer.Elapsed() < _waitTimeout)
        {
            if(InterruptBehavior::Implement(*this))
            {
                _logErr(DeviceError::OperatorInterruptErr);
                return _logRetValue(false);
            }

            if(_pDev->Read(data))
                return _logRetValue(true);
        }

        _logErr(DeviceError::WaitTimeOutErr, "��������");
        return _logRetValue(false);
    }
    virtual bool Write(const ByteArray& data)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        if(!IsTryWrite)
            return _logRetValue(_pDev->Write(data));

        Timer timer;
        while(timer.Elapsed() < _waitTimeout)
        {
            if(InterruptBehavior::Implement(*this))
            {
                _logErr(DeviceError::OperatorInterruptErr);
                return _logRetValue(false);
            }

            if(_pDev->Write(data))
                return _logRetValue(true);
        }

        _logErr(DeviceError::WaitTimeOutErr, "��������");
        return _logRetValue(false);
    }
    /// �Ƿ��ڷ���ʱ���г�ʱ����
    bool IsTryWrite;
    /// �Ƿ��ڽ���ʱ���г�ʱ����
    bool IsTryRead;
};
//--------------------------------------------------------- 
} // namespace ability 
} // namespace extension 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_TIMERDEVADAPTER_H_
//========================================================= 