//========================================================= 
/**@file IConverter.h 
 * @brief 数据转换接口DLL函数导出声明(使用自动的工具宏导出为C方式) 
 * 
 * @date 2013-07-30   22:53:10 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
/*
 * 每个转换器DLL中可以包含多个转换接口,每个转换接口由3个函数构成
 * 其中,Tips接口可以不支持.转换时,ConvertFrom将数据转换,并在DLL中
 * 缓存,上层通过ConvertTo来分段获取数据.
 *
 * 转换的输入不一定仅限为数据,可以带一些配置信息,或者为文件路径,
 * 转换时输入路径,转换后直接以文件的形式将数据输出也行 
 *
 * 在配置插件时,由3部分组成:
 * "插件的显示名称" + "=" + "插件路径" + ["," + "插件入口"]
 * 上述格式中,中括号的数据可选,如果不指明入口点,则认为插件的显示名
 * 称就是入口点,比如:
 * BcdToAscii="[Driver:<.\converters\BaseConvert.dll>][Convert:<BcdToAscii>][Arg:<>][Name:<>]"
 * AsciiToBcd=".\converters\BaseConvert.dll"
 */
//--------------------------------------------------------- 
#include "../../include/Base.h"
#include "../../include/Extension.h"
//--------------------------------------------------------- 
/* 转换器DLL需要支持的接口 */
/// 将input按照format格式转换,返回需要的缓冲区大小,转换后的结果由ConvertTo保存
typedef size_t (__cdecl *fpConvertFrom)(const char* input, const char* arg);
/// 获取转换后的结果,返回当前已经拷贝的数据长度
typedef size_t (__cdecl *fpConvertTo)(char* output, size_t outsize);
/// 获取转换器的一些说明[128长度内]
typedef void   (__cdecl *fpTips)(char tips[128]);
//--------------------------------------------------------- 
/* 如果使用const char* 类型作为输入,ByteBuilder 类型作为输出则可以使用以下示例简化操作 */
//--------------------------------------------------------- 
/* 宏声明 */
/// 函数调用方式 
#ifndef CONVERTER_CALL
#   define CONVERTER_CALL __cdecl 
#endif
/// 函数导出方式 
#ifndef CONVERTER_API
#   define CONVERTER_API extern "C" __declspec(dllexport)
#endif
/// 转换后剩余数据的长度 
#define CONVERTER_LEN(spacename) _##spacename##_global_length
/// 转换的临时缓冲区 
#define CONVERTER_BUFF(spacename) _##spacename##_global_buffer

/// 转换的导出函数 
#define CONVERTER_EXPORT(spacename) \
ByteBuilder CONVERTER_BUFF(spacename)(128); \
size_t CONVERTER_LEN(spacename); \
CONVERTER_API size_t CONVERTER_CALL spacename##_ConvertFrom(const char* input, const char* arg) \
{ \
    CONVERTER_BUFF(spacename).Clear(); \
    spacename(input, CONVERTER_BUFF(spacename), arg); \
    CONVERTER_LEN(spacename) = CONVERTER_BUFF(spacename).GetLength(); \
    return CONVERTER_LEN(spacename); \
} \
CONVERTER_API size_t CONVERTER_CALL spacename##_ConvertTo(char* output, size_t outsize) \
{ \
    if(CONVERTER_LEN(spacename) < 1) \
        return 0; \
    size_t copylen = _min(outsize - 1, CONVERTER_LEN(spacename)); \
    size_t offset = CONVERTER_BUFF(spacename).GetLength() - CONVERTER_LEN(spacename); \
    memcpy(output, CONVERTER_BUFF(spacename).GetBuffer(offset), copylen); \
    output[copylen] = 0; \
    CONVERTER_LEN(spacename) -= copylen; \
    return copylen; \
}
#define CONVERTER_TIPS_EXPORT(spacename,tipsstr) \
CONVERTER_API void CONVERTER_CALL spacename##_Tips(char tips[128]) \
{ \
    int len = strlen(tipsstr); \
    memcpy(tips, tipsstr, len); \
    tips[len] = 0; \
}
/*
--- 函数实现 ---
bool BcdToAscii(const char* bcd, ByteBuilder& ascii, const char* arg)
{
    DevCommand::FromAscii(bcd, ascii);

    return true;
}
--- 转换接口定义(实际导出的转换函数) ---
CONVERTER_EXPORT(BcdToAscii);
CONVERTER_TIPS_EXPORT(BcdToAscii, "将16进制表示的字符串剔除空格后转换为ASCII码串(D6D0416231322121=>中Ab12!!)");
*/
//--------------------------------------------------------- 
/// 转换器接口
struct IConverter
{
    /**
     * @brief 转换接口
     * 
     * @param [in] input 需要转换的数据
     * @param [out] output 转换后的结果
     * @param [in] arg 转换时的参数设置"-x"
     * 
     * @return bool 
     */
    virtual bool Convert(const char* input, string& output, const char* arg) = 0;
};
//--------------------------------------------------------- 
/// 转换驱动加载器
class ConverterInvoker : public IConverter
{
protected:
    //----------------------------------------------------- 
    fpConvertFrom _ConvertFrom;
    fpConvertTo _ConvertTo;
    fpTips _Tips;

