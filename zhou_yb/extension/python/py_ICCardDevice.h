//========================================================= 
/**@file py_ICCardDevice.h 
 * @brief python下的智能卡读卡器 
 * 
 * @date 2014-05-05   20:18:45 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../include/Base.h"
#include "../../include/Container.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace python {
//--------------------------------------------------------- 
/// Python下IC卡设备必须支持的自定义接口 
#define ICCARD_DEVICE_PYTHON(className) \
    boost::python::class_<className>(#className) \
        .def("PowerOn", &className::PowerOn, boost::python::arg("reader")="") \
        .def("Apdu", &className::Apdu, boost::python::arg("sApdu")) \
        .def("PowerOff", &className::PowerOff) \
        .add_property("Atr", &className::getATR) \
        .add_property("Response", &className::getResponse) \
        .add_property("Reader", &className::getReader) 
//--------------------------------------------------------- 
/// Python下IC卡接口设备
template<class baseIcDev>
class py_ICCardDevice
{
protected:
    shared_base_obj<baseIcDev> _dev;
    ByteBuilder _atr;
    ByteBuilder _rApdu;
    ByteBuilder _tmpBuff;

    string _reader;
public:
    //----------------------------------------------------- 
    /**
     * @brief 给指定名称的设备上电 
     * @attention 只能重载PowerOn 内部负责处理字符串参数 
     */ 
    virtual bool PowerOn(const char* reader)
    {
        _reader = _strput(reader);
        _atr.Clear();
        return _dev.obj().PowerOn(reader, &_atr);
    }

    /// 下电 
    bool PowerOff()
    {
        _dev.obj().PowerOff();
        return true;
    }
    /// 交换APDU 
    bool Apdu(const char* sApdu)
    {
        _tmpBuff.Clear();
        DevCommand::FromAscii(sApdu, _tmpBuff);

        _rApdu.Clear();
        return _dev.obj().Apdu(_tmpBuff, _rApdu);
    }

    /// 接收APDU相应 
    string getResponse()
    {
        if(_rApdu.IsEmpty())
            return "";
        _tmpBuff.Clear();
        ByteConvert::ToAscii(_rApdu, _tmpBuff);
        return _tmpBuff.GetString();
    }
    /// 获取当前读卡器名称 
    string getReader()
    {
        return _reader;
    }
    /// 获取上电时候返回的ATR 
    string getATR()
    {
        if(_atr.IsEmpty())
            return "";
        _tmpBuff.Clear();
        ByteConvert::ToAscii(_atr, _tmpBuff);

        return _tmpBuff.GetString();
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace python
} // namespace extension 
} // namespace zhou_yb
//========================================================= 