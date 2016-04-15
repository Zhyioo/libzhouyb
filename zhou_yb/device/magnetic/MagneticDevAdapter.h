//========================================================= 
/**@file MagneticDevAdapter.h 
 * @brief 磁条读写器的操作流程
 * 
 * @date 2011-10-17   17:44:57 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_MAGNETICDEVADAPTER_H_
#define _LIBZHOUYB_MAGNETICDEVADAPTER_H_
//--------------------------------------------------------- 
#include "../../include/Base.h"
#include "../../include/BaseDevice.h"
//---------------------------------------------------------
namespace zhou_yb {
namespace device {
namespace magnetic {
//---------------------------------------------------------
/// 磁条机的操作逻辑实现 
class MagneticDevAdapter : public DevAdapterBehavior<IInteractiveTrans>
{
public:
    //-----------------------------------------------------
    /// 磁条错误枚举
    enum MagneticErr 
    {
        // 磁道数据错误
        ModeErr = DeviceError::ENUM_MAX(ErrorCode),
        Track1Err,
        Track2Err,
        Track3Err,
        Track23Err,
        Track12Err,
        Track13Err,
        Track123Err,

        ENUM_MAX(MagneticErr)
    };
    //-----------------------------------------------------
    /// 磁条操作模式
    enum MagneticMode 
    {
        UnKnownMode = 0x00,
        Track1 = 0x01,
        Track2 = 0x02,
        Track3 = 0x04,
        Track12 = (Track1 | Track2),
        Track13 = (Track1 | Track3),
        Track23 = (Track2 | Track3),
        Track123 = (Track1 | Track2 | Track3)
    };
    //-----------------------------------------------------
    /// 二磁道BPI模式
    enum Track2BpiMode 
    {
        Bpi75 = 75,
        Bpi210 = 210
    };
    /// 磁条高低抗
    enum ImpedanceMode
    {
        /// 不进行设置
        NoneImpedance,
        /// 抵抗
        LowImpedance,
        /// 高抗
        HighImpedance,
    };
    //-----------------------------------------------------
    /// 一磁道数据最大长度
    static const size_t MaxTrack1Len/* = 79*/;
    /// 75BPI二磁道数据最大长度 
    static const size_t Max75BpiTrack2Len/* = 38*/;
    /// 210BPI二磁道数据最大长度
    static const size_t Max210BpiTrack2Len/* = 108*/;
    /// 三磁道数据最大长度
    static const size_t MaxTrack3Len/* = 108*/;
    /// 一磁道数据起始标识符
    static const byte Track1Flag/* = 0x25*/;
    /// 三磁道数据起始标识符
    static const byte Track3Flag/* = 0x41*/;
    /// 磁条数据结束符(写磁条时)
    static const byte TrackEndFlag/* = 0x3F*/;
    /// 磁条数据读取错误标识符
    static const byte TrackErrFlag/* = 0x7F*/;
    /// 设备操作回送命令错误标识符
    static const byte DevRetErrFlag/* = 0x71*/;
    //-----------------------------------------------------
    /// 获取错误码的文字描述
    virtual const char* TransErrToString(int errCode) const 
    {
        if((int)errCode < DeviceError::ENUM_MAX(ErrorCode))
            return DeviceError::TransErrToString(static_cast<int>(errCode));

        switch(errCode)
        {
        case ModeErr:
            return "磁条模式错误";
        case Track1Err:
            return "一磁道数据错误";
        case Track2Err:
            return "二磁道数据错误";
        case Track3Err:
            return "三磁道数据错误";
        case Track12Err:
            return "一二磁道数据错误";
        case Track13Err:
            return "一三磁道数据错误";
        case Track123Err:
            return "一二三磁道数据错误";
        }
        return "无法识别的错误";
    }
    //-----------------------------------------------------
    /// 获取磁条模式的文字描述
    static const char* TransModeToString(MagneticMode mode)
    {
        switch(mode)
        {
        case Track1:
            return "一磁道";
        case Track2:
            return "二磁道";
        case Track3:
            return "三磁道";
        case Track12:
            return "一二磁道";
        case Track23:
            return "二三磁道";
        case Track13:
            return "一三磁道";
        case Track123:
            return "一二三磁道";
        default:
            break;
        }
        return "错误模式";
    }
    //-----------------------------------------------------
    /**
     * @brief 构造写入磁条的数据,返回写入的模式(对应磁道如果为NULL表示不写数据,为""则写空数据)
     *
     * @param [in] tr1 一磁道数据
     * @param [in] tr2 二磁道数据
     * @param [in] tr3 三磁道数据
     * @param [out] dst 生成的磁条指令
     * @param [out] pErrMode [default:NULL] 数据错误时标识出错的磁道
     *
     * @return zhou_yb::device::magnetic::MagneticDevAdapter::MagneticMode
     */
    static MagneticMode PackWriteCmd(const char* tr1, const char* tr2,
        const char* tr3, ByteBuilder& dst, MagneticMode* pErrMode = NULL)
    {
        // 1B 74 二磁道数据 41 三磁道数据 25 一磁道数据 1D 1B 5C 
        MagneticMode mode = UnKnownMode;
        // 构造数据头 
        DevCommand::FromAscii("1B 74", dst);
        // 增加二磁道数据
        size_t len = _strlen(tr2);
        if(len > Max210BpiTrack2Len)
        {
            if(pErrMode != NULL)
                (*pErrMode) = Track2;

            return UnKnownMode;
        }
        else if(NULL != tr2 && len <= Max210BpiTrack2Len)
        {
            // 二磁道数据无标识符 
            dst += tr2;
            mode = static_cast<MagneticMode>(mode | Track2);
        }
        // 增加三磁道数据
        len = _strlen(tr3);
        if(len > MaxTrack3Len)
        {
            if(pErrMode != NULL)
                (*pErrMode) = Track3;

            return UnKnownMode;
        }
        else if(NULL != tr3 && len <= MaxTrack3Len)
        {
            // 三磁道数据标识符
            dst.Append(Track3Flag);
            dst += tr3;
            mode = static_cast<MagneticMode>(mode | Track3);
        }
        // 增加一磁道数据
        len = _strlen(tr1);
        if(len > MaxTrack1Len)
        {
            if(pErrMode != NULL)
                (*pErrMode) = Track1;

            return UnKnownMode;
        }
        else if(NULL != tr1 && len <= MaxTrack1Len)
        {
            // 一磁道数据标识符
            dst.Append(Track1Flag);
            dst += tr1;
            mode = static_cast<MagneticMode>(mode | Track1);
        }
        // 构造数据尾
        DevCommand::FromAscii("1D 1B 5C", dst);

        return mode;
    }
    /// 解包读取出来的数据,src中的数据自带模式,返回解析后的模式 
    static MagneticMode UnpackRecvCmd(const ByteArray& src,
        ByteBuilder* tr1, ByteBuilder* tr2, ByteBuilder* tr3)
    {
        // 1B 73 二磁道数据 41 三磁道数据 25 一磁道数据 3F 1C
        // 校验包头包尾
        const char MagHead[2] = { 0x1B, 0x73 };
        const char MagTail[2] = { 0x3F, 0x1C };
        bool isOkFormat = src.GetLength() > 4 &&
            StringConvert::StartWith(src, ByteArray(MagHead, sizeof(MagHead))) &&
            StringConvert::EndWith(src, ByteArray(MagTail, sizeof(MagTail)));
        MagneticMode mode = UnKnownMode;
        if(!isOkFormat) return mode;

        ByteArray tr1Data;
        ByteArray tr2Data;
        ByteArray tr3Data;
        size_t sublen = 0;

        const byte flagArr[3] = { Track3Flag, Track1Flag, TrackEndFlag };
        size_t startIndex = 2;
        // 先找二磁道信息 
        size_t trIndex = StringConvert::IndexOfAny(src, ByteArray(flagArr, sizeof(flagArr)));
        // 有二磁道信息 
        if(trIndex != SIZE_EOF && trIndex != startIndex)
        {
            sublen = trIndex - startIndex;
            tr2Data = src.SubArray(startIndex, sublen);
            if(tr2Data.GetLength() > 0 && tr2Data[0] != TrackErrFlag)
            {
                if(tr2 != NULL) (*tr2) += tr2Data;
                mode = static_cast<MagneticMode>(mode | Track2);
            }
        }
        // 找三磁道信息
        startIndex = StringConvert::IndexOf(src, flagArr[0]);
        // 有三磁道信息
        if(startIndex != SIZE_EOF)
        {
            // 跳过本身的 0x41 标识符
            trIndex = StringConvert::IndexOfAny(src, ByteArray(flagArr + 1, sizeof(flagArr) - 1));
            sublen = trIndex - startIndex - 1;
            tr3Data = src.SubArray(startIndex + 1, sublen);
            if(tr3Data.GetLength() > 0 && tr3Data[0] != TrackErrFlag)
            {
                if(tr3 != NULL) (*tr3) += tr3Data;
                mode = static_cast<MagneticMode>(mode | Track3);
            }
        }
        // 找一磁道信息
        startIndex = StringConvert::IndexOf(src, flagArr[1]);
        // 有一磁道信息
        if(startIndex != SIZE_EOF)
        {
            // 跳过前面的 0x41 0x25 标识符
            trIndex = StringConvert::IndexOf(src, TrackEndFlag);
            sublen = trIndex - startIndex - 1;
            tr1Data = src.SubArray(startIndex + 1, sublen);
            if(tr1Data.GetLength() > 0 && tr1Data[0] != TrackErrFlag)
            {
                if(tr1 != NULL) (*tr1) += tr1Data;
                mode = static_cast<MagneticMode>(mode | Track1);
            }
        }

        return mode;
    }
    /// 解析对应模式下的命令
    static bool PackReadCmd(MagneticMode mode, ByteBuilder& cmd)
    {
        bool result = true;
        switch(mode)
        {
        case Track1:
            DevCommand::FromAscii("1B 43 5D", cmd);
            break;
        case Track2:
            DevCommand::FromAscii("1B 5D", cmd);
            break;
        case Track3:
            DevCommand::FromAscii("1B 54 5D", cmd);
            break;
        case Track12:
            DevCommand::FromAscii("1B 44 5D", cmd);
            break;
        case Track23:
            DevCommand::FromAscii("1B 42 5D", cmd);
            break;
        case Track13:
            DevCommand::FromAscii("1B 45 5D", cmd);
            break;
        case Track123:
            DevCommand::FromAscii("1B 46 5D", cmd);
            break;
        default:
            result = false;
            break;
        }
        return result;
    }
    /// 按照磁条指令格式接收数据  
    static bool RecvByFormat(IInteractiveTrans& dev, ByteBuilder& emptyBuffer)
    {
        // 70,71,1C结尾
        bool bRet = false;
        do
        {
            bRet = dev.Read(emptyBuffer);
            size_t len = emptyBuffer.GetLength();
            if(bRet && len > 0)
            {
                if(emptyBuffer[len - 1] == 0x70 ||
                    emptyBuffer[len - 1] == 0x71 ||
                    emptyBuffer[len - 1] == 0x1C)
                {
                    break;
                }
            }
        } while(bRet);

        return bRet;
    }
    //----------------------------------------------------- 
