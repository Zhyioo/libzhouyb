//========================================================= 
/**@file CCID_Device.h 
 * @brief Windows和Linux PCSC CCID协议下的USB设备(IC卡)
 * 
 * @date 2011-10-17   15:39:58 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_CCID_DEVICE_H_
#define _LIBZHOUYB_CCID_DEVICE_H_
//--------------------------------------------------------- 
/* Linux和Windows下预处理宏 */
#ifdef _MSC_VER
#   pragma comment(lib, "winscard.lib")
#   include "./win32/WinHandler.h"
using zhou_yb::base_device::env_win32::WinLastErrBehavior;
#endif

/* Linux下和Windows下都需要这个头文件 */
#include <winscard.h>

#include "../include/Base.h"
#include "../include/Container.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace base_device {
//--------------------------------------------------------- 
/// VC9中没有设备的返回码 
#ifndef SCARD_E_NO_READERS_AVAILABLE
#   define SCARD_E_NO_READERS_AVAILABLE (0x8010002EL)
#endif

/// 系统中CCID设备的最大数目(USB设备的最大数目)
#ifndef MAXIMUM_SMARTCARD_READERS
#   define MAXIMUM_SMARTCARD_READERS 128
#endif

/// EscapeCommand
#ifndef SCARD_CTL_CODE
    #define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
        ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
    )
    #define FILE_DEVICE_SMARTCARD           0x00000031
    #define METHOD_BUFFERED                 0
    #define FILE_ANY_ACCESS                 0
    #define SCARD_CTL_CODE(code) CTL_CODE(FILE_DEVICE_SMARTCARD, \
                                            (code), \
                                            METHOD_BUFFERED, \
                                            FILE_ANY_ACCESS)
#endif

/// 获取智能卡设备状态失败 
#define SCARD_STATE_FAILE  0x00000000
// 是否共享相同的智能卡服务上下文 
//#define SCARD_USING_SHARED_CONTEXT
//@}
//---------------------------------------------------------
/**
 * @brief CCID设备
 */
class CCID_Device : public ICCardDevice, public DeviceBehavior, public RefObject
{
    /// 禁用拷贝构造函数 
    UnenableObjConsturctCopyBehavior(CCID_Device);
public:
    //----------------------------------------------------- 
    /// CCID_Device.PowerOn参数配置项主键 DevReader
    static const char DevReaderKey[10];
    //-----------------------------------------------------
    /// CCID命令类型 
    enum CommandMode
    {
        /// EscapeCommand
        EscapeCommand = 0,
        /// ApduCommand
        ApduCommand   = 1
    };
    /// CCID智能卡设备错误码定义
    enum SCardErr
    {
        //@{
        /**@name
         * @brief 卡片API的错误
         */
        // 33 
        ListReadersErr = DeviceError::ENUM_MAX(ErrorCode),
        // 34
        ConnectErr,
        // 35
        EstablishContextErr,
        // 36
        TransmitErr,
        // 37
        ControlErr,
        // 38
        GetStatusChangeErr,
        // 39
        FreeMemoryErr,
        // 40
        ReleaseContextErr,
        // 41
        DisconnectErr,
        /// 取ATR信息失败  42
        GetAtrErr,
        /// 设备未上电     43
        NoPowerOnErr,

