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
#include <boost/shared_ptr.hpp>

#include "../../include/Base.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
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
        return ctype_cast(int)&(_instance.obj());
    }
    /// ��ȡ�ӿھ��
    template<class TInterface>
    int getInstance()
    {
        return ctype_cast(int)&(_instance.obj());
    }
};
//--------------------------------------------------------- 
/* �����ӿ����� */
/// Invoker
template<class T>
class py_Invoker
{
protected:
    boost::shared_ptr<WeakRef<T> > _ref;
    Ref<T> _invoker;
public:
    py_Invoker(T* p = NULL)
    {
        if(p != NULL)
        {
            _ref = boost::shared_ptr<WeakRef<T> >(new WeakRef<T>(*p));
            _invoker = (*_ref);
        }
    }
    /// �ص�
    virtual bool Invoke(boost::python::object pyObj, const char* funcName)
    {
        int handle = NULL;
        _invoker = Ref<T>();
        try
        {
            handle = boost::python::call_method<int>(pyObj.ptr(), funcName);
        }
        catch(...)
        {            
            return false;
        }
        T* ptr = ctype_cast(T*)handle;
        if(ptr != NULL)
        {
            _ref = boost::shared_ptr<WeakRef<T> >(new WeakRef<T>(*ptr));
            _invoker = (*_ref);
        }
        
        return !(_invoker.IsNull());
    }
};
//--------------------------------------------------------- 
/// IAdapter
template<class TAdapter, class TInterface = TAdapter::InterfaceType>
class py_Adapter : public py_Instance<TAdapter>, public py_Invoker<TInterface>
{
public:
    /// �ص�
    virtual bool Invoke(boost::python::object pyObj, const char* funcName)
    {
        bool bInvoke = py_Invoker<TInterface>::Invoke(pyObj, funcName);
        if(bInvoke)
        {
            py_Instance<TAdapter>::_instance.obj().SelectDevice(py_Invoker<TInterface>::_invoker);
            bInvoke = py_Instance<TAdapter>::_instance.obj().IsValid();
            if(!bInvoke)
            {
                py_Invoker<TInterface>::_invoker = Ref<TInterface>();
            }
        }
        return bInvoke;
    }
    /// ���ص�ǰ�ص�����Ƿ���Ч
    bool IsValid()
    {
        return !(_invoker.IsNull());
    }
    /// �ͷŻص����
    void Dispose()
    {
        _invoker = Ref<TInterface>();
        py_Instance<TAdapter>::_instance.obj().ReleaseDevice();
    }
};
//--------------------------------------------------------- 
/// IBaseDevice
class py_IBaseDevice : public py_Invoker<IBaseDevice>
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
class py_IInteractiveTrans : public py_Invoker<IInteractiveTrans>
{
public:
    /// ��������
    bool Write(const char* data)
    {
        return _invoker->Write(DevCommand::FromAscii(data));
    }
    /// ��������
    boost::python::list Read()
    {
        boost::python::list ret;
        ByteBuilder recv(8);

        ret.append(_invoker->Read(recv));

        ByteBuilder tmp(8);
        ByteConvert::ToAscii(recv, tmp);
        ret.append(tmp.GetString());

        return ret;
    }
};
//--------------------------------------------------------- 
/// ITransceiveTrans
class py_ITransceiveTrans : public py_Invoker<ITransceiveTrans>
{
public:
    boost::python::list TransCommand(const char* sCmd)
    {
        boost::python::list ret;
        ByteBuilder send(8);
        ByteBuilder recv(8);

        DevCommand::FromAscii(sCmd, send);
        ret.append(_invoker->TransCommand(send, recv));

        ByteBuilder tmp(8);
        ByteConvert::ToAscii(recv, tmp);
        ret.append(tmp.GetString());

        return ret;
    }
};
//--------------------------------------------------------- 
/// IICCardDevice
class py_IICCardDevice : public py_Invoker<IICCardDevice>
{
public:
    //----------------------------------------------------- 
    /**
     * @brief ��ָ�����Ƶ��豸�ϵ�
     * @attention ֻ������PowerOn �ڲ��������ַ�������
     */
    boost::python::list PowerOn(const char* reader)
    {
        boost::python::list ret;
        ByteBuilder atr(8);

        ret.append(_invoker->PowerOn(reader, &atr));
            
        ByteBuilder tmp(8);
        ByteConvert::ToAscii(atr, tmp);
        ret.append(tmp.GetString());

        return ret;
    }
    /// ����APDU 
    boost::python::list Apdu(const char* sApdu)
    {
        boost::python::list ret;
        
        ByteBuilder rApdu(8);
        ret.append(_invoker->Apdu(DevCommand::FromAscii(sApdu), rApdu));

        ByteBuilder tmp(8);
        ByteConvert::ToAscii(rApdu, tmp);
        ret.append(tmp.GetString());

        return ret;
    }
    /// �µ� 
    bool PowerOff()
    {
        return _invoker->PowerOff();
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace python
} // namespace extension
} // namespace zhou_yb
//========================================================= 