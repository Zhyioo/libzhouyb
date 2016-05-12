//========================================================= 
/**@file base.h
 * @brief 
 * 
 * @date 2016-04-23   17:57:15
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_BASE_H_
#define _LIBZHOUYB_BASE_H_
//--------------------------------------------------------- 
#include "../pyTools.h"
#include "../pyInvoker.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace python {
//--------------------------------------------------------- 
/// BoolInterrupter
class py_BoolInterrupter :
    public py_Instance<BoolInterrupter>,
    public py_IInterrupter
{
public:
    static void PyExport()
    {
        python_class(py_BoolInterrupter, "BoolInterrupter")
            .def_extends(py_BoolInterrupter, IInterrupter);
    }

    py_BoolInterrupter()
    {
        py_IInterrupter::_invoker = _instance.obj();
    }
};
//--------------------------------------------------------- 
/// py基础单元
struct pyBaseUnit
{
    /// 导出接口声明
    static void PyExport()
    {
        python_interface(py_IBaseDevice, IBaseDevice);
        python_interface(py_IInteractiveTrans, IInteractiveTrans);
        python_interface(py_ITransceiveTrans, ITransceiveTrans);
        python_interface(py_IICCardDevice, IICCardDevice);

        python_interface(py_IInterrupter, IInterrupter);
        python_interface(py_ITimeoutBehavior, ITimeoutBehavior);
        python_interface(py_ILastErrBehavior, ILastErrBehavior);
        python_interface(py_InterruptBehavior, InterruptBehavior);
        python_interface(py_ILoggerBehavior, ILoggerBehavior);
    }
};
//--------------------------------------------------------- 
} // namespace python
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_BASE_H_
//========================================================= 