        MaxSCardErr
        //@}
    };
    //-----------------------------------------------------
    /// 通过错误码获取错误信息
    virtual const char* TransErrToString(int errCode) const 
    {
        if(errCode < DeviceError::ENUM_MAX(ErrorCode))
            return DeviceError::TransErrToString(errCode);

        switch(errCode)
        {
        case ConnectErr:
            return "SCardConnect失败";
        case DisconnectErr:
            return "SCardDisconnect失败";
        case EstablishContextErr:
            return "SCardEstablishContext失败";
        case GetAtrErr:
            return "SCard获取ATR信息失败";
        case GetStatusChangeErr:
            return "SCardGetStatusChange失败";
        case ListReadersErr:
            return "SCardListReaders失败";
        case NoPowerOnErr:
            return "SCard设备未上电";
        case FreeMemoryErr:
            return "ScardFreeMemory失败";
        case ReleaseContextErr:
            return "SCardReleaseContext失败";
        case TransmitErr:
            return "SCardTransmit失败";
        case ControlErr:
            return "SCardControl失败";
        default:
            return "SCard未知错误";
        }
    }
    /// 获取设备状态的描述信息 
    static string StateToString(DWORD state)
    {
        string statestr;
        if(state == SCARD_STATE_UNAWARE)
        {
            statestr += "SCARD_STATE_UNAWARE ";
        }
        if(state & SCARD_STATE_IGNORE)
        {
            statestr += "SCARD_STATE_IGNORE ";
        }
        if(state & SCARD_STATE_UNAVAILABLE)
        {
            statestr += "SCARD_STATE_UNAVAILABLE ";
        }
        if(state & SCARD_STATE_EMPTY)
        {
            statestr += "SCARD_STATE_EMPTY ";
        }
        if(state & SCARD_STATE_PRESENT)
        {
            statestr += "SCARD_STATE_PRESENT ";
        }
        if(state & SCARD_STATE_ATRMATCH)
        {
            statestr += "SCARD_STATE_ATRMATCH ";
        }
        if(state & SCARD_STATE_EXCLUSIVE)
        {
            statestr += "SCARD_STATE_EXCLUSIVE ";
        }
        if(state & SCARD_STATE_INUSE)
        {
            statestr += "SCARD_STATE_INUSE ";
        }
        if(state & SCARD_STATE_CHANGED)
        {
            statestr += "SCARD_STATE_CHANGED ";
        }
        if(state & SCARD_STATE_UNKNOWN)
        {
            statestr += "SCARD_STATE_UNKNOWN ";
        }
        if(state & SCARD_STATE_MUTE)
        {
            statestr += "SCARD_STATE_MUTE ";
        }
        return statestr;
    }
    //-----------------------------------------------------
protected:
    //-----------------------------------------------------
    #ifdef SCARD_USING_SHARED_CONTEXT
    /// 智能卡服务上下文 
    static SCARDCONTEXT _scardContext;
    /// 引用该服务的CCID_Device引用计数 
    static size_t _use_count;
    #else
    /// 智能卡服务上下文 
    SCARDCONTEXT _scardContext;
    #endif

    /// 卡设备句柄
    SCARDHANDLE _hCard;
    /// 卡类别(T0,T1)
    DWORD _activeProtocol;
    /// 是否是标准的APDU命令还是EscapeCommand
    bool _isApduMode;
    /// 字符转换 
    CharConverter _convert;

