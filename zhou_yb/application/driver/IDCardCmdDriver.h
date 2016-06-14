//========================================================= 
/**@file IDCardCmdDriver.h
 * @brief 二代证阅读器命令驱动
 * 
 * @date 2016-05-04   21:02:01
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_IDCARDCMDDRIVER_H_
#define _LIBZHOUYB_IDCARDCMDDRIVER_H_
//--------------------------------------------------------- 
#include "CommonCmdDriver.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace driver {
//--------------------------------------------------------- 
/// 二代证命令驱动
class IDCardCmdDriver : 
    public DevAdapterBehavior<IInteractiveTrans>, 
    public InterruptBehavior,
    public CommandCollection, 
    public RefObject
{
protected:
    LoggerInvoker _logInvoker;
    LastErrInvoker _objErr;
    LastErrExtractor _lastErr;
    InterruptInvoker _interruptInvoker;
    SdtApiDevAdapter _sdtapiAdapter;
    IDCardParser _idParser;
public:
    static void ToArgParser(IDCardInformation& idInfo, ICommandHandler::CmdArgParser& rlt)
    {
        rlt.PushValue("Name", idInfo.Name);
        rlt.PushValue("ID", idInfo.ID);
        rlt.PushValue("Gender", idInfo.Gender);
        rlt.PushValue("Nation", idInfo.Nation);
        rlt.PushValue("Birthday", idInfo.Birthday);
        rlt.PushValue("Address", idInfo.Address);
        rlt.PushValue("Department", idInfo.Department);
        rlt.PushValue("StartDate", idInfo.StartDate);
        rlt.PushValue("EndDate", idInfo.EndDate);
        rlt.PushValue("GenderCode", ArgConvert::ToString<uint>(idInfo.GenderCode));
        rlt.PushValue("NationCode", ArgConvert::ToString<uint>(idInfo.NationCode));
    }
    static void FromArgParser(ICommandHandler::CmdArgParser& arg, IDCardInformation& idInfo)
    {
        idInfo.Name = arg["Name"].To<string>();
        idInfo.ID = arg["ID"].To<string>();
        idInfo.Gender = arg["Gender"].To<string>();
        idInfo.Nation = arg["Nation"].To<string>();
        idInfo.Birthday = arg["Birthday"].To<string>();
        idInfo.Address = arg["Address"].To<string>();
        idInfo.Department = arg["Department"].To<string>();
        idInfo.StartDate = arg["StartDate"].To<string>();
        idInfo.EndDate = arg["EndDate"].To<string>();
        idInfo.GenderCode = arg["GenderCode"].To<ushort>();
        idInfo.NationCode = arg["NationCode"].To<ushort>();
    }

    IDCardCmdDriver()
    {
        _objErr.Invoke(_lasterr, _errinfo);

        _lastErr.IsFormatMSG = false;
        _lastErr.IsLayerMSG = true;
        _lastErr.Select(_sdtapiAdapter, "SDTAPI");
        _lastErr.Select(_idParser, "IDC");
        _lastErr.Select(_objErr);

        _idParser.SelectDevice(_sdtapiAdapter);

        select_helper<LoggerInvoker::SelecterType>::select(_logInvoker),
            _sdtapiAdapter, _idParser;

        select_helper<InterruptInvoker::SelecterType>::select(_interruptInvoker),
            _idParser;

        IdcConvert = NULL;

        _Registe("WaitIdCard", (*this), &IDCardCmdDriver::WaitIdCard);
    }
    LC_CMD_INTERRUPT(_interruptInvoker);
    LC_CMD_ADAPTER(IInteractiveTrans, _sdtapiAdapter);
    LC_CMD_LASTERR(_lastErr);
    LC_CMD_LOGGER(_logInvoker);
    /**
     * @brief 读取身份证信息
     * @date 2016-06-11 22:30
     * 
     * @param [in] Timeout : uint 等待放卡的超时时间(以毫秒为单位)
     * @param [in] FingerFormat : string 指纹数据格式
     * - 参数:
     *  - 空 不需要指纹
     *  - Hex
     *  - Base64
     * .
     * @param [in] Bmpfile : string 需要生成的照片路径
     * 
     * @retval Name : string 姓名
     * @retval Gender : string 性别
     * @retval Nation : string 民族
     * @retval Birthday : string 出生年月
     * @retval Address : string 住址
     * @retval Department : string 签发机关
     * @retval StartDate : string 起始日期
     * @retval EndDate : string 截至日期
     * @retval GenderCode : uint 性别代码
     * @retval NationCode : uint 民族代码
     * @retval Finger : string 指纹数据
     * @retval Bmpfile : string 生成的照片路径
     */
    LC_CMD_METHOD(WaitIdCard)
    {
        uint timeoutMs = arg["Timeout"].To<uint>(DEV_WAIT_TIMEOUT);
        string fingerFormat = arg["FingerFormat"].To<string>();
        string bmp = arg["Bmpfile"].To<string>();

        // 是否需要读取指纹
        bool isFinger = fingerFormat.length() > 0;
        // 是否需要解析照片
        bool isBmp = bmp.length() > 0;

        ByteBuilder txtMsg(256);
        ByteBuilder bmpMsg(1024);
        ByteBuilder finger(256);

        _idParser.SetWaitTimeout(timeoutMs);
        if(!_idParser.GetBaseInformation(txtMsg, isBmp ? &bmpMsg : NULL, isFinger ? &finger : NULL))
            return false;

        // 解析基本数据
        IDCardInformation idInfo;
        if(!_idParser.ParseToTXT(txtMsg, idInfo, IdcConvert))
            return false;
        ToArgParser(idInfo, rlt);
        // 转换指纹
        if(isFinger)
        {
            ByteBuilder fingerBuff(512);
            if(StringConvert::Compare(fingerFormat.c_str(), "Base64", true))
            {
                Base64_Provider::Encode(finger, fingerBuff);
            }
            else
            {
                ByteConvert::ToAscii(finger, fingerBuff);
            }
            rlt.PushValue("Finger", fingerBuff.GetString());
        }
        if(isBmp)
        {
            // 解析照片
            if(bmp.length() > 0 && WltDecoder.IsNull())
            {
                _logErr(DeviceError::ArgIsNullErr, "解码器为空");
                return false;
            }
            ByteBuilder finalBmp(8);
            if(!_idParser.ParseToBMP(WltDecoder, bmpMsg, bmp.c_str(), &finalBmp))
                return false;
            rlt.PushValue("Bmpfile", finalBmp.GetString());
        }
        return true;
    }
    /// 二代证信息格式转换器
    IDCardParser::fpIdcConvert IdcConvert;
    /// 二代证照片解码器
    Ref<IWltDecoder> WltDecoder;
};
//--------------------------------------------------------- 
} // nemespace driver
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_IDCARDCMDDRIVER_H_
//========================================================= 