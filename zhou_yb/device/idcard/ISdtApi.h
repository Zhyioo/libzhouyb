//========================================================= 
/**@file ISdtApi.h 
 * @brief 公安部SDT身份证阅读器驱动接口描述文件,仅仅定义读取
 * 身份证信息所需要的基本接口 
 * 
 * @date 2014-12-25   15:44:31 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../include/Base.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace device {
namespace idcard {
//--------------------------------------------------------- 
/// SdtApi接口封装 
struct ISdtApi
{
    //----------------------------------------------------- 
    /// SDT错误码 
    enum SDT_Error
    {
        /// 错误 
        SDT_Err = 0x00,
        /// 端口打开失败/端口不合法
        SDT_PortErr = 0x01,
        /// 超时,设置不成功
        SDT_TimeoutErr = 0x02,
        /// 数据传输错误
        SDT_TransmitErr = 0x03,
        /// 无法获得SAM_V波特率,SAM_V不可用
        SDT_SAM_V_Err = 0x05,
        /// 接收业务终端数据的校验和错
        SDT_CheckValueErr = 0x10,
        /// 接收业务终端数据的长度错
        SDT_LengthErr = 0x11,
        /// uiCurrBaud,uiSetBaud输入参数值错误
        SDT_ArgErr = 0x21,
        /// 越权操作
        SDT_PermissionErr = 0x23,
        /// 无法识别的错误
        SDT_UnknownErr = 0x24,
        /// 证/卡认证SAM_V失败
        SDT_CardToSamvErr = 0x31,
        /// SAM_V认证证/卡失败
        SDT_SamvToCardErr = 0x32,
        /// 信息验证失败
        SDT_InfoErr = 0x33,
        /// 无法识别的卡类型
        SDT_CardErr = 0x40,
        /// 读证/卡失败
        SDT_ReadErr = 0x41,
        /// 取随机数失败
        SDT_RandomErr = 0x47,
        /// SAM_V自检失败,不能接收命令
        SDT_SamvErr = 0x60,
        /// SAM_V未经过授权,无法使用
        SDT_SamvPermissionErr = 0x66,
        /// 找卡失败
        SDT_FindCardErr = 0x80,
        /// 选卡失败
        SDT_SelectCardErr = 0x81,
        /// 成功 
        SDT_Success = 0x90,
        /// 没有信息 
        SDT_NoInfoErr = 0x91,
        /// 找卡成功
        SDT_FindCardSuccess = 0x9F,

        ENUM_MAX(SDT_Error)
    };
    //----------------------------------------------------- 
    /// 将错误码转为错误信息 
    static const char* TransErrToString(int iRet)
    {
        switch(iRet)
        {
        case SDT_Success:
            return "成功";
        case SDT_PortErr:
            return "端口打开失败/端口不合法";
        case SDT_TimeoutErr:
            return "超时,设置不成功";
        case SDT_TransmitErr:
            return "数据传输错误";
        case SDT_SAM_V_Err:
            return "无法获得SAM_V波特率,SAM_V不可用";
        case SDT_CheckValueErr:
            return "接收业务终端数据的校验和错";
        case SDT_LengthErr:
            return "接收业务终端数据的长度错";
        case SDT_ArgErr:
            return "uiCurrBaud,uiSetBaud输入参数值错误";
        case SDT_PermissionErr:
            return "越权操作";
        case SDT_UnknownErr:
            return "无法识别的错误";
        case SDT_FindCardSuccess:
            return "找卡成功";
        case SDT_FindCardErr:
            return "找卡失败";
        case SDT_SelectCardErr:
            return "选卡失败";
        case SDT_CardToSamvErr:
            return "证/卡认证SAM_V失败";
        case SDT_SamvToCardErr:
            return "SAM_B认证证/卡失败";
        case SDT_InfoErr:
            return "信息验证失败";
        case SDT_CardErr:
            return "无法识别的卡类型";
        case SDT_ReadErr:
            return "读证/卡失败";
        case SDT_RandomErr:
            return "取随机数失败";
        case SDT_SamvErr:
            return "SAM_V自检失败,不能接收命令";
        case SDT_SamvPermissionErr:
            return "SAM_V未经过授权,无法使用";
        case SDT_NoInfoErr:
            return "该项无内容";
        }
        return "失败";
    }
    //----------------------------------------------------- 
    /* SDT类型读卡器必须支持的接口 */
    /// 打开端口,为0时表示打开上一次的端口或者默认端口 
    virtual bool OpenPort(int nPort = 0) = 0;
    /// 关闭端口 
    virtual bool ClosePort() = 0;
    /// 重置SAM V 
    virtual bool ResetSAM() = 0;
    /// 检查SAM V是否正常 
    virtual bool CheckSAM() = 0;
    /// 获取SAM V ID号 
    virtual bool GetSamID(ByteBuilder& samID) = 0;
    /// 找卡 
    virtual bool FindIDCard(ByteBuilder* pManaInfo = NULL) = 0;
    /// 选卡 
    virtual bool SelectIDCard(ByteBuilder* pManaInfo = NULL) = 0;
    /// 读取所有信息,信息的格式为原始的数据格式,不带AA AA AA 96 69数据头 
    virtual bool ReadBaseMsg(ByteBuilder* pTxtInfo, ByteBuilder* pPhotoInfo) = 0;
    /// 读取追加信息 
    virtual bool ReadAppendMsg(ByteBuilder& idAppendInfo) = 0;
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace idcard 
} // namespace device 
} // namespace zhou_yb
//--------------------------------------------------------- 
//========================================================= 