    /// 当前连接的读卡器设备名
    ByteBuilder _reader;
    //-----------------------------------------------------
    /// 发送APDU指令(只发送，不处理下次需要返回数据)
    BOOL _sendCmd(const ByteArray& apdu, ByteBuilder& recv)
    {
        /* Log Header */
        LOGGER(_log_reader());
        LOGGER(_log.WriteLine(_isApduMode ? "Send ApduCommand:" : "Send EscapeCommand:"));
        LOGGER(_log<<apdu<<endl);

        /* SCardTransmit */
        LPCSCARD_IO_REQUEST pioSendPci = 
            (_activeProtocol == SCARD_PROTOCOL_T0) ? SCARD_PCI_T0 : SCARD_PCI_T1;

        BYTE recvBuf[MAX_APDU_LEN] = {0};
        DWORD recvlen = sizeof(recvBuf);

        LONG iRet = !SCARD_S_SUCCESS;
        if(!_isApduMode)
        {
            // EscapCommandMode
            iRet = SCardControl(_hCard, SCARD_CTL_CODE(3500), 
                apdu.GetBuffer(), apdu.GetLength(), recvBuf, sizeof(recvBuf), &recvlen);
        }
        else
        {
            // ApduCommandMode
            iRet = SCardTransmit(_hCard, pioSendPci, apdu.GetBuffer(), apdu.GetLength(), NULL, recvBuf, &recvlen);
        }

        if(iRet != SCARD_S_SUCCESS)
        {
            _logErr(_isApduMode ? TransmitErr : ControlErr, iRet);
        }

        /* 发送命令成功 */
        if(iRet == SCARD_S_SUCCESS)
        {
            // 将数据拷贝到输出参数
            ByteArray recvArray(recvBuf, recvlen);
            recv.Append(recvArray);

            LOGGER(
            _log.WriteLine(_isApduMode ? "Recv ApduCommand:" : "Recv EscapeCommand:");
            _log.WriteHex(ByteArray(recvBuf, recvlen))<<endl);
        }

        return ToBOOL(iRet == SCARD_S_SUCCESS);
    }
    /**
     * @brief 获取ATR信息
     * @warning pArt指针必须有效
     */
    BOOL _getATR(ByteBuilder& atr)
    {
        /* 设置初始化的状态 */
        SCARD_READERSTATE readerState;
        readerState.szReader = _convert.to_char_t(_reader.GetString());
        readerState.dwCurrentState = SCARD_STATE_UNAWARE;

        /* SCardGetStatusChange */
        LONG iRet = SCardGetStatusChange(_scardContext, INFINITE, &readerState, 1);
        if(iRet != SCARD_S_SUCCESS)
        {
            _logErr(GetStatusChangeErr, iRet);

            return FALSE;
        }
        /* 输出ATR信息 */
        LOGGER(_log<<"ATR:");

        for(size_t i = 0;i < readerState.cbAtr; ++i)
        {
            atr += static_cast<byte>(readerState.rgbAtr[i]);
        }

        LOGGER(_log.WriteLine(atr));

        return TRUE;
    }
    /// 记录错误信息 
    void _logErr(int errCode, long lReturn)
    {
        DeviceBehavior::_logErr(errCode, _hex_num(lReturn).c_str());
#ifdef _MSC_VER
        LOGGER(_log << "描述信息:[" << lReturn << "," << WinLastErrBehavior::TransErrToString(lReturn) << "]" << endl);
#endif
    }
    /// 记录错误信息 
    virtual void _logErr(int errCode, const char* errinfo = NULL)
    {
        DeviceBehavior::_logErr(errCode, errinfo);
    }
    /// 初始化相关变量 
    inline void _init()
    {
        #ifdef SCARD_USING_SHARED_CONTEXT
        // 每个设备都增加一次引用计数 
        ++_use_count;
        #else
        _scardContext = static_cast<SCARDCONTEXT>(NULL);
        #endif

        _hCard = static_cast<SCARDHANDLE>(NULL);
        _activeProtocol = 0;

        // 默认为Apdu方式 
        _isApduMode = true;
    }

    bool _establishContext(SCARDCONTEXT& scardContext)
    {
        #ifdef SCARD_USING_SHARED_CONTEXT
        if(NULL != CCID_Device::_scardContext)
        {
            scardContext = CCID_Device::_scardContext;
        }
        else
        {
        #endif
        LONG iRet = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &scardContext);
        if(iRet != SCARD_S_SUCCESS)
        {
            _logErr(EstablishContextErr, iRet);
            return false;
        }
        #ifdef SCARD_USING_SHARED_CONTEXT
        CCID_Device::_scardContext = scardContext;
        }
        #endif

        return true;
    }
    void _releaseContext(SCARDCONTEXT& scardContext)
    {
        #ifdef SCARD_USING_SHARED_CONTEXT
        if(CCID_Device::_use_count > 0)
            return ;
        #endif

        // 释放资源 
        LONG iRet = SCardReleaseContext(scardContext);
        if(iRet != SCARD_S_SUCCESS)
        {
            _logErr(ReleaseContextErr, iRet);
        }
        scardContext = static_cast<SCARDCONTEXT>(NULL);
    }
    LOGGER(
    inline void _log_reader()
    {
        _log<<"Reader:["<<_reader.GetString()<<"]\n";
    });
    //-----------------------------------------------------
public:
    //-----------------------------------------------------
    CCID_Device() : DeviceBehavior() { _init(); }
    virtual ~CCID_Device()
    {
        #ifdef SCARD_USING_SHARED_CONTEXT
        --(CCID_Device::_use_count);
        #endif

        PowerOff();
    }
    //-----------------------------------------------------
    #ifdef SCARD_USING_SHARED_CONTEXT
    /// 返回共享的句柄 
    static const SCARDCONTEXT& SharedSCardContext()
    {
        return CCID_Device::_scardContext;
    }
    /// 重新获取智能卡服务描述句柄(如果操作中服务进程被强行结束则必须重新获取该句柄)
    static bool ReEstablishSCardContext()
    {
        if(NULL != CCID_Device::_scardContext)
        {
            SCardReleaseContext(CCID_Device::_scardContext);
            _scardContext = static_cast<SCARDCONTEXT>(NULL);
        }
        LONG iRet = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &CCID_Device::_scardContext);
        
        return (iRet == SCARD_S_SUCCESS);
    }
    #endif
    //----------------------------------------------------- 
    /**
     * @brief 获取读卡器名称列表
     *
     * 不设置为static静态函数，利用单独的设备来获取可以记录错误的相信信息
     * 和错误码 
     * 
     * @param [out] readerlist 设备列表
     * @return 设备个数
     * @retval 0 没有CCID设备
     */
    size_t EnumDevice(list<string>& readerlist)
    {
        /* Log Header */
        LOG_FUNC_NAME();

        size_t readerCount = 0;
        SCARDCONTEXT scardContext = static_cast<SCARDCONTEXT>(NULL);

        /* SCardEstablishContext */
        if(!_establishContext(scardContext))
            return _logRetValue(readerCount);

        /* SCardListReaders */
        char_t* szReaders = NULL;
        DWORD cchReaders = SCARD_AUTOALLOCATE;
        int dwI;

        LONG iRet = SCardListReaders(scardContext, NULL, reinterpret_cast<LPTSTR>(&szReaders), &cchReaders);
        char_t* szRdr = szReaders;

        switch(iRet)
        {
        // 没有读卡器 
        case SCARD_E_NO_READERS_AVAILABLE:
            _logErr(ListReadersErr, "没有读卡器");
            break;
        // 枚举读卡器列表 
        case SCARD_S_SUCCESS:
            /* 获取列表 */
            readerCount = 0;

            LOGGER(_log.WriteLine("获取到的读卡器列表:"));
            // 将获取到的列表添加到list中备用
            for(dwI = 0; dwI < MAXIMUM_SMARTCARD_READERS; dwI++)
            {
                if (0 == *szRdr)
                    break;

                ++readerCount;
                readerlist.push_back(_convert.to_char(szRdr));

                LOGGER(_log << (dwI + 1) << '.' << readerlist.back() << endl);

                szRdr += strlen_t(szRdr) + 1;
            }
            /* 释放API中分配的内存 */
            iRet = SCardFreeMemory(scardContext, szReaders);
            if(iRet != SCARD_S_SUCCESS)
            {
                _logErr(FreeMemoryErr, iRet);
            }
            szReaders = NULL;
            break;
        // 枚举错误 
        default:
            _logErr(ListReadersErr, iRet);
            break;
        }
        /* SCardReleaseContext */
        _releaseContext(scardContext);

        return _logRetValue(readerCount);
    }
    //----------------------------------------------------- 
    /**
     * @brief 获取指定设备的状态
     * @param [in] currentState 当前状态 
     * @param [in] eventState 事件状态 
     * @param [in] readerName [default:NULL] 要获取状态的设备名(如果readerName为NULL则表示获取自己的状态) 
     * @retval bool
     * @return 
     */
    bool SCardReaderState(DWORD& currentState, DWORD& eventState, const char* readerName = NULL)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        LOGGER(_log<<"Reader:<"<<_strput(readerName)<<">\n");

        /* Set SCardReaderState */
        bool bRet = false;

        size_t len = _strlen(readerName);
        if(!(len < 1 && _reader.IsEmpty()))
        {
            SCARDCONTEXT scardContext;
            if(!_establishContext(scardContext))
            {
                return _logRetValue(bRet);
            }

            SCARD_READERSTATE readerState;
            readerState.szReader = _convert.to_char_t((len < 1 ? _reader.GetString() : readerName));
            readerState.dwCurrentState = SCARD_STATE_UNAWARE;

            /* SCardGetStatusChange */
            LONG iRet = SCardGetStatusChange(scardContext, INFINITE, &readerState, 1);
            if(iRet == SCARD_S_SUCCESS)
            {
                currentState = readerState.dwCurrentState;
                eventState = readerState.dwEventState;

                bRet = true;

                LOGGER(
                _log.WriteLine("读卡器状态:");
                _log<<"当前:<"<<StateToString(currentState)<<">\n"
                    <<"事件:<"<<StateToString(eventState)<<">\n");
            }
            else
            {
                _logErr(GetStatusChangeErr, iRet);
            }

            _releaseContext(scardContext);
        }
        LOGGER(
        else
        {
            _log.WriteLine("读卡器名称为空,无法获取状态");
        });
        
        return _logRetValue(bRet);
    }
    //-----------------------------------------------------
    //@{
    /**@name 
     * @brief IICardDevice成员函数 
     */ 
    /**
     * @brief 给指定设备名的设备上电,如已经有设备则自动下电
     * @param [in] readerName 要上电的设备名,必须是完整的名称,支持reader,[reader:<name>]
     * @param [out] pAtr [default:NULL] ATR缓冲区指针(默认为空，表示不需要ATR信息)
     */
    virtual bool PowerOn(const char* readerName, ByteBuilder* pAtr = NULL)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        bool isArgNull = _is_empty_or_null(readerName);
        const char* reader = readerName;
        /* 找不到读卡器名称 */
        if(isArgNull && _reader.IsEmpty())
        {
            _logErr(DeviceError::ArgIsNullErr, "读卡器名称为空");
            return _logRetValue(false);
        }
        /* 需要对原来的设备重新上电 */
        if(isArgNull)
        {
            reader = _reader.GetString();
        }
        else
        {
            /* 解析读卡器名称 */
            ArgParser cfg;
            string argReaderName;
            cfg.Parse(readerName);
            // 符合[]格式
            if(cfg.GetValue(DevReaderKey, argReaderName))
            {
                reader = argReaderName.c_str();
            }
        }
        /* 已经上电则下电 */
        PowerOff();

        LOGGER(_log<<"上电:<"<<reader<<">\n");

        /* SCardEstablishContext */
        if(!_establishContext(_scardContext))
            return _logRetValue(false);

        /* 上电 SCardConnect */
        LONG iRet = SCARD_S_SUCCESS;
        CharConverter cvt;
        const char_t* reader_t = cvt.to_char_t(reader);
        if(!_isApduMode)
        {
            iRet = SCardConnect(_scardContext, reader_t, SCARD_SHARE_DIRECT,
                SCARD_PROTOCOL_UNDEFINED, &_hCard, &_activeProtocol);
        }
        else
        {
            iRet = SCardConnect(_scardContext, reader_t, SCARD_SHARE_SHARED,
                SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &_hCard, &_activeProtocol);
        }

        if(iRet != SCARD_S_SUCCESS)
        {
            /* SCardReleaseContext */
            _releaseContext(_scardContext);

            _logErr(ConnectErr, iRet);
            return _logRetValue(false);
        }
        LOGGER(
        _log<<"类型:<";
        switch(_activeProtocol)
        {
        case SCARD_PROTOCOL_T0:
            _log<<"T0>"<<endl;
            break;
        case SCARD_PROTOCOL_T1:
            _log<<"T1>"<<endl;
            break;
        default:
            _log<<"Escape>"<<endl;
            break;
        });

        /* 一定得先设置_reader才能保证能够获取到ATR */
        if(!isArgNull)
            _reader = reader;
        /* 获取ATR信息(Escape模式不需要取ATR) */
        if(_isApduMode && (NULL != pAtr))
        {
            if(_getATR(*pAtr) == FALSE)
                _logErr(GetAtrErr, "");
        }

        return _logRetValue(true);
    }
    /// 交换APDU指令
    virtual bool TransCommand(const ByteArray& send, ByteBuilder& recv)
    {
        LOG_FUNC_NAME();

        /* Send Command */
        ASSERT_FuncErrRet(HasPowerOn(), NoPowerOnErr);
        ASSERT_FuncErrRet(send.GetLength() > 0, DeviceError::ArgLengthErr);

        return _logRetValue(_sendCmd(send, recv) == TRUE);
    }
    /**
     * @brief 发送APDU指令
     * @param [in] sendBcd 发送的APDU指令对应的BCD码形式
     * @param [out] recvBcd 返回的APDU结果对应的BCD码形式
     */
    virtual bool Apdu(const ByteArray& sendBcd, ByteBuilder& recvBcd)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        // (ByteBuilder)sendBcd:内部需要修改一位用于数据操作，操作完后复位
        bool iRet = ICCardDevice::Apdu(sendBcd, recvBcd);

        return _logRetValue(iRet);
    }
    /// 返回是否已经上电
    virtual bool HasPowerOn() const
    {
        bool iRet = (_hCard != static_cast<SCARDHANDLE>(NULL)) && (!_reader.IsEmpty());

        return iRet;
    }
    /// 设备下电(多次下电不引发异常)
    virtual bool PowerOff()
    {
        return PowerOff(_isApduMode ? SCARD_UNPOWER_CARD : SCARD_LEAVE_CARD);
    }
    //@}
    //-----------------------------------------------------
    /**
     * @brief 设备下电(带卡片状态)
     * @param [in] dwDisposition 状态枚举值
     * - 可包括
     *  - "SCARD_LEAVE_CARD"
     *  - "SCARD_RESET_CARD"
     *  - "SCARD_UNPOWER_CARD"
     *  - "SCARD_EJECT_CARD"
     * .
     */ 
    bool PowerOff(DWORD dwDisposition)
    {
        /* Log Header */
        LOG_FUNC_NAME();

        LOGGER(
        _log<<"dwDisposition:<";
        switch(dwDisposition)
        {
        case SCARD_LEAVE_CARD:
            _log.WriteLine("SCARD_LEAVE_CARD [Do not do anything special]>");
            break;
        case SCARD_RESET_CARD:
            _log.WriteLine("SCARD_RESET_CARD [Reset the card]>");
            break;
        case SCARD_UNPOWER_CARD:
            _log.WriteLine("SCARD_UNPOWER_CARD [Power down the card]>");
            break;
        case SCARD_EJECT_CARD:
            _log.WriteLine("SCARD_EJECT_CARD [Eject the card]>");
            break;
        default:
            _log<<_hex_num(dwDisposition)<<","<<dwDisposition<<">\n";
            break;
        });

        LOGGER(_log_reader());

        _reader = "";
        /* 下电 SCardDisconnect */
        if(_hCard)
        {
            // 下电
            LONG iRet = SCardDisconnect(_hCard, dwDisposition);
            if(iRet != SCARD_S_SUCCESS)
            {
                _logErr(DisconnectErr, iRet);
            }
            _hCard = static_cast<SCARDHANDLE>(NULL);
        }
        if(_scardContext)
        {
            // 释放资源 
            _releaseContext(_scardContext);
        }

        return _logRetValue(true);
    }
    //-----------------------------------------------------
    /// 返回当前上电的设备名
    inline const char* GetReader() const
    {
        return _reader.GetString();
    }
    /// 设置当前的连接方式ApduCommand:EscapeCommand
    bool SetMode(CommandMode mode)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        /* EscapeCommand模式中不能交换APDU(没有卡) */
        if(!_isApduMode && mode != EscapeCommand && HasPowerOn())
        {
            _logErr(DeviceError::DevAlreadyOpenErr, "必须在断开连接后才能设置该项");
            return _logRetValue(false);
        }

        _isApduMode = (mode != EscapeCommand);
        LOGGER(_log.WriteLine(_isApduMode ? "设置ApduCommand模式" : "设置EscapeCommand模式"));

        return _logRetValue(true);
    }
    /// 获取当前设置的模式 
    inline CommandMode GetMode() const { return _isApduMode ? ApduCommand : EscapeCommand; }
    //-----------------------------------------------------
};
//---------------------------------------------------------
} // namespace base_device
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_CCID_DEVICE_H_
//=========================================================
