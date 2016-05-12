//========================================================= 
/**@file Ref_Simple.h
 * @brief 
 * 
 * @date 2016-05-10   10:50:47
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_REF_SIMPLE_H_
#define _LIBZHOUYB_REF_SIMPLE_H_
//--------------------------------------------------------- 
#include <include/Base.h>
//--------------------------------------------------------- 
struct Ref_Simple : public RefObject
{
    static void Main(LoggerAdapter& _log)
    {

        Ref_Simple dev;
        Ref<Ref_Simple> r1;
        Ref<Ref_Simple> r2;

        r2 = r1;
        r1 = dev;

        _log << r1.IsNull() << endl;
        _log << r2.IsNull() << endl;
    }
};
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_REF_SIMPLE_H_
//========================================================= 