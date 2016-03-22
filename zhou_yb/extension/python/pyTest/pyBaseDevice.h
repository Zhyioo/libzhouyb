//========================================================= 
/**@file pyBaseDevice.h
 * @brief 
 * 
 * @date 2016-03-18   18:18:17
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "./pyInvoker.h"

#include "../../../include/BaseDevice.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace python {
//--------------------------------------------------------- 
/// pythonÏÂComDeviceµÄ·â×°
class py_ComDevice : public py_Instance<ComDevice>, public py_IBaseDeviceInvoker
{
public:
    py_ComDevice()
    {
        
    }
    boost::python::list EnumDevice()
    {
        boost::python::list comlist;

        return comlist;
    }
};
//--------------------------------------------------------- 
} // namespace python
} // namespace extensinon
} // namespace zhou_yb
//========================================================= 