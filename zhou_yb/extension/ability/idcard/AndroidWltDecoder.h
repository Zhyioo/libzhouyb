//========================================================= 
/**@file AndroidWltDecoder.h
 * @brief Android下Wlt数据解码器 
 * 
 * @date 2015-05-09   16:31:53
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include <dlfcn.h>

#include "../../../device/idcard/IDCardParser.h"
using zhou_yb::device::idcard::IWltDecoder;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace ability {
//--------------------------------------------------------- 
/// Android下二代证图片数据解码器 
class AndroidWltDecoder : public IWltDecoder, public LoggerBehavior, public RefObject
{
public:
    //----------------------------------------------------- 
    /**
     * @brief 将解码出来的BMP内存数据保存成一个bmp文件
     * @param [in][out] bmpRGB 126*102*3 大小的RGB数据
     * @param [in] bmpPath [default:NULL] 生成的bmp文件当前路径
     * - 路径:
     *  - *.bmp结尾则生成到指定的目录下
     *  - 为目录则生成名称为zp.bmp
     * .
     * @param [out] bmpfinalPath 最后实际生成的bmp文件路径
     */
    static bool RGBToBMP(ByteArray& bmpRGB, const char* bmpPath = NULL, ByteBuilder* bmpfinalPath = NULL)
    {
        ByteBuilder bmpfile = bmpPath;
        if(bmpfile.IsEmpty())
            bmpfile += ".";
        // 不以 .bmp 结尾
        if(!StringConvert::EndWith(bmpfile, ".bmp", true))
        {
            if(StringConvert::LastCharAt(bmpfile, 0) != PATH_SEPARATOR)
                bmpfile += static_cast<byte>(PATH_SEPARATOR);
            bmpfile += "zp.bmp";
        }
        if(bmpfinalPath != NULL)
        {
            (*bmpfinalPath) = bmpfile;
        }
        // 交换R,B的顺序,调整为:BGR
        size_t BMP_W = 102;
        size_t BMP_H = 126;
        size_t BMP_SZIE = BMP_W * BMP_H * 3;

        ofstream fout;
        fout.open(bmpfile.GetString(), ios::binary | ios::out);
        if(fout.fail())
            return false;

        // 文件头数据
        const char BMP_HEADER[] =
            "424DCE970000000000003600000028"
            "000000660000007E00000001001800"
            "000000000000000000000000000000"
            "000000000000000000";

        byte tmp = 0;
        for(size_t i = 0;i < BMP_SZIE; i += 3)
        {
            tmp = bmpRGB[i];
            bmpRGB[i] = bmpRGB[i + 2];
            bmpRGB[i + 2] = tmp;
        }
        ByteBuilder BMP_HEADER_HEX(54);
        ByteConvert::FromAscii(BMP_HEADER, BMP_HEADER_HEX);
        fout.write(reinterpret_cast<char*>(const_cast<byte*>(BMP_HEADER_HEX.GetBuffer())), BMP_HEADER_HEX.GetLength());

        byte* pBmpRGB = const_cast<byte*>(bmpRGB.GetBuffer());
        for(size_t i = 0;i < BMP_H; ++i)
        {
            fout.write(reinterpret_cast<char*>(pBmpRGB + i*BMP_W*3), BMP_W*3);
            fout.write(reinterpret_cast<char*>(pBmpRGB + i*BMP_W*3-3), 2);
        }
        fout.close();

        return true;
    }
    //----------------------------------------------------- 
protected:
    //----------------------------------------------------- 
    /// WltRS.dll驱动路径 
    ByteBuilder _wltrs;
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /// 设置WltRS.dll路径 
    void SetWltRS(const char* wltRs = NULL)
    {
        LOG_FUNC_NAME();

        _wltrs = wltRs;
        if(_wltrs.GetLength() < 1)
        {
            _wltrs = "libwlt2bmp.so";
        }
        // 不以 .so 结尾,则为文件夹
        else if(!StringConvert::EndWith(_wltrs, ".so", true))
        {
            if(_wltrs[_wltrs.GetLength() - 1] != PATH_SEPARATOR)
                _wltrs += static_cast<byte>(PATH_SEPARATOR);

            _wltrs += "libwlt2bmp.so";
        }
        LOGGER(_log << "授权文件路径:<" << _wltrs.GetString() << ">\n");
        LOGGER(_logRetValue(true));
    }
    //-----------------------------------------------------
    /// 将wlt数据解码为bmp数据
    virtual bool WltToBmp(const ByteArray& photoMsg, const char* bmpFile)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "授权文件路径:<" << _wltrs.GetString() << ">\n");

        typedef int (*fpwlt2bmp)(const byte *src, byte *dst, int bmpSave);

        void* handle = NULL;
        handle = dlopen(_wltrs.GetString(), RTLD_LAZY);
        if(handle == NULL)
        {
            LOGGER(_log<<"加载授权文件失败["<<dlerror()<<"]\n");
            return _logRetValue(false);
        }

        fpwlt2bmp wlt2bmp = NULL;
        wlt2bmp = (fpwlt2bmp)dlsym(handle, "wlt2bmp");

        if(wlt2bmp == NULL)
        {
            LOGGER(_log<<"加载函数失败["<<dlerror()<<"]\n");
            dlclose(handle);
            return _logRetValue(false);
        }

        const size_t BMP_SIZE = 102*126*3;
        byte dst[BMP_SIZE + 1] = {0};
        int iRet = wlt2bmp(photoMsg.GetBuffer(), dst, 198);
        LOGGER(_log<<"wlt2bmp返回:<"<<iRet<<">\n");
        if(iRet != 1)
        {
            LOGGER(_log.WriteLine("解析照片数据失败"));
            return _logRetValue(false);
        }
        /* 解照片 */
        ByteArray dstArray(dst, BMP_SIZE);
        if(!RGBToBMP(dstArray, bmpFile))
        {
            LOGGER(_log.WriteLine("生成照片文件失败"));
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
//========================================================= 
