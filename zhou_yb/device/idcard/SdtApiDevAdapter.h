//========================================================= 
/**@file SdtApiDevAdapter.h 
 * @brief 二代证模块指令集适配器 
 * 
 * 接口返回的错误码为SDT标准的错误码(不是DeviceError的枚举类型)
 * 
 * @date 2014-12-25   15:44:18 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "ISdtApi.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace device {
namespace idcard {
//--------------------------------------------------------- 
/// 二代证模块指令集封装 
class SdtApiDevAdapter : 
    public ISdtApi,
    public DevAdapterBehavior<IInteractiveTrans>,
    public RefObject
{
public:
    //----------------------------------------------------- 
    /// 前导码 AA AA AA 96 69
    static const ByteArray SDT_CMD_PREAMBLE;
    //----------------------------------------------------- 
    /// 转换错误码为文字信息 
    virtual const char* TransErrToString(int errCode) const
    {
        return ISdtApi::TransErrToString(errCode);
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /// 返回指定的状态字是否成功
    static inline bool IsSuccessSW(ISdtApi::SDT_Error err)
    {
        return (err == ISdtApi::SDT_Success) || (err == ISdtApi::SDT_FindCardSuccess);
    }
    /// 组装发送给设备的数据
    static void PackSendCmd(byte cmd, byte para, const ByteArray& data, ByteBuilder& dst)
    {
        size_t lastLen = dst.GetLength();

        dst += SDT_CMD_PREAMBLE;
        // 3为 cmd 和 para 校验码 的长度 
        ushort len = static_cast<ushort>(data.GetLength() + 3);

        ByteConvert::FromObject(len, dst);
        dst += cmd;
        dst += para;

        dst += data;

        // 5为命令开头的 AA AA AA 96 69
        lastLen += 5;
        byte chkNum = ByteConvert::XorVal(dst.SubArray(lastLen));

        dst += chkNum;
    }
    /// 按照二代证的格式收取数据 
    static ISdtApi::SDT_Error RecvByFormat(IInteractiveTrans& dev, ByteBuilder& emptyBuffer)
    {
        const size_t CmdHeaderLength = 7;
        bool bRet = false;
        // 接收前导码长度错误,尝试多次读取 
        do
        {
            bRet = dev.Read(emptyBuffer);
            if(!bRet) return ISdtApi::SDT_ReadErr;
        } while(emptyBuffer.GetLength() < CmdHeaderLength);
        
        // 前导码不正确
        if(!StringConvert::StartWith(emptyBuffer, SDT_CMD_PREAMBLE))
            return ISdtApi::SDT_InfoErr;

        // 指令返回的数据标识长度 
        size_t len = static_cast<size_t>(emptyBuffer[5]);
        len <<= BIT_OFFSET;
        len += static_cast<size_t>(emptyBuffer[6]);

        // 7字节为指令头,后续的为数据 
        size_t count = (emptyBuffer.GetLength() - CmdHeaderLength);
        size_t lastlen = 0;
        while(count < len)
        {
            lastlen = emptyBuffer.GetLength();
            if(!dev.Read(emptyBuffer))
                return ISdtApi::SDT_ReadErr;

            count += (emptyBuffer.GetLength() - lastlen);
        }

        // 如果收到的数据大于标识的长度则截断 
        if(len < count)
            emptyBuffer.RemoveTail(count - len);
        return ISdtApi::SDT_Success;
    }
    /// 解包收到的设备数据,返回设备指示的错误信息 
    static ISdtApi::SDT_Error UnpackRecvCmd(const ByteArray& recv, ByteBuilder* pDst = NULL)
    {
        ISdtApi::SDT_Error err = ISdtApi::SDT_UnknownErr;
        // 收到的数据长度不正确
        if(recv.GetLength() < 11)
            return ISdtApi::SDT_LengthErr;
        // 前导码不正确
        if(!StringConvert::StartWith(recv, SDT_CMD_PREAMBLE))
            return ISdtApi::SDT_ReadErr;
        // 取长度
        ushort len = static_cast<ushort>(recv[5]);
        len <<= BIT_OFFSET;
        len += static_cast<ushort>(recv[6]);
        // 收到的数据标识长度与实际长度不符合
        if(recv.GetLength() - 7 != len)
            return ISdtApi::SDT_LengthErr;
        // SW3
        len = recv[9];
        err = static_cast<ISdtApi::SDT_Error>(len);
        // 状态值错误
        if(!IsSuccessSW(err))
            return err;
        // 验证校验码 
        byte xorVal = ByteConvert::XorVal(recv.SubArray(5, recv.GetLength() - 6));
        if(xorVal != recv[recv.GetLength() - 1])
            return ISdtApi::SDT_CheckValueErr;
        // 取数据
        if(NULL != pDst)
        {
            pDst->Append(recv.SubArray(10, recv.GetLength() - 11));
        }

        return ISdtApi::SDT_Success;
    }
    //----------------------------------------------------- 
protected:
    //----------------------------------------------------- 
    /// 临时缓冲区 
    ByteBuilder _tmpBuffer;
    //----------------------------------------------------- 
    /// 更改设备状态 
    bool _change_status(byte cmd, byte para, const char* info, ByteBuilder* pDst = NULL)
    {
        _tmpBuffer.Clear();
        PackSendCmd(cmd, para, "", _tmpBuffer);

        ByteBuilder infoStr(64);
        infoStr.Format("发送%s命令失败", _strput(info));
        ASSERT_FuncErrInfo(_pDev->Write(_tmpBuffer), ISdtApi::SDT_TransmitErr, infoStr.GetString());
        // 把不需要使用的命令当做临时接收数据的缓冲区使用
        infoStr.Format("接收%s响应失败", _strput(info));
        _tmpBuffer.Clear();

        ISdtApi::SDT_Error err = RecvByFormat(_pDev, _tmpBuffer);
        LOGGER(_log.WriteLine("RecvByFormat:").WriteLine(_tmpBuffer));
        ASSERT_FuncErrInfo(err == ISdtApi::SDT_Success, err, infoStr.GetString());

        err = UnpackRecvCmd(_tmpBuffer, pDst);
        ASSERT_FuncErrInfo(IsSuccessSW(err), err, info);

        return true;
    }
    //----------------------------------------------------- 
    /// 复位 
    inline bool _reset()
    {
        return _change_status(0x10, 0xFF, "复位");
    }
    /// 状态检测 
    inline bool _chk_status()
    {
        return _change_status(0x11, 0xFF, "状态检测");
    }
    /// 读SAM编号 
    inline bool _read_sam(ByteBuilder& sam)
    {
        return _change_status(0x12, 0xFF, "读SAM编号", &sam);
    }
    /// 更改波特率 
    inline bool _set_baud(byte para)
    {
        return _change_status(0x60, para, "更改波特率");
    }
    /// 找卡 
    inline bool _find_card(ByteBuilder* pInfo)
    {
        return _change_status(0x20, 0x01, "找卡", pInfo);
    }
    /// 选卡 
    inline bool _select_card(ByteBuilder* pInfo)
    {
        return _change_status(0x20, 0x02, "选卡", pInfo);
    }
    /// 读基本信息  
    inline bool _read_base_info(ByteBuilder& info)
    {
        return _change_status(0x30, 0x01, "读基本信息", &info);
    }
    /// 读最新追加信息 
    inline bool _read_last_info(ByteBuilder& info)
    {
        return _change_status(0x30, 0x03, "读最新追加信息", &info);
    }
    /// 读基本信息加指纹 
    inline bool _read_info_finger(ByteBuilder& info)
    {
        return _change_status(0x30, 0x10, "读信息+指纹", &info);
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    SdtApiDevAdapter() : DevAdapterBehavior<IInteractiveTrans>() { _init(); }
    //----------------------------------------------------- 
    /// 直接发送操作指令
    bool TransCommand(byte cmd, byte para, const char* cmdMsg = NULL, ByteBuilder* info = NULL)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "Cmd:<" << _hex(cmd) << ">,Para:<" << _hex(para) << ">,Msg:<" << _strput(cmdMsg) << ">\n");
        ASSERT_Device();
        return _logRetValue(_change_status(cmd, para, cmdMsg, info));
    }
    //----------------------------------------------------- 
    /// 打开端口,为0时表示打开上一次的端口或者默认端口 
    virtual bool OpenPort(int = 0)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        /* 不需要实现,由适配的底层设备去操作 */

        return _logRetValue(true);
    }
    /// 检查SAM V是否正常 
    virtual bool CheckSAM()
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        // 检查状态
        return _logRetValue(_chk_status());
    }
    /// 获取SAM V ID号 
    virtual bool GetSamID(ByteBuilder& samID)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        bool bRet = _read_sam(samID);

        LOGGER(
        if(bRet)
        {
            _log.WriteLine("SAM卡编号:");
            _log.WriteLine(samID);
        });

        return _logRetValue(bRet);
    }
    /// 找卡 
    virtual bool FindIDCard(ByteBuilder* pManaInfo = NULL)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        return _logRetValue(_find_card(pManaInfo));
    }
    /// 选卡 
    virtual bool SelectIDCard(ByteBuilder* pManaInfo = NULL)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        return _logRetValue(_select_card(pManaInfo));
    }
    /// 读取所有信息,信息的格式为原始的数据格式 
    virtual bool ReadBaseMsg(ByteBuilder* pTxtInfo, ByteBuilder* pPhotoInfo)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        ByteBuilder baseInfo(1024);
        // 获取出来的数据为 0100 0400 + 字段数据 
        bool bRet = _read_base_info(baseInfo);
        if(bRet)
        {
            size_t txtLen = 0;
            size_t photoLen = 0;

            txtLen = baseInfo[0] << BIT_OFFSET;
            txtLen += baseInfo[1];

            photoLen = baseInfo[2] << BIT_OFFSET;
            photoLen += baseInfo[3];

            if(pTxtInfo != NULL)
                pTxtInfo->Append(baseInfo.SubArray(4, txtLen));
            if(pPhotoInfo != NULL)
                pPhotoInfo->Append(baseInfo.SubArray(4 + txtLen, photoLen));
        }

        return _logRetValue(bRet);
    }
    /// 读取追加信息 
    virtual bool ReadAppendMsg(ByteBuilder& idAppendInfo)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        return _logRetValue(_read_last_info(idAppendInfo));
    }
    /// 读取信息+指纹 
    virtual bool ReadMsgFinger(ByteBuilder* pTxtInfo, ByteBuilder* pPhotoInfo, ByteBuilder* pFingerInfo)
    {
        LOG_FUNC_NAME();

        ByteBuilder baseInfo(1024);
        // 获取出来的数据为 0100 0400 + 字段数据 
        bool bRet = _read_info_finger(baseInfo);
        if(bRet)
        {
            size_t txtLen = 0;
            size_t photoLen = 0;
            size_t fingerLen = 0;

            txtLen = baseInfo[0] << BIT_OFFSET;
            txtLen += baseInfo[1];

            photoLen = baseInfo[2] << BIT_OFFSET;
            photoLen += baseInfo[3];

            fingerLen = baseInfo[4] << BIT_OFFSET;
            fingerLen += baseInfo[5];

            if(pTxtInfo != NULL)
                pTxtInfo->Append(baseInfo.SubArray(6, txtLen));
            if(pPhotoInfo != NULL)
                pPhotoInfo->Append(baseInfo.SubArray(6 + txtLen, photoLen));
            if(pFingerInfo != NULL)
                pFingerInfo->Append(baseInfo.SubArray(6 + txtLen + photoLen, fingerLen));
        }

        return _logRetValue(bRet);
    }
    /// 重置SAM V 
    virtual bool ResetSAM()
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        return _logRetValue(_reset());
    }
    /// 关闭端口 
    virtual bool ClosePort()
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        /* 不需要实现,由适配的底层设备去操作 */

        return _logRetValue(true);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace idcard 
} // namespace device 
} // namespace zhou_yb
//--------------------------------------------------------- 

//========================================================= 