    string _tips;
    HMODULE _hDll;
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    ConverterInvoker()
    {
        _tips = "";
        _hDll = NULL;
        _ConvertFrom = NULL;
        _ConvertTo = NULL;
        _Tips = NULL;
    }
    virtual ~ConverterInvoker()
    {
        Dispose();
    }
    /// 转换数据
    virtual bool Convert(const char* input, string& output, const char* arg)
    {
        size_t buffsize = _ConvertFrom(input, arg);
        if(0 == buffsize)
            return false;

        const unsigned int BUFF_SIZE = 512;
        char recvbuff[BUFF_SIZE];
        size_t recvlen = 0;
        int num = buffsize / BUFF_SIZE;

        recvlen = _ConvertTo(recvbuff, BUFF_SIZE);
        while(recvlen > 0)
        {
            output += recvbuff;

            recvbuff[0] = 0;
            recvlen = _ConvertTo(recvbuff, BUFF_SIZE);
        }

        return true;
    }
    /// 从指定的驱动文件里面加载转换器
    bool Load(const char* path, const char* spacename = NULL)
    {
        _tips = path;

        if(NULL == path || strlen(path) < 1)
            return false;
        Dispose();

        CharConverter cvt;
        _hDll = LoadLibrary(cvt.to_char_t(path));
        if(!IsValid())
            return false;

        string convertFrom = "";
        string convertTo = "";
        string tips = "";
        // 有命名空间 
        if(NULL != spacename && strlen(spacename) > 1)
        {
            convertFrom += spacename;
            convertFrom += "_";

            convertTo += spacename;
            convertTo += "_";

            tips += spacename;
            tips += "_";
        }
        convertFrom += "ConvertFrom";
        convertTo += "ConvertTo";
        tips += "Tips";

        _ConvertFrom = (fpConvertFrom)GetProcAddress(_hDll, convertFrom.c_str());
        _ConvertTo = (fpConvertTo)GetProcAddress(_hDll, convertTo.c_str());
        _Tips = (fpTips)GetProcAddress(_hDll, tips.c_str());

        if(NULL == _ConvertFrom || NULL == _ConvertTo)
            return false;

        char tmpbuff[128] = { 0 };
        if(NULL != _Tips)
        {
            _Tips(tmpbuff);
            _tips = tmpbuff;
        }
        return true;
    }
    /// 返回转换器是否有效
    bool IsValid()
    {
        return NULL != _hDll;
    }
    /// 释放转换器驱动
    void Dispose()
    {
        if(!IsValid())
            return;

        FreeLibrary(_hDll);
        _hDll = NULL;

        _ConvertFrom = NULL;
        _ConvertTo = NULL;
        _Tips = NULL;

        _tips = "";
    }
    /// 返回转换器的内部提示信息
    const string& Tips() const
    {
        return _tips;
    }
    /// 转换器的显示标题
    string Title;
    /// 默认的配置参数
    string Arg;
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/// 转换器加载器
class ConverterLoader
{
protected:
    ConverterLoader() {}
public:
    /// 加载整个驱动里面的转换器
    static size_t LoadFromDriver(const char* driver, list<ConverterInvoker>& converterlist)
    {
        list<string> funclist;
        size_t count = 0;
        WinDllHelper::EnumFunction(driver, funclist);

        // 过滤以ConvertFrom结尾的函数名称
        list<string>::iterator itr;
        ByteArray nameEof("ConvertFrom");
        ByteBuilder funcName(8);
        list<string> entrypointlist;
        for(itr = funclist.begin();itr != funclist.end(); ++itr)
        {
            funcName = itr->c_str();
            if(StringConvert::EndWith(funcName, nameEof))
            {
                // "_ConvertFrom"
                funcName.RemoveTail(nameEof.GetLength() + 1);
                if(!funcName.IsEmpty())
                {
                    entrypointlist.push_back(funcName.GetString());
                }
            }
        }
        if(entrypointlist.size() < 1)
            return count;

        // 加载转换器
        list<string>::iterator epItr;
        for(epItr = entrypointlist.begin();epItr != entrypointlist.end(); ++epItr)
        {
            converterlist.push_back(ConverterInvoker());
            if(converterlist.back().Load(driver, epItr->c_str()))
            {
                converterlist.back().Title = (*epItr);
                ++count;
            }
            else
            {
                converterlist.pop_back();
            }
        }

        return count;
    }
    /// 从文件夹下加载
    static size_t LoadFromFolder(const char* folder, list<ConverterInvoker>& converterlist)
    {
        list<string> drvs;
        FolderHelper::EnumFiles(folder, drvs, NULL, "*.dll");

        size_t count;
        list<string>::iterator itr;
        for(itr = drvs.begin();itr != drvs.end(); ++itr)
        {
            count += LoadFromDriver(itr->c_str(), converterlist);
        }

        return count;
    }
    /**
     * @brief 根据配置加载转换器
     * 
     * @param [in] sArg 配置项参数
     *
     * - 配置格式:
     *  - 驱动文件路径
     *  - [Arg:<默认输入的参数>]
     *  - [Driver:<驱动路径>]
     *  - [Convert:<转换接口名称>]
     *  - [Name:<显示名称>]
     * .
     * @param [out] cvt 加载到的转换器
     */
    static bool LoadFromConfig(const char* sArg, ConverterInvoker& cvt)
    {
        ArgParser cfg;
        bool hasConfig = cfg.Parse(sArg);
        string path;
        string entrypoint;
        if(cfg.Parse(sArg))
        {
            if(!ArgConvert::FromConfig(cfg, "Driver", path, true))
                return false;
            ArgConvert::FromConfig(cfg, "Convert", entrypoint, true);
            ArgConvert::FromConfig(cfg, "Arg", cvt.Arg, true);
            ArgConvert::FromConfig(cfg, "Name", cvt.Title, true);
        }
        // 只有驱动文件路径,将驱动名称设定为转换接口名称
        ByteArray fileExt(".dll");
        if(!StringConvert::EndWith(ByteArray(path.c_str(), path.length()), fileExt, true))
            path += fileExt.GetString();
        ByteArray drvPath(path.c_str(), path.length());
        size_t index = StringConvert::LastIndexOf(drvPath, '/');
        size_t extindex = StringConvert::LastIndexOf(drvPath, '.');
        if(index == SIZE_EOF)
        {
            index = 0;
        }
        ByteBuilder spacename;
        if(entrypoint.length() < 1)
        {
            spacename = drvPath.SubArray(index, extindex);
        }
        else
        {
            spacename = entrypoint.c_str();
        }
        return cvt.Load(drvPath.GetString(), spacename.GetString());
    }
};
//========================================================= 