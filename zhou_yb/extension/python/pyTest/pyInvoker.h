//========================================================= 
/**@file pyInvoker.h
 * @brief 处理C++和python直接接口及其操作回调
 * 
 * @date 2016-03-05   12:05:36
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include <boost/python.hpp>

#include "../../../include/Base.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace python {
//--------------------------------------------------------- 
/// py类对象 
template<class T>
class py_Instance
{
protected:
    shared_obj<T> _instance;
public:
    /// 返回对象句柄
    int getInstance()
    {
        return &(_instance.obj());
    }
    /// 获取接口句柄
    template<class TInterface>
    int getInstance()
    {
        return &(_instance.obj());
    }
};
//--------------------------------------------------------- 
/* 各个接口声明 */
/// Invoker
template<class T>
class py_Invoker
{
protected:
    T* _invoker;
public:
    py_Invoker()
    {
        _invoker = NULL;
    }
    /// 回调
    bool Invoke(int instance)
    {
        _invoker = ctype_cast(T*)instance;
        return IsValid();
    }
    /// 当前回调对象是否有效
    bool IsValid()
    {
        return _invoker != NULL;
    }
};
//--------------------------------------------------------- 
template<class T>
class py_Adapter
{
public:
    void SelectDevice(boost::python::object obj, const char* funcName)
    {
        //int hInstance = 
    }
};
//--------------------------------------------------------- 
/// IBaseDevice
class py_IBaseDeviceInvoker : public py_Invoker<IBaseDevice>
{
public:
    /// 打开
    bool Open(const char* sArg)
    {
        return _invoker->Open(sArg);
    }
    /// 返回是否打开
    bool IsOpen()
    {
        return _invoker->IsOpen();
    }
    /// 关闭
    void Close()
    {
        return _invoker->Close();
    }
};
//--------------------------------------------------------- 
/// IInteractiveTrans
class py_IInteractiveTransInvoker : public py_Invoker<IInteractiveTrans>
{
protected:
    ByteBuilder _recv;
public:
    /// 发送数据
    bool Write(const char* data)
    {
        return _invoker->Write(DevCommand::FromAscii(data));
    }
    /// 接收数据
    bool Read()
    {
        _recv.Clear();

        ByteBuilder tmp(8);
        if(!_invoker->Read(tmp))
            return false;

        ByteConvert::ToAscii(tmp, _recv);
        return true;
    }
    /// 回应信息
    string getResponse()
    {
        return _recv.GetString();
    }
};
//--------------------------------------------------------- 
/// IICCardDevice
class py_IICCardDeviceInvoker : public py_Invoker<IICCardDevice>
{
protected:
    ByteBuilder _atr;
    ByteBuilder _rApdu;

    string _reader;
public:
    //----------------------------------------------------- 
    /**
     * @brief 给指定名称的设备上电
     * @attention 只能重载PowerOn 内部负责处理字符串参数
     */
    bool PowerOn(const char* reader)
    {
        _reader = _strput(reader);
        _atr.Clear();

        ByteBuilder tmp(8);
        if(!_invoker->PowerOn(reader, &tmp))
            return false;

        ByteConvert::ToAscii(tmp, _atr);
        return true;
    }
    /// 下电 
    bool PowerOff()
    {
        _invoker->PowerOff();
        return true;
    }
    /// 交换APDU 
    bool Apdu(const char* sApdu)
    {
        _rApdu.Clear();

        ByteBuilder tmp(8);
        if(!_invoker->Apdu(DevCommand::FromAscii(sApdu), tmp))
            return false;

        ByteConvert::ToAscii(tmp, _rApdu);
        return true;
    }
    /// 接收APDU相应 
    string getResponse()
    {
        return _rApdu.GetString();
    }
    /// 获取当前读卡器名称 
    string getReader()
    {
        return _reader;
    }
    /// 获取上电时候返回的ATR 
    string getATR()
    {
        return _atr.GetString();
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace python
} // namespace extension
} // namespace zhou_yb
//========================================================= 