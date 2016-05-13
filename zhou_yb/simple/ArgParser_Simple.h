//========================================================= 
/**@file ArgParserSimple.h
 * @brief ArgParser½âÎöÆ÷Ê¾Àý
 * 
 * @date 2016-04-19   20:55:21
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_ARGPARSERSIMPLE_H_
#define _LIBZHOUYB_ARGPARSERSIMPLE_H_
//--------------------------------------------------------- 
#include <include/Base.h>
#include <extension/ability/SplitArgParser.h>
using zhou_yb::extension::ability::SplitArgParser;
//--------------------------------------------------------- 
namespace simple {
//--------------------------------------------------------- 
struct ArgParser_Simple
{
    static void Main(LoggerAdapter& _log, LoggerAdapter& _devlog, const char* sArg)
    {
        LOG_FUNC_NAME();

        _log << "Parse:<" << _strput(sArg) << ">\n";

        ArgParser arg;
        size_t count = arg.Parse(sArg);

        _log << "Count:<" << count << ">\n";

        string key;
        string val;

        arg.EnumValue(NULL);
        while(arg.EnumValue(&val, &key))
        {
            _log << "Key:<" << key << ">,Val:<" << val << ">\n";
        }

        _log << "int Index=" << arg["Index"].To<int>(5) << endl;
        _log << "string Key=" << arg["Key"].To<string>() << endl;
    }
};
struct SplitArgParserSimple
{
    static void Main(LoggerAdapter& _log, LoggerAdapter& _devlog, const char* sArg)
    {
        LOG_FUNC_NAME();

        _log << "Parse:<" << _strput(sArg) << ">\n";

        SplitArgParser arg;
        size_t count = arg.Parse(sArg);

        _log << "Count:<" << count << ">\n";

        string key;
        string val;

        arg.EnumValue(NULL);
        while(arg.EnumValue(&val, &key))
        {
            _log << "Key:<" << key << ">,Val:<" << val << ">\n";
        }
        arg.EnumValue(NULL);
        _log << "int Index=" << arg["Index"].To<int>(5) << endl;
        _log << "string Key=" << arg["Key"].To<string>() << endl;
    }
};
//--------------------------------------------------------- 
} // namespace simple
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_ARGPARSERSIMPLE_H_
//========================================================= 