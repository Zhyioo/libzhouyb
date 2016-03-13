//========================================================= 
/**@file DllICCardDeviceAdapter.h
 * @brief DLL IC卡驱动封装 
 * 
 * @date 2012-12-03   22:44:30
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../include/Base.h"
#include "../../extension/ability/logger/StringCallBackLogger.h"
using zhou_yb::extension::ability::StringCallBackLogger;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace iccard {
//--------------------------------------------------------- 
/// DLL IC卡驱动封装 
class DllICCardDeviceAdapter : public IBaseDevice, public IICCardDevice, public LoggerBehavior, public RefObject
{
public:
    //----------------------------------------------------- 
    typedef int(__stdcall *DLL_FUNCTION(PowerOn))(const char* sArg, byte* atr, size_t* atrLen);
    typedef int(__stdcall *DLL_FUNCTION(PowerOff))();
    typedef int(__stdcall *DLL_FUNCTION(HasPowerOn))();
    typedef int(__stdcall *DLL_FUNCTION(Apdu))(const byte* sApdu, size_t sLen, byte* rApdu, size_t* rLen);
    typedef void(__stdcall *DLL_FUNCTION(SetLogger))(StringCallBackLogger::logOutputCallBack callback);
    //----------------------------------------------------- 
protected:
    //----------------------------------------------------- 
    string _path;
    HMODULE _hDll;
    fpPowerOn _fpPowerOn;
    fpPowerOff _fpPowerOff;
    fpHasPowerOn _fpHasPowerOn;
    fpApdu _fpApdu;
    fpSetLogger _fpSetLogger;
    //----------------------------------------------------- 
public:
    DllICCardDeviceAdapter() : _hDll(NULL) { Close(); }
    virtual ~DllICCardDeviceAdapter()
    {
        if(IsOpen())
        {
            PowerOff();
            Close();
        }
    }
    //----------------------------------------------------- 
    //@{
    /**@name 
     * @brief IBaseDeivce接口实现 
     */ 
    /// 打开DLL
    virtual bool Open(const char* sArg = NULL)
    {
        if(!_is_empty_or_null(sArg))
            _path = sArg;
        if(_path.length() < 1)
            return false;

        CharConverter cvt;
        _hDll = LoadLibrary(cvt.to_char_t(_path.c_str()));
        if(_hDll == NULL)
            return false;

        _fpApdu = (fpApdu)GetProcAddress(_hDll, "Apdu");
        _fpHasPowerOn = (fpHasPowerOn)GetProcAddress(_hDll, "HasPowerOn");
        _fpPowerOff = (fpPowerOff)GetProcAddress(_hDll, "PowerOff");
        _fpPowerOn = (fpPowerOn)GetProcAddress(_hDll, "PowerOn");

        _fpSetLogger = (fpSetLogger)GetProcAddress(_hDll, "SetLogger");

        if(_fpApdu == NULL || _fpHasPowerOn == NULL || _fpPowerOff == NULL || _fpPowerOn == NULL)
        {
            Close();
            return false;
        }

        return true;
    }
    /// 是否打开 
    virtual bool IsOpen()
    {
        return _hDll != NULL;
    }
    /// 关闭 
    virtual void Close()
    {
        if(_hDll != NULL)
        {
            SetLogger(NULL);
            FreeLibrary(_hDll);
            _hDll = NULL;
        }
        _fpPowerOn = NULL;
        _fpPowerOff = NULL;
        _fpHasPowerOn = NULL;
        _fpApdu = NULL;
        _fpSetLogger = NULL;
    }
    //@}
    //----------------------------------------------------- 
    //@{
    /**@name 
     * @brief IICCardDevice接口实现 
     */ 
    /// 上电
    virtual bool PowerOn(const char* readerName, ByteBuilder* pAtr = NULL)
    {
        LOG_FUNC_NAME();
        LOGGER(_log<<"readerName:<"<<readerName<<">\n");

        byte atr[64] = {0};
        size_t atrLen = 0;

        if(_fpPowerOn(readerName, atr, &atrLen) != DeviceError::Success)
            return _logRetValue(false);

        if(pAtr != NULL)
        {
            pAtr->Append(ByteArray(atr, atrLen));
        }

        LOGGER(
        _log.WriteLine("ATR:");
        _log.WriteHex(ByteArray(atr, atrLen));
        _log.WriteLine());

        return _logRetValue(true);
    }
    /// 下电
    virtual bool PowerOff()
    {
        LOG_FUNC_NAME();
        return _logRetValue(_fpPowerOff() == DeviceError::Success);
    }
    /// 是否已经上电
    virtual bool HasPowerOn() const
    {
        return (_fpHasPowerOn() != DeviceError::Success);
    }
    /// 交换APDU(提供默认实现) 
    virtual bool Apdu(const ByteArray& sendBcd, ByteBuilder& recvBcd)
    {
        LOG_FUNC_NAME();
        LOGGER(
        _log.WriteLine("Send Apdu:");
        _log.WriteLine(sendBcd));

        bool bRet = ICCardDevice::AutoApdu(*this, sendBcd, recvBcd);

        LOGGER(
        if(bRet)
        {
            _log.WriteLine("Recv Apdu:");
            _log.WriteLine(recvBcd);
        });

        return _logRetValue(bRet);
    }
    // 只提供交互单独的一条指令 
    virtual bool TransCommand(const ByteArray& send, ByteBuilder& recv)
    {
        LOG_FUNC_NAME();

        byte rApdu[1024] = {0};
        size_t rLen = 0;

        LOGGER(
        _log.WriteLine("Send Command:");
        _log.WriteLine(send));

        if(_fpApdu((byte*)send.GetBuffer(), send.GetLength(), rApdu, &rLen) != DeviceError::Success)
            return _logRetValue(false);

        recv.Append(ByteArray(rApdu, rLen));

        LOGGER(
        _log.WriteLine("Recv Command:");
        _log.WriteLine(recv));

        return _logRetValue(true);
    }
    //@}
    void SetLogger(StringCallBackLogger::logOutputCallBack callback)
    {
        if(_fpSetLogger != NULL)
        {
            _fpSetLogger(callback);
        }
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace iccard
} // namespace application
} // namespace zhou_yb
//========================================================= 