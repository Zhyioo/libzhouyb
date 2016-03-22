//========================================================= 
/**@file pyTools.h
 * @brief ����*.pyd�ļ���Ҫ����ؽӿ�
 * 
 * @date 2016-03-19   21:28:26
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "pyInvoker.h"
//--------------------------------------------------------- 
/// class python�ർ��
#define python_class(className, pyName) \
    boost::python::class_<className>(pyName)
//--------------------------------------------------------- 
/// interface python�ӿڵ���
#define python_interface(className, interfaceName) \
    boost::python::class_<className>(#interfaceName) \
    .def("__Invoke", &className::Invoke, boost::python::arg("instance")) \
    .def##interfaceName(className)
//--------------------------------------------------------- 
/// classʵ��interface python�ӿڵ���
#define def_implements(className, interfaceName) \
    def("get"#interfaceName, &className::getInstance<interfaceName>)
//--------------------------------------------------------- 
/// IBaseDevice python�ӿڵ���
#define def_IBaseDevice(className) \
    def("Open", &className::Open, boost::python::arg("sArg")="") \
    .def("IsOpen", &className::IsOpen) \
    .def("Close", &className::Close) 
//--------------------------------------------------------- 
/// IInteractiveTrans python�ӿڵ���
#define def_IInteractiveTrans(className) \
    def("Write", &className::Write, boost::python::arg("data")) \
    .def("Read", &className::Read) \
    .add_property("Response", &className::getResponse) 
//--------------------------------------------------------- 
/// IICCardDevice python�ӿڵ���
#define def_IICCardDevice(className) \
    def("PowerOn", &className::PowerOn, boost::python::arg("reader")="") \
    .def("Apdu", &className::Apdu, boost::python::arg("sApdu")) \
    .def("PowerOff", &className::PowerOff) \
    .add_property("Atr", &className::getATR) \
    .add_property("Response", &className::getResponse) \
    .add_property("Reader", &className::getReader)
//--------------------------------------------------------- 
//========================================================= 