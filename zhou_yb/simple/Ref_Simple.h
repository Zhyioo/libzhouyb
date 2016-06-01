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
namespace simple {
//--------------------------------------------------------- 
struct Ref_Simple : public RefObject
{
    static void Main(LoggerAdapter& _log)
    {
        Ref<Ref_Simple> r1;
        Ref<Ref_Simple> r2;

        _log << r1.IsNull() << endl;
        _log << r2.IsNull() << endl;


        {
            Ref_Simple dev;
            r1 = dev;

            _log << r1.IsNull() << endl;
        }
        _log << r1.IsNull() << endl;

        if(!r1.IsNull())
        {
            ;
        }
    }
};
//--------------------------------------------------------- 
} // namespace simple
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_REF_SIMPLE_H_
//========================================================= 