protected:
    //-----------------------------------------------------
    /// 返回模式错误 
    MagneticErr _getTrackModeErr(const MagneticMode input, const MagneticMode output)
    {
        if(input == output)
            return static_cast<MagneticErr>(DeviceError::Success);

        MagneticMode mode = MagneticMode(input ^ output);
        switch(mode)
        {
        case Track1:
            return Track1Err;
        case Track2:
            return Track2Err;
        case Track3:
            return Track3Err;
        case Track12:
            return Track12Err;
        case Track13:
            return Track13Err;
        case Track23:
            return Track23Err;
        case Track123:
            return Track123Err;
        default:
            return static_cast<MagneticErr>(DeviceError::Success);
        }
    }
    /// 判断设备回送命令是否正确
    bool _checkRetCmd()
    {
        ByteBuilder retCmd(8);
        // 设备无法继续写直接返回成功
        if(_pDev->Write(DevCommand::FromAscii("1B 6A")))
        {
            if(RecvByFormat(_pDev, retCmd) && retCmd.GetLength() > 0)
            {
                bool bRet = true;
                LOGGER(
                _log.WriteLine("设备返回操作状态:");
                _log.WriteLine(retCmd));

                bRet = (retCmd[retCmd.GetLength() - 1] != DevRetErrFlag);

                return bRet;
            }
            if(IsAutoReset)
            {
                Reset(_pDev);
            }

            return false;
        }
        return true;
    }
    //-----------------------------------------------------
public:
    //-----------------------------------------------------
    MagneticDevAdapter() : DevAdapterBehavior<IInteractiveTrans>()
    {
        IsCheckRetStatus = false;
        IsAutoReset = true;
    }
    virtual ~MagneticDevAdapter()
    {
        if(IsAutoReset && !_pDev.IsNull()) Reset(_pDev);
    }
    //-----------------------------------------------------
    /// 是否在退出时自动清除状态 
    bool IsAutoReset;
    /// 是否在读磁条后检查设备返回的状态码判断磁条数据是否正确 
    bool IsCheckRetStatus;
    //----------------------------------------------------- 
    /// 读磁道信息(对应磁道数据为空则表示没有数据) 
    bool ReadTrack(MagneticMode mode, ByteBuilder* tr1, ByteBuilder* tr2, ByteBuilder* tr3)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        LOGGER(_log<<"模式:<"<<TransModeToString(mode)<<">\n");
        // 当前没有适配设备
        ASSERT_Device();
        // 构造命令
        ByteBuilder cmd(8);
        if(!PackReadCmd(mode, cmd))
        {
            LOGGER(_log.WriteLine("构造命令参数:"));
            _logErr(ModeErr, ArgConvert::ToString<int>(mode).c_str());
            return _logRetValue(false);
        }

        if(IsAutoReset)
        {
            Reset(_pDev);
        }
        // 发命令
        ASSERT_FuncErrRet(_pDev->Write(cmd), DeviceError::SendErr);
        // 收数据
        ByteBuilder recv(256);
        if(!RecvByFormat(_pDev, recv))
        {
            // 设备软复位(操作超时)
            Reset(_pDev);

            _logErr(DeviceError::WaitTimeOutErr);
            return _logRetValue(false);
        }
        // 设备返回操作失败
        if(IsCheckRetStatus && !_checkRetCmd())
        {
            _logErr(DeviceError::DevReturnErr);
            return _logRetValue(false);
        }
        // 解析数据
        MagneticMode unpackMode = UnpackRecvCmd(recv, tr1, tr2, tr3);
        // 格式错误 
        if(unpackMode == UnKnownMode)
        {
        	_logErr(DeviceError::RecvFormatErr);
        	return _logRetValue(false);
        }
        /* 如果磁道数据空则将字符串置空 */
        if(IsCheckRetStatus && unpackMode != mode)
        {
            _logErr(ModeErr, ArgConvert::ToString<int>(unpackMode).c_str());
            return _logRetValue(false);
        }
        return _logRetValue(true);
    }
    /** 
     * @brief 写磁条信息 
     * @param [in] tr1 需要写入的一磁道数据(为NULL表示不写入一磁道信息) 
     * @param [in] tr2 需要写入的二磁道数据(为NULL表示不写入二磁道信息) 
     * @param [in] tr3 需要写入的三磁道数据(为NULL表示不写入三磁道信息) 
     * 
     * @warning tr1,tr2,tr3只有在为NULL时才不写,如果传为""则会将对应磁道数据写空 
     */ 
    bool WriteTrack(const char* tr1, const char* tr2, const char* tr3)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        LOGGER(
        _log.WriteLine("写入参数:");
        _log<<"一磁道:<"<<(tr1==NULL?"NULL":tr1)<<">\n"
            <<"二磁道:<"<<(tr2==NULL?"NULL":tr2)<<">\n"
            <<"三磁道:<"<<(tr3==NULL?"NULL":tr3)<<">\n");
        // 当前没有适配设备
        ASSERT_Device();
        // 构造命令
        ByteBuilder cmd(256);
        MagneticMode errMode = UnKnownMode;
        MagneticMode mode = PackWriteCmd(tr1, tr2, tr3, cmd, &errMode);
        LOGGER(_log<<"写模式:<"<<TransModeToString(mode)<<">\n");
        ByteBuilder errInfo;
        switch(mode)
        {
        case Track1:
            errInfo.Format("一磁道数据长度错误(实际长度:<%d>,限制长度:<%d>)", strlen(tr1), MaxTrack1Len);
            break;
        case Track2:
            errInfo.Format("二磁道数据长度错误(实际长度:<%d>,限制长度:<%d>)", strlen(tr2), Max210BpiTrack2Len);
            break;
        case Track3:
            errInfo.Format("三磁道数据长度错误(实际长度:<%d>,限制长度:<%d>)", strlen(tr3), MaxTrack3Len);
            break;
        default:
            break;
        }
        if(mode == UnKnownMode)
        {
            _logErr(DeviceError::ArgRangeErr, errInfo.GetString());
            _logErr(ModeErr, "磁道数据格式错误");
            return _logRetValue(false);
        }

        if(IsAutoReset)
        {
            Reset(_pDev);
        }
        // 发命令
        ASSERT_FuncErrRet(_pDev->Write(cmd), DeviceError::SendErr);
        // 判断设备返回的状态
        if(IsCheckRetStatus && !_checkRetCmd())
        {
            // 写失败设备返回错误  
            _logErr(DeviceError::DevReturnErr);
            return _logRetValue(false);
        }
        return _logRetValue(true);
    }
    //----------------------------------------------------- 
    /// 设置BPI 
    static bool SetTrack2BPI(IInteractiveTrans& dev, Track2BpiMode mode)
    {
        // 构造命令
        bool bRet = false;
        switch(mode)
        {
        case Bpi75:
            bRet = dev.Write(DevCommand::FromAscii("1B 48"));
            break;
        case Bpi210:
            bRet = dev.Write(DevCommand::FromAscii("1B 4C"));
            break;
        default:
            break;
        }
        return bRet;
    }
    /// 设置高低抗模式
    static bool SetImpedanceMode(IInteractiveTrans& dev, ImpedanceMode mode)
    {
        bool bWrite = true;
        switch(mode)
        {
        case LowImpedance:
            bWrite = dev.Write(DevCommand::FromAscii("1B 79"));
            break;
        case HighImpedance:
            bWrite = dev.Write(DevCommand::FromAscii("1B 78"));
            break;
        default:
            break;
        }

        return bWrite;
    }
    /// 设置写电路电流(mA) 
    static bool SetTrElectric(IInteractiveTrans& dev, uint electric)
    {
        ByteBuilder cmd(8);
        DevCommand::FromAscii("1B 69", cmd);
        cmd.Format("%u", electric);
        cmd += static_cast<byte>(0x1D);
        return dev.Write(cmd);
    }
    /// 复位 
    static bool Reset(IInteractiveTrans& dev)
    {
        return dev.Write(DevCommand::FromAscii("1B 30"));
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------
} // namespace magnetic 
} // namespace device 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_MAGNETICDEVADAPTER_H_
//=========================================================
