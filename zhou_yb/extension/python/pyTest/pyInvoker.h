//========================================================= 
/**@file pyInvoker.h
 * @brief ����C++��pythonֱ�ӽӿڼ�������ص�
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
/// py����� 
template<class T>
class py_Instance
{
protected:
    shared_obj<T> _instance;
public:
    /// ���ض�����
    int getInstance()
    {
        return &(_instance.obj());
    }
    /// ��ȡ�ӿھ��
    template<class TInterface>
    int getInstance()
    {
        return &(_instance.obj());
    }
};
//--------------------------------------------------------- 
/* �����ӿ����� */
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
    /// �ص�
    bool Invoke(int instance)
    {
        _invoker = ctype_cast(T*)instance;
        return IsValid();
    }
    /// ��ǰ�ص������Ƿ���Ч
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
    /// ��
    bool Open(const char* sArg)
    {
        return _invoker->Open(sArg);
    }
    /// �����Ƿ��
    bool IsOpen()
    {
        return _invoker->IsOpen();
    }
    /// �ر�
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
    /// ��������
    bool Write(const char* data)
    {
        return _invoker->Write(DevCommand::FromAscii(data));
    }
    /// ��������
    bool Read()
    {
        _recv.Clear();

        ByteBuilder tmp(8);
        if(!_invoker->Read(tmp))
            return false;

        ByteConvert::ToAscii(tmp, _recv);
        return true;
    }
    /// ��Ӧ��Ϣ
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
     * @brief ��ָ�����Ƶ��豸�ϵ�
     * @attention ֻ������PowerOn �ڲ��������ַ�������
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
    /// �µ� 
    bool PowerOff()
    {
        _invoker->PowerOff();
        return true;
    }
    /// ����APDU 
    bool Apdu(const char* sApdu)
    {
        _rApdu.Clear();

        ByteBuilder tmp(8);
        if(!_invoker->Apdu(DevCommand::FromAscii(sApdu), tmp))
            return false;

        ByteConvert::ToAscii(tmp, _rApdu);
        return true;
    }
    /// ����APDU��Ӧ 
    string getResponse()
    {
        return _rApdu.GetString();
    }
    /// ��ȡ��ǰ���������� 
    string getReader()
    {
        return _reader;
    }
    /// ��ȡ�ϵ�ʱ�򷵻ص�ATR 
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