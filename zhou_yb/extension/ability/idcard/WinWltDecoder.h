//========================================================= 
/**@file WinWltDecoder.h
 * @brief Windows下二代证照片数据解码器 
 * 
 * @date 2015-05-09   16:31:39
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_WINWLTDECODER_H_
#define _LIBZHOUYB_WINWLTDECODER_H_
//--------------------------------------------------------- 
#include "../../../device/idcard/IDCardParser.h"
using zhou_yb::device::idcard::IWltDecoder;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace ability {
//--------------------------------------------------------- 
/// Windows下二代证图片数据解码器 
class WinWltDecoder : public IWltDecoder, public LoggerBehavior, public RefObject
{
protected:
    //----------------------------------------------------- 
    /// WltRS.dll驱动路径 
    ByteBuilder _wltrs;
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /**
     * @brief 设置WltRS.dll路径 
     * 
     * @param [in] wltRs 路径 
     * - wltRs:
     *  - NULL或"" "WltRs.dll"
     *  - 以.dll结尾则直接使用 
     *  - 不以.dll结尾则为文件夹,查找目录下"WltRS.dll"
     * .
     * 
     * @return void 
     */
    void SetWltRS(const char* wltRs = NULL)
    {
        LOG_FUNC_NAME();

        _wltrs = wltRs;
        if(_wltrs.GetLength() < 1)
        {
            _wltrs = "WltRS.dll";
        }
        // 不以 .dll 结尾,则为文件夹 
        else if(!StringConvert::EndWith(_wltrs, ".dll", true))
        {
            if(_wltrs[_wltrs.GetLength() - 1] != PATH_SEPARATOR)
                _wltrs += static_cast<byte>(PATH_SEPARATOR);

            _wltrs += "WltRS.dll";
        }
        LOGGER(_log << "授权文件路径:<" << _wltrs.GetString() << ">\n");
        LOGGER(_logRetValue(true));
    }
    /// 将wlt数据解码为bmp数据 
    virtual bool WltToBmp(const ByteArray& photoMsg, const char* bmpFile)
    {
        LOG_FUNC_NAME();
        if(_wltrs.IsEmpty())
        {
            SetWltRS();
        }
        LOGGER(_log << "授权文件路径:<" << _wltrs.GetString() << ">\n");

        typedef int(__stdcall * lpGetBmp)(const char*, int);

        HINSTANCE hDll = NULL;
        lpGetBmp _GetBmp = NULL;
        CharConverter cvt;
        hDll = LoadLibrary(cvt.to_char_t(_wltrs.GetString()));
        if(NULL == hDll)
        {
            LOGGER(_log.WriteLine("加载授权文件失败"));
            return _logRetValue(false);
        }
        _GetBmp = reinterpret_cast<lpGetBmp>(GetProcAddress(hDll, "GetBmp"));
        if(NULL == _GetBmp)
        {
            LOGGER(_log.WriteLine("加载函数GetBmp失败"));
            FreeLibrary(hDll);
            return _logRetValue(false);
        }
        /* 解照片 */
        ofstream fout;
        ByteBuilder bmpPath = bmpFile;
        if(bmpPath.IsEmpty())
            bmpPath += ".";
        // 如果传入的文件名以.bmp结尾则改为.wlt以转换
        if(StringConvert::EndWith(bmpPath, ".bmp", true))
        {
            bmpPath.RemoveTail(4);
        }
        else
        {
            if(bmpPath[bmpPath.GetLength() - 1] != PATH_SEPARATOR)
                bmpPath += static_cast<byte>(PATH_SEPARATOR);

            bmpPath += "zp";
        }
        bmpPath += ".wlt";

        fout.open(bmpPath.GetString(), ios::out | ios::binary);
        if(fout.fail())
        {
            LOGGER(_log.WriteLine("创建wlt照片文件失败"));
            return _logRetValue(false);
        }
        /* 照片数据的起始地址 */
        fout.write(photoMsg.GetString(), photoMsg.GetLength());
        fout.close();

        BOOL iRet = _GetBmp(bmpPath.GetString(), 1);
        FreeLibrary(hDll);

        if(iRet != TRUE)
        {
            LOGGER(_log << "GetBmp返回:<" << iRet << ">\n");

            LOGGER(_log.WriteLine("调用GetBmp解码失败"));
            return _logRetValue(false);
        }
        return _logRetValue(true);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace ability
} // namespace extension
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_WINWLTDECODER_H_
//========================================================= 
