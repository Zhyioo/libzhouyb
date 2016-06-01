//========================================================= 
/**@file cmd_adapter.h
 * @brief 
 * 
 * @date 2016-04-23   17:57:23
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_CMD_ADAPTER_H_
#define _LIBZHOUYB_CMD_ADAPTER_H_
//--------------------------------------------------------- 
#include "../pyTools.h"
#include "../pyInvoker.h"

#include "../../../include/Device.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace python {
//--------------------------------------------------------- 
class py_ComICCardCmdAdapter :
    public py_Adapter<ComICCardCmdAdapter, IInteractiveTrans>,
    public py_IInteractiveTrans
{
public:
    static void PyExport()
    {
        python_adapter(py_ComICCardCmdAdapter, IInteractiveTrans, "ComICCardCmdAdapter")
            .add_property("StatusCode", &py_ComICCardCmdAdapter::GetStatusCode);
    }

    py_ComICCardCmdAdapter()
    {
        py_Adapter::_invoker = _instance.obj();
        py_IInteractiveTrans::_invoker = _instance.obj();
    }

    int GetStatusCode()
    {
        return _instance.obj().GetStatusCode();
    }
};
//--------------------------------------------------------- 
} // namespace python
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_CMD_ADAPTER_H_
//========================================================= 