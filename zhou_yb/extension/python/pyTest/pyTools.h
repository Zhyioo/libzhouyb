//========================================================= 
/**@file pyTools.h
 * @brief 导出*.pyd文件需要的相关接口
 * 
 * @date 2016-03-19   21:28:26
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "pyInvoker.h"
//--------------------------------------------------------- 
/// class python类导出
#define python_class(className, pyName) \
    boost::python::class_<className>(pyName)
//--------------------------------------------------------- 
/// interface python接口导出
#define python_interface(className, interfaceName) \
    boost::python::class_<className>(#interfaceName) \
    .def("__Invoke", &className::Invoke, boost::python::arg("instance")) \
    .def##interfaceName(className)
//--------------------------------------------------------- 
/// class实现interface python接口导出
#define def_implements(className, interfaceName) \
    def("get"#interfaceName, &className::getInstance<interfaceName>)
//--------------------------------------------------------- 
/// IBaseDevice python接口导出
#define def_IBaseDevice(className) \
    def("Open", &className::Open, boost::python::arg("sArg")="") \
    .def("IsOpen", &className::IsOpen) \
    .def("Close", &className::Close) 
//--------------------------------------------------------- 
/// IInteractiveTrans python接口导出
#define def_IInteractiveTrans(className) \
    def("Write", &className::Write, boost::python::arg("data")) \
    .def("Read", &className::Read) \
    .add_property("Response", &className::getResponse) 
//--------------------------------------------------------- 
/// IICCardDevice python接口导出
#define def_IICCardDevice(className) \
    def("PowerOn", &className::PowerOn, boost::python::arg("reader")="") \
    .def("Apdu", &className::Apdu, boost::python::arg("sApdu")) \
    .def("PowerOff", &className::PowerOff) \
    .add_property("Atr", &className::getATR) \
    .add_property("Response", &className::getResponse) \
    .add_property("Reader", &className::getReader)
//--------------------------------------------------------- 
//========================================================= 