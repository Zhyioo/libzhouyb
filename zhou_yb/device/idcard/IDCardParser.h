//========================================================= 
/**@file IDCardParser.h 
 * @brief 身份证阅读器的操作流程及数据格式解析转换 
 * 
 * @date 2014-10-17   17:43:05 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_IDCARDPARSER_H_
#define _LIBZHOUYB_IDCARDPARSER_H_
//--------------------------------------------------------- 
#include "ISdtApi.h"
#include "IDCardCharsetConvert.h"
using std::wstring;
//---------------------------------------------------------
namespace zhou_yb {
namespace device {
namespace idcard {
//---------------------------------------------------------
/// 身份证中对应数据的起始地址和长度
extern const ushort _id_info_table[16];
//---------------------------------------------------------
/// 身份证信息 
struct IDCardInformation
{
    /// 姓名
    string Name;
    /// 性别
    string Gender;
    /// 民族
    string Nation;
    /// 生日
    string Birthday;
    /// 住址
    string Address;
    /// 身份证号
    string ID;
    /// 签发机关
    string Department;
    /// 有效起始日期
    string StartDate;
    /// 有效截止日期
    string EndDate;
    /// 保留(16进制原始数据) 
    string Reserved;
    /// 性别代码
    ushort GenderCode;
    /// 民族代码
    ushort NationCode;
};
/// 身份证信息(Unicode编码)
struct wIDCardInformation
{
    /// 姓名 
    wstring Name;
    /// 性别
    wstring Gender;
    /// 民族
    wstring Nation;
    /// 生日
    wstring Birthday;
    /// 住址
    wstring Address;
    /// 身份证号
    wstring ID;
    /// 签发机关
    wstring Department;
    /// 有效起始日期
    wstring StartDate;
    /// 有效截止日期
    wstring EndDate;
    /// 保留 
    wstring Reserved;
    /// 性别代码
    ushort GenderCode;
    /// 民族代码
    ushort NationCode;
};
/// WLT数据照片解码器接口  
struct IWltDecoder
{
    /**
     * @brief WltToBmp 将二代证WLT数据解码为BMP文件 
     * 
     * @param [in] photoMsg 读取出来的原始图像数据 
     * @param [in] bmpFile 生成的bmp文件路径 
     * 
     * @return bool 返回是否解码成功 
     */
    virtual bool WltToBmp(const ByteArray& photoMsg, const char* bmpFile) = 0;
};
//--------------------------------------------------------- 
/// 身份证阅读器
class IDCardParser : 
    public DevAdapterBehavior<ISdtApi>,
    public TimeoutBehavior,
    public InterruptBehavior,
    public RefObject
{
public:
    //----------------------------------------------------- 
    /// 转换错误码为字符串描述信息 
    virtual const char* TransErrToString(int errCode) const
    {
        return DeviceError::TransErrToString(errCode);
    }
    //----------------------------------------------------- 
    /// 根据民族的标识号获取相关的民族文字描述信息(Unicode编码)
    static const wchar_t* GetNation(ushort nationNum)
    {
        switch(nationNum)
        {
        case 1:
            return L"汉";
        case 2:
            return L"蒙古";
        case 3:
            return L"回";
        case 4:
            return L"藏";
        case 5:
            return L"维吾尔";
        case 6:
            return L"苗";
        case 7:
            return L"彝";
        case 8:
            return L"壮";
        case 9:
            return L"布依";
        case 10:
            return L"朝鲜";
        case 11:
            return L"满";
        case 12:
            return L"侗";
        case 13:  
            return L"瑶";
        case 14: 
            return L"白";
        case 15: 
            return L"土家";
        case 16: 
            return L"哈尼";
        case 17: 
            return L"哈萨克";
        case 18:  
            return L"傣";
        case 19:  
            return L"黎";
        case 20:  
            return L"傈僳";
        case 21:  
            return L"佤";
        case 22:  
            return L"畲";
        case 23:  
            return L"高山";
        case 24:  
            return L"拉祜";
        case 25:  
            return L"水";
        case 26:  
            return L"东乡";
        case 27:  
            return L"纳西";
        case 28:  
            return L"景颇";
        case 29:  
            return L"柯尔克孜";
        case 30:  
            return L"土";
        case 31:  
            return L"达斡尔";
        case 32:  
            return L"仫佬";
        case 33:  
            return L"羌";
        case 34:  
            return L"布朗";
        case 35:  
            return L"撒拉";
        case 36:  
            return L"毛南";
        case 37:  
            return L"仡佬";
        case 38:  
            return L"锡伯";
        case 39:  
            return L"阿昌";
        case 40:  
            return L"普米";
        case 41:  
            return L"塔吉克";
        case 42:  
            return L"怒";
        case 43:  
            return L"乌孜别克";
        case 44:  
            return L"俄罗斯";
        case 45:  
            return L"鄂温克";
        case 46:  
            return L"德昂";
        case 47:  
            return L"保安";
        case 48:  
            return L"裕固";
        case 49:  
            return L"京";
        case 50:  
            return L"塔塔尔";
        case 51:  
            return L"独龙";
        case 52:  
            return L"鄂伦春";
        case 53:  
            return L"赫哲";
        case 54:  
            return L"门巴";
        case 55:  
            return L"珞巴";
        case 56:  
            return L"基诺";
        case 97:  
            return L"其他";
        case 98:  
            return L"外国血统中国籍人士";
        }
        return L"";
    }
    /// 转换函数 
    typedef size_t (*fpIdcConvert)(const wchar_t* unicode, size_t unicodeLen, ByteBuilder& gbk);
    /// 转换身份证信息编码格式 
    static bool wIDCtoIDC(wIDCardInformation& wIdInfo, IDCardInformation& idInfo, fpIdcConvert cvt)
    {
        ByteBuilder gbk(64);
        // 住址 
        if(!cvt(wIdInfo.Address.c_str(), wIdInfo.Address.length(), gbk))
            return false;
        idInfo.Address = gbk.GetString();
        gbk.Clear();
        // 生日 
        if(!cvt(wIdInfo.Birthday.c_str(), wIdInfo.Birthday.length(), gbk))
            return false;
        idInfo.Birthday = gbk.GetString();
        gbk.Clear();
        // 签发机关 
        if(!cvt(wIdInfo.Department.c_str(), wIdInfo.Department.length(), gbk))
            return false;
        idInfo.Department = gbk.GetString();
        gbk.Clear();
        // 截止日期 
        if(!cvt(wIdInfo.EndDate.c_str(), wIdInfo.EndDate.length(), gbk))
            return false;
        idInfo.EndDate = gbk.GetString();
        gbk.Clear();
        // 身份证号  
        if(!cvt(wIdInfo.ID.c_str(), wIdInfo.ID.length(), gbk))
            return false;
        idInfo.ID = gbk.GetString();
        gbk.Clear();
        // 姓名  
        if(!cvt(wIdInfo.Name.c_str(), wIdInfo.Name.length(), gbk))
            return false;
        idInfo.Name = gbk.GetString();
        gbk.Clear();
        // 民族  
        if(!cvt(wIdInfo.Nation.c_str(), wIdInfo.Nation.length(), gbk))
            return false;
        idInfo.Nation = gbk.GetString();
        idInfo.NationCode = wIdInfo.NationCode;
        gbk.Clear();
        // 性别  
        if(!cvt(wIdInfo.Gender.c_str(), wIdInfo.Gender.length(), gbk))
            return false;
        idInfo.Gender = gbk.GetString();
        idInfo.GenderCode = wIdInfo.GenderCode;
        gbk.Clear();
        // 起始日期 
        if(!cvt(wIdInfo.StartDate.c_str(), wIdInfo.StartDate.length(), gbk))
            return false;
        idInfo.StartDate = gbk.GetString();
        // 保留数据  
        if(!cvt(wIdInfo.Reserved.c_str(), wIdInfo.Reserved.length(), gbk))
            return false;
        idInfo.Reserved = gbk.GetString();
        gbk.Clear();

        return true;
    }
    /// 将身份证信息整合到一个字符串中("姓名|身份证号|性别|民族|出生日期|住址|签发机关|起始日期|截止日期|性别代码|民族代码")
    static void Join(IDCardInformation& idInfo, string& msg, const char* splitFlag = SPLIT_STRING)
    {
        string splitMsg = _strput(splitFlag);

        msg += idInfo.Name;
        msg += splitMsg;

        msg += idInfo.ID;
        msg += splitMsg;

        msg += idInfo.Gender;
        msg += splitMsg;

        msg += idInfo.Nation;
        msg += splitMsg;

        msg += idInfo.Birthday;
        msg += splitMsg;

        msg += idInfo.Address;
        msg += splitMsg;

        msg += idInfo.Department;
        msg += splitMsg;

        msg += idInfo.StartDate;
        msg += splitMsg;

        msg += idInfo.EndDate;
        msg += splitMsg;

        msg += ArgConvert::ToString<ushort>(idInfo.GenderCode);
        msg += splitMsg;

        msg += ArgConvert::ToString<ushort>(idInfo.NationCode);
    }
    //-----------------------------------------------------
protected:
    //-----------------------------------------------------
    /// 获取转换后的数据 
    static void _id_subdata(const ByteArray& txtMsg, int idx, wchar_t* buf, size_t buflen)
    {
        memset(buf, 0, buflen);
        size_t len = _id_info_table[idx + 1] - _id_info_table[idx];
        ushort* pSrc = reinterpret_cast<ushort*>(const_cast<byte*>(txtMsg.GetBuffer()) + _id_info_table[idx]);
        // 按字节拷贝 
        for(size_t i = 0;i < len/2; ++i)
            buf[i] = pSrc[i];
        // 从第一个空格开始截断 
        size_t tmplen = wcslen(buf);
        for(size_t i = 0;i < tmplen; ++i)
        {
            if(buf[i] == L' ')
            {
                buf[i] = L'\0';
                break;
            }
        }
    }
    //-----------------------------------------------------
public:
    //----------------------------------------------------- 
    /**
     * @brief GetBaseInformation 获取身份证基本信息
     * 
     * @param [out] txtMsg 获取到的身份证文本信息
     * @param [out] pPhotoMsg [default:NULL] 获取到的身份证图片信息 
     * @param [out] pFingerMsg [default:NULL] 获取到的身份证指纹信息 
     * 
     * @return bool 
     */
    bool GetBaseInformation(ByteBuilder& txtMsg, ByteBuilder* pPhotoMsg = NULL, ByteBuilder* pFingerMsg = NULL)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        Timer timer;
        LOGGER(Timer _wait);
        int step = 0;
        bool bRead = false;

        while(timer.Elapsed() < _waitTimeout)
        {
            /* 中断支持 */
            if(!Interrupter.IsNull() && Interrupter->InterruptionPoint())
            {
                _logErr(DeviceError::OperatorInterruptErr, "操作被取消");
                return _logRetValue(false);
            }

            switch(step)
            {
            case 0:
                if(_pDev->FindIDCard())
                {
                    ++step;
                    LOGGER(_log << "寻卡成功,耗时:<" << _wait.TickCount() << "ms>\n");
                    timer.Restart();
                    continue;
                }
                break;
            case 1:
                if(_pDev->SelectIDCard())
                {
                    ++step;
                    LOGGER(_log << "选卡成功,耗时:<" << _wait.TickCount() << "ms>\n");
                    timer.Restart();
                    continue;
                }
                break;
            case 2:
                if(pFingerMsg != NULL)
                    bRead = _pDev->ReadMsgFinger(&txtMsg, pPhotoMsg, pFingerMsg);
                else
                    bRead = _pDev->ReadBaseMsg(&txtMsg, pPhotoMsg);

                if(bRead)
                {
                    ++step;
                    LOGGER(_log << "读卡成功,耗时:<" << _wait.TickCount() << "ms>\n");

                    return _logRetValue(true);
                }
                break;
            default:
                break;
            }

            Timer::Wait(_waitInterval);
        }

        _logErr(DeviceError::WaitTimeOutErr);
        return _logRetValue(false);
    }
    /// 转换身份证数据为文本信息 
    bool ParseToTXT(const ByteArray& txtMsg, wIDCardInformation& wIdInfo)
    {
        LOG_FUNC_NAME();
        wchar_t buffer[256] = { 0 };
        int idx = 0;
        // 姓名
        _id_subdata(txtMsg, idx++, buffer, sizeof(buffer));
        wIdInfo.Name = buffer;
        // 性别
        _id_subdata(txtMsg, idx++, buffer, sizeof(buffer));
        wIdInfo.GenderCode = static_cast<ushort>(buffer[0] - L'0');
        switch(wIdInfo.GenderCode)
        {
        case 1:
            wIdInfo.Gender = L"男";
            break;
        case 2:
            wIdInfo.Gender = L"女";
            break;
        case 9:
            wIdInfo.Gender = L"未说明";
            break;
        default:
            wIdInfo.Gender = L"未知";
            break;
        }
        // 民族
        _id_subdata(txtMsg, idx++, buffer, sizeof(buffer));
        ushort nationIndex = 10 * (buffer[0] - L'0') + (buffer[1] - L'0');
        wIdInfo.NationCode = nationIndex;
        wIdInfo.Nation = GetNation(nationIndex);
        // 出生年月
        _id_subdata(txtMsg, idx++, buffer, sizeof(buffer));
        wIdInfo.Birthday = buffer;
        // 地址
        _id_subdata(txtMsg, idx++, buffer, sizeof(buffer));
        wIdInfo.Address = buffer;
        // 身份证
        _id_subdata(txtMsg, idx++, buffer, sizeof(buffer));
        wIdInfo.ID = buffer;
        // 发证机关
        _id_subdata(txtMsg, idx++, buffer, sizeof(buffer));
        wIdInfo.Department = buffer;
        // 起始日期
        _id_subdata(txtMsg, idx++, buffer, sizeof(buffer));
        wIdInfo.StartDate = buffer;
        // 终止日期
        _id_subdata(txtMsg, idx++, buffer, sizeof(buffer));
        wIdInfo.EndDate = buffer;
        // 保留项 
        _id_subdata(txtMsg, idx++, buffer, sizeof(buffer));
        wIdInfo.Reserved = buffer;

        return _logRetValue(true);
    }
    /**
     * @brief 转换身份证数据为文本信息 
     * @param [in] txtMsg 身份证基本数据 
     * @param [out] idInfo 多字节格式的身份证数据 
     * @param [in] IdcConvert [default:NULL] 字符集转换器件,NULL表示使用内嵌的字符集
     */ 
    bool ParseToTXT(const ByteArray& txtMsg, IDCardInformation& idInfo, fpIdcConvert IdcConvert = NULL)
    {
        LOG_FUNC_NAME();

        wIDCardInformation wIdInfo;
        ASSERT_FuncRet(ParseToTXT(txtMsg, wIdInfo));

        if(IdcConvert == NULL)
            IdcConvert = IDCardCharsetConvert::UnicodeToGBK;

        IDCardParser::wIDCtoIDC(wIdInfo, idInfo, IdcConvert);

        LOGGER(
        _log << "姓名:<" << idInfo.Name << ">\n"
            << "性别:<" << idInfo.Gender << ">\n"
            << "民族:<" << idInfo.Nation << ">\n"
            << "生日:<" << idInfo.Birthday << ">\n"
            << "住址:<" << idInfo.Address << ">\n"
            << "身份证号:<" << idInfo.ID << ">\n"
            << "签发机关:<" << idInfo.Department << ">\n"
            << "起始日期:<" << idInfo.StartDate << ">\n"
            << "截止日期:<" << idInfo.EndDate << ">\n"
            << "保留:<" << idInfo.Reserved << ">\n");

        return _logRetValue(true);
    }
    /**
     * @brief 解析照片
     *
     * @param [in] IWltDecoder 解码器 
     *
     * @param [in] photoMsg 读取出来的照片基本数据(未经过转换的原始数据)
     * @param [in] bmpPath [default:NULL] 生成的图片位置 
     * - bmpPath格式 
     *  - 1. NULL,"" 在当前目录下生成名称为<zp.bmp>的图片
     *  - 2. *.bmp格式 保存到指定的文件路径下 
     *  - 3. 不以*.bmp结尾 保存到指定文件夹下，图片名以身份证号命名 
     * .
     * @param [out] pFinalPath [default:NULL] 最后确定下来的bmp存放位置(为空则表示不需要) 
     */
    bool ParseToBMP(Ref<IWltDecoder> wltDecoder, const ByteArray& photoMsg, 
        const char* bmpPath = NULL, ByteBuilder* pFinalPath = NULL)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "BmpPath:<" << _strput(bmpPath) << ">\n");

        ByteBuilder bmpfile = bmpPath;
        if(bmpfile.GetLength() < 1)
            bmpfile += ".";
        // 不以 .bmp 结尾
        if(!StringConvert::EndWith(bmpfile, ".bmp", true))
        {
            if(bmpfile[bmpfile.GetLength() - 1] != PATH_SEPARATOR)
                bmpfile += static_cast<byte>(PATH_SEPARATOR);
            bmpfile += "zp.bmp";
        }

        LOGGER(_log<<"图片信息路径:<"<<bmpfile.GetString()<<">\n");
        if(NULL != pFinalPath)
            *pFinalPath = bmpfile;

        ASSERT_FuncErrInfoRet(!wltDecoder.IsNull(), DeviceError::ArgIsNullErr, "解码器错误");
        ASSERT_FuncErrInfoRet(wltDecoder->WltToBmp(photoMsg, bmpfile.GetString()), DeviceError::ArgFormatErr, "解码图片数据失败");

        return _logRetValue(true);
    }
    //-----------------------------------------------------
};
//--------------------------------------------------------- 
} // namespace idcard 
} // namespace device 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_IDCARDPARSER_H_
//=========================================================
