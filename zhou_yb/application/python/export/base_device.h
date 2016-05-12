//========================================================= 
/**@file base_device.h
 * @brief 
 * 
 * @date 2016-04-23   17:57:20
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_BASE_DEVICE_H_
#define _LIBZHOUYB_BASE_DEVICE_H_
//--------------------------------------------------------- 
#include "../pyTools.h"
#include "../pyInvoker.h"

#include "../../../include/BaseDevice.h"
#include "../../../include/Extension.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace python {
//--------------------------------------------------------- 
#ifndef NO_INCLUDE_PCSC_SOURCE
/// CCID_Device
class py_CCID_Device :
    public py_Instance<CCID_Device>,
    public py_ITransceiveTrans,
    public py_IICCardDevice
{
public:
    static void PyExport()
    {
        boost::python::def("StateToString", &CCID_Device::StateToString);

        python_class(py_CCID_Device, "CCID_Device")
            .def_extends(py_CCID_Device, ITransceiveTrans)
            .def_extends(py_CCID_Device, IICCardDevice)
            .def("EnumDevice", &py_CCID_Device::EnumDevice)
            .add_property("CommandMode", &py_CCID_Device::GetCommandMode, &py_CCID_Device::SetCommandMode)
            .def("ScardState", &py_CCID_Device::ScardState);

        boost::python::enum_<CCID_Device::CommandMode>("CommandMode")
            .value("EscapeCommand", CCID_Device::EscapeCommand)
            .value("ApduCommand", CCID_Device::ApduCommand);
    }

    py_CCID_Device()
    {
        py_ITransceiveTrans::_invoker = _instance.obj();
        py_IICCardDevice::_invoker = _instance.obj();
    }
    boost::python::list EnumDevice()
    {
        boost::python::list ccidlist;

        list<string> devlist;
        list<string>::iterator itr;
        _instance.obj().EnumDevice(devlist);
        for(itr = devlist.begin();itr != devlist.end(); ++itr)
        {
            ccidlist.append(*itr);
        }

        return ccidlist;
    }
    inline int GetCommandMode()
    {
        return _instance.obj().GetMode();
    }
    inline void SetCommandMode(int mode)
    {
        _instance.obj().SetMode(static_cast<CCID_Device::CommandMode>(mode));
    }
    boost::python::list ScardState(const char* reader = NULL)
    {
        boost::python::list state;
        DWORD eventState = SCARD_STATE_EMPTY;
        DWORD currentState = SCARD_STATE_EMPTY;

        state.append(_instance.obj().SCardReaderState(currentState, eventState, reader));
        state.append(currentState);
        state.append(eventState);

        return state;
    }
};
#endif // NO_INCLUDE_PCSC_SOURCE
//--------------------------------------------------------- 
#ifndef NO_INCLUDE_COM_SOURCE
/// ComDevice
class py_ComDevice :
    public py_Instance<ComDevice>,
    public py_IInteractiveTrans,
    public py_IBaseDevice,
    public py_ITimeoutBehavior,
    public py_InterruptBehavior
{
public:
    static void PyExport()
    {
        python_class(py_ComDevice, "ComDevice")
            .def_extends(py_ComDevice, IInteractiveTrans)
            .def_extends(py_ComDevice, IBaseDevice)
            .def_extends(py_ComDevice, ITimeoutBehavior)
            .def_extends(py_ComDevice, InterruptBehavior)
            .def("EnumDevice", &py_ComDevice::EnumDevice);
    }

    py_ComDevice()
    {
        py_IInteractiveTrans::_invoker = _instance.obj();
        py_IBaseDevice::_invoker = _instance.obj();
        py_ITimeoutBehavior::_invoker = _instance.obj();
        py_InterruptBehavior::_invoker = _instance.obj();
    }
    virtual bool Open(const char* sArg)
    {
        uint baud = CBR_9600;
        byte gate = 0x00;
        uint port = ParamHelper::ParseCOM(sArg, gate, baud);

        if(ComDeviceHelper::OpenDevice<ComDevice>(_instance.obj(), port, baud) != DevHelper::EnumSUCCESS)
            return false;
        return true;
    }
    boost::python::list EnumDevice()
    {
        boost::python::list comlist;

        list<uint> devlist;
        list<uint>::iterator itr;
        _instance.obj().EnumDevice(devlist);
        for(itr = devlist.begin();itr != devlist.end(); ++itr)
        {
            comlist.append(*itr);
        }

        return comlist;
    }
};
#endif // NO_INCLUDE_COM_SOURCE
//--------------------------------------------------------- 
#ifndef NO_INCLUDE_USB_SOURCE

#endif // NO_INCLUDE_USB_SOURCE
//--------------------------------------------------------- 
#ifdef INCLUDE_SOCKET_SOURCE

#endif
//--------------------------------------------------------- 
} // namespace python
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_BASE_DEVICE_H_
//========================================================= 