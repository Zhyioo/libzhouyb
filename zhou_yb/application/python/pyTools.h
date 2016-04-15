//========================================================= 
/**@file pyTools.h
 * @brief ����*.pyd�ļ���Ҫ����ؽӿ�
 * 
 * @date 2016-03-19   21:28:26
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_PYTOOLS_H_
#define _LIBZHOUYB_PYTOOLS_H_
//--------------------------------------------------------- 
/// class python�ർ��
#define python_class(className, pyName) \
    boost::python::class_<className>(pyName) \
    .def("getInstance", &className::getInstance)
//--------------------------------------------------------- 
/// interface python�ӿڵ���
#define python_interface(className, interfaceName) \
    boost::python::class_<className>(#interfaceName, boost::python::no_init) \
    .def_##interfaceName(className)
//--------------------------------------------------------- 
/// adapter python�ӿڵ���
#define python_adapter(className, interfaceName, pyName) \
    python_class(className, pyName) \
    .def("Invoke", &className::Invoke, boost::python::arg("fId")="get"#interfaceName) \
    .def("IsValid", &className::IsValid) \
    .def("Dispose", &className::Dispose)
//--------------------------------------------------------- 
/// classʵ��interface python�ӿڵ���
#define def_implements(className, interfaceName) \
    def("get"#interfaceName, &className::getInstance<interfaceName>)
//--------------------------------------------------------- 
/// classʵ�ֲ��ṩinterface python�ӿڵ���
#define def_extends(className, interfaceName) \
    def_implements(className, interfaceName) \
    .def_##interfaceName(className)
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
    .def("Read", &className::Read)
//--------------------------------------------------------- 
/// ITransceiveTrans python�ӿڵ���
#define def_ITransceiveTrans(className) \
    def("TransCommand", &className::TransCommand, boost::python::arg("sCmd"))
//--------------------------------------------------------- 
/// IICCardDevice python�ӿڵ���
#define def_IICCardDevice(className) \
    def("PowerOn", &className::PowerOn, boost::python::arg("reader")="") \
    .def("Apdu", &className::Apdu, boost::python::arg("sApdu")) \
    .def("PowerOff", &className::PowerOff)
//--------------------------------------------------------- 
/// IInterrupter python�ӿڵ���
#define def_IInterrupter(className) \
    def("InterruptionPoint", &className::InterruptionPoint) \
    .def("Interrupt", &className::Interrupt) \
    .def("Reset", &className::Reset)
//--------------------------------------------------------- 
/// ITimeoutBehavior python�ӿڵ���
#define def_ITimeoutBehavior(className) \
    def("SetWaitTimeout", &className::SetWaitTimeout) \
    .def("SetOperatorInterval", &className::SetOperatorInterval)
//--------------------------------------------------------- 
/// ILastErrBehavior python�ӿڵ���
#define def_ILastErrBehavior(className) \
    def("GetLastErr", &className::GetLastErr) \
    .def("GetErrMessage", &className::GetErrMessage)
//--------------------------------------------------------- 
/// InterruptBehavior python�ӿڵ���
#define def_InterruptBehavior(className) \
    add_property("Interrupter", &className::GetHandle, &className::SetHandle)
//--------------------------------------------------------- 
/// ILoggerBehavior python�ӿڵ���
#define def_ILoggerBehavior(className)
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_PYTOOLS_H_
//========================================================= 