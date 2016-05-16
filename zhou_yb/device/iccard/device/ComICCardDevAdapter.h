//========================================================= 
/**@file ComICCardDevAdapter.h 
 * @brief 串口IC卡适配器
 * 
 * @date 2012-06-23   11:16:56 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_COMICCARDDEVADAPTER_H_
#define _LIBZHOUYB_COMICCARDDEVADAPTER_H_
//--------------------------------------------------------- 
#include "../../../include/Base.h"

#include "../../cmd_adapter/ComICCardCmdAdapter.h"
using zhou_yb::device::cmd_adapter::ComICCardCmdAdapter;
//---------------------------------------------------------
namespace zhou_yb {
namespace device {
namespace iccard {
namespace device {
//---------------------------------------------------------
/**
 * @brief 串口接触式IC卡交互适配器 
 * 
 * 适配串口发送的指令
 */
class ComContactICCardBaseDevAdapter :
    public ICCardDevice,
    public DevAdapterBehavior<IInteractiveTrans>,
    public RefObject
{
public:
    //----------------------------------------------------- 
    /// 卡槽号
    static const char SlotKey[8];
    /// 超时时间
    static const char DelayTimeKey[10];
    //----------------------------------------------------- 
protected:
    //-----------------------------------------------------
    /// 卡序号 00H - 0FH
    byte _cardNum;
    /// 发送缓冲区(减小空间分配的性能开销)
    ByteBuilder _sendBuffer;
    /// 接收缓冲区(减小空间分配的性能开销)
    ByteBuilder _recvBuffer;
    /// 卡片是否上电状态 
    bool _hasPowerOn;
    //----------------------------------------------------- 
    /// 初始化数据成员 
    inline void _init()
    {
        _hasPowerOn = false; 
        _cardNum = 0x00;
    }
    /// 返回指定的卡序号是否为有效序号,如果有效则对其进行转换 
    virtual bool _isVaildCardNumber(byte& cardNum)
    {
        // 只需要判断，不需要转换 
        return (cardNum >= 0x00 && cardNum <= 0x1F);
    }
    //-----------------------------------------------------
public:
    //-----------------------------------------------------
    ComContactICCardBaseDevAdapter() : DevAdapterBehavior<IInteractiveTrans>() { _init(); }
    virtual ~ComContactICCardBaseDevAdapter() { PowerOff(); }
    //-----------------------------------------------------
    //@{
    /**@name
     * @brief IICCardDevice成员
     */
    /**
     * @brief 接触式IC卡上电 
     * @param [in] readerName [default:NULL] 读卡器名称
     * - 支持的格式
     *  - 数字 "0","15"
     *  - 配置参数 [N:<0>][T:<400>],[CardNo:<15>][DelayTime:<1000>]
     * .
     * @param [out] pAtr [default:NULL] 上电返回的ATR信息,默认不需要ATR信息
     */
    virtual bool PowerOn(const char* readerName = NULL, ByteBuilder* pAtr = NULL)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();
        LOGGER(_log<<"读卡器参数:<"<<_strput(readerName)<<">\n");

        byte cardNo = _cardNum;
        ushort delayTime = 0;

        /* 解析配置的参数 */
        if(!_is_empty_or_null(readerName))
        {
            ArgParser cfg;
            cfg.Parse(readerName);
            // 没有配置项，检查是否为 "0", "15" 这样的格式 
            if(cfg.Count() < 1)
            {
                // 默认为0，如果以数字开头则直接转换 
                if(_is_digital(readerName[0]))
                    cardNo = _itobyte(atoi(readerName));
            }
            else
            {
                ByteBuilder tmpArgCfg;
                int tmpNum = 0;
                if(ArgConvert::FromConfig<int>(cfg, "N", tmpNum)
                    || ArgConvert::FromConfig<int>(cfg, SlotKey, tmpNum))
                {
                    cardNo = _itobyte(tmpNum);
                }
                if(ArgConvert::FromConfig<int>(cfg, "T", tmpNum)
                    || ArgConvert::FromConfig<int>(cfg, DelayTimeKey, tmpNum))
                {
                    delayTime = static_cast<ushort>(tmpNum);
                }
            }
            LOGGER(_log << "卡序号:<" << _hex(cardNo) << ">,超时:<" << delayTime << ">\n");
            /* 验证参数合法性 */
            ASSERT_FuncErrInfoRet(_isVaildCardNumber(cardNo), DeviceError::ArgRangeErr, "接触式卡卡槽号范围不正确");
        }
        _cardNum = cardNo;

        /* 已经上电则下电 */
        PowerOff();

        /* 设置为未上电状态 */
        _hasPowerOn = false;
        /* 在第一次使用时才分配空间(不将代码放到构造里面) */
        if(_sendBuffer.GetSize() < 1)
        {
            _sendBuffer.Resize(DEV_BUFFER_SIZE);
            _recvBuffer.Resize(DEV_BUFFER_SIZE);
        }
        /* 上电 */
        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromAscii("32 22", _sendBuffer);
        ByteConvert::FromObject(delayTime, _sendBuffer);
        _sendBuffer += _cardNum;
        /* 交换命令 */
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);
        // 取ATR
        if(NULL != pAtr)
        {
            pAtr->Append(_recvBuffer);
        }

        LOGGER(
        _log.WriteLine("ATR:");
        _log.WriteLine(_recvBuffer));

        _hasPowerOn = true;

        return _logRetValue(true);
    }
    //-----------------------------------------------------
    /**
     * @brief 发送指令 
     */
    virtual bool TransCommand(const ByteArray& sendBcd, ByteBuilder& recvBcd)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        ASSERT_FuncErrRet(!sendBcd.IsEmpty(), DeviceError::ArgIsNullErr);
        ASSERT_FuncErrInfoRet(HasPowerOn(), DeviceError::DevStateErr, "卡片未上电");

        LOGGER(
        _log.WriteLine("Send Apdu :");
        _log.WriteLine(sendBcd));

        _sendBuffer.Clear();

        DevCommand::FromAscii("32 26", _sendBuffer);
        _sendBuffer += _cardNum;
        _sendBuffer += sendBcd;

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(recvBcd), DeviceError::RecvErr);

        LOGGER(
        _log.WriteLine("Recv Apdu :");
        _log.WriteLine(recvBcd));

        return _logRetValue(true);
    }
    //-----------------------------------------------------
    /**
     * @brief 交换APDU
     */
    virtual bool Apdu(const ByteArray& sendBcd, ByteBuilder& recvBcd)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        /* 验证是否可操作 */
        ASSERT_Device();
        ASSERT_FuncErrInfoRet(HasPowerOn(), DeviceError::DevStateErr, "卡片未上电");

        LOGGER(
        _log.WriteLine("发送 Apdu:");
        _log.WriteLine(sendBcd));

        ASSERT_FuncErrInfoRet(ICCardDevice::Apdu(sendBcd, recvBcd), 
            DeviceError::TransceiveErr, "交换Apdu失败");

        LOGGER(
        _log.WriteLine("接收 Apdu:");
        _log.WriteLine(recvBcd));

        return _logRetValue(true);
    }
    //-----------------------------------------------------
    /**
     * @brief 接触式IC卡下电 
     */
    virtual bool PowerOff()
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();
        LOGGER(_log << "Slot:<" << _hex(_cardNum) << ">\n");
        if(HasPowerOn())
        {
            /* 下电 */
            _sendBuffer.Clear();
            _recvBuffer.Clear();
            DevCommand::FromAscii("32 23", _sendBuffer);
            _sendBuffer += _cardNum;

            ASSERT_FuncErrInfoRet(_pDev->Write(_sendBuffer), DeviceError::SendErr, "发送下电命令失败");
            ASSERT_FuncErrInfoRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr, "下电命令返回失败");

            _hasPowerOn = false;
        }

        return _logRetValue(true);
    }
    //-----------------------------------------------------
    /// 判断当前是否已经上电 
    virtual bool HasPowerOn() const
    {
        return _hasPowerOn;
    }
    //@}
    //-----------------------------------------------------
};
//---------------------------------------------------------
/**
 * @brief 串口非接触式IC卡交互适配器 
 * 
 * 适配串口发送的指令
 */
class ComContactlessICCardDevAdapter :
    public ICCardDevice,
    public DevAdapterBehavior<IInteractiveTrans>,
    public RefObject
{
protected:
    //-----------------------------------------------------
    /// 标记是否已经上电
    bool _hasPowerOn;
    /// 发送缓冲区(减小空间分配的性能开销)
    ByteBuilder _sendBuffer;
    /// 接收缓冲区(减小空间分配的性能开销)
    ByteBuilder _recvBuffer;
    //----------------------------------------------------- 
    /// 初始化数据成员 
    inline void _init()
    {
        _hasPowerOn = false;
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    // 非接卡槽序号(0xFF) 
    static const byte CardNumber;
    //-----------------------------------------------------
public:
    //-----------------------------------------------------
    ComContactlessICCardDevAdapter() : DevAdapterBehavior<IInteractiveTrans>() { _init(); }
    //-----------------------------------------------------
    /// 设置非接触式卡片为halt状态 
    bool SetHaltStatus(ushort haltTime = 0)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        if(!HasPowerOn())
        {
            LOGGER(_log.WriteLine("未上电,不需要下电"));
            return _logRetValue(true);
        }
        LOGGER(_log<<"Halt状态超时时间:<"<<haltTime<<">\n");

        if(_sendBuffer.GetSize() < 1)
        {
            _sendBuffer.Resize(DEV_BUFFER_SIZE);
            _recvBuffer.Resize(DEV_BUFFER_SIZE);
        }

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromAscii("32 25", _sendBuffer);
        ByteConvert::FromObject(haltTime, _sendBuffer);
        /* 交换命令 */
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        _hasPowerOn = false;

        return _logRetValue(true);
    }
    /**
     * @brief 激活非接触式IC卡 
     * @param [in] delayTime [default:0] 等待的时间(ms),默认为0
     * @param [out] pCardType [default:NULL] 返回的卡片种类 0x0A A类卡片,0x0B B类卡片(默认不需要) 
     * @param [out] pUid [default:NULL] 返回的卡片UID(默认不需要) 
     * @param [out] pAtr [default:NULL] 返回卡片的ATR(默认不需要) 
     */
    bool ActiveContactlessICCard(ushort delayTime = 0, 
        byte* pCardType = NULL, ByteBuilder* pUid = NULL, ByteBuilder* pAtr = NULL)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        /* 已经上电则下电 */
        PowerOff();
        /* 设置为未上电状态 */
        _hasPowerOn = false;

        LOGGER(_log<<"等待放卡时间:<"<<delayTime<<">\n");

        _sendBuffer.Clear();
        _recvBuffer.Clear();

        DevCommand::FromAscii("32 24", _sendBuffer);
        ByteConvert::FromObject(delayTime, _sendBuffer);
        /* 交换命令 */
        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(_recvBuffer), DeviceError::RecvErr);

        size_t len = _recvBuffer.GetLength();
        _hasPowerOn = true;

        // 只有ATR部分 
        if(NULL != pAtr)
            pAtr->Append(_recvBuffer);
        
        /* 标识格式为带UID和卡片类型的数据 卡片类型(1字节) + UID长度(1字节) + UID(x字节) + ATR长度(1字节) + ATR(x字节) */
        if(_itobyte(len) >= 3)
        {
            size_t uidlen = _recvBuffer[1];
            size_t atrlen = 0;
            LOGGER(_log<<"UID长度:<"<<uidlen<<">\n");
            // 长度符合,检查是否有ATR
            if(len > (uidlen + 3))
            {
                // ATR的标识长度 
                atrlen = _recvBuffer[uidlen + 2];
            }
            LOGGER(_log<<"ATR长度:<"<<atrlen<<">\n");
            // 格式错误 
            if(len != (uidlen + 2 + atrlen + 1))
            {
                LOGGER(_log.WriteLine("ATR数据中不包含卡片类型和UID信息"));
                return _logRetValue(true);
            }
            // 处理是否确实包含UID 
            if(NULL != pAtr)
                pAtr->RemoveTail(len);
            
            /* 输出获取到的卡信息 */
            LOGGER(
            _sendBuffer.Clear();
            _log<<"卡片类型:<"<<_hex(_recvBuffer[0])<<">\n";
            _sendBuffer.Append(_recvBuffer.SubArray(2, uidlen));
            _log.WriteLine("UID:");
            _log.WriteLine(_sendBuffer);
            _sendBuffer.Clear();
            _sendBuffer.Append(_recvBuffer.SubArray(3 + uidlen, atrlen));
            _log.WriteLine("ATR:");
            _log.WriteLine(_sendBuffer));
        
            // 取卡类型
            if(NULL != pCardType)
            {
                *pCardType = _recvBuffer[0];
            }
            // 取UID
            if(NULL != pUid)
            {
                pUid->Append(_recvBuffer.SubArray(2, uidlen));
            }
            // 取ATR
            if(NULL != pAtr)
            {
                pAtr->Append(_recvBuffer.SubArray(3 + uidlen, atrlen));
            }
        }

        return _logRetValue(true);
    }
    //-----------------------------------------------------
    //@{
    /**@name
     * @brief IICCardDevice成员
     */
    /**
     * @brief 非接触式IC卡上电 
     * @param [in] readerName [default:NULL] 读卡器名称,超时时间(ms)[T:<100>],[DelayTime:<100>]
     * @param [out] pAtr [default:NULL] 上电返回的ATR信息,默认不需要ATR信息
     */
    virtual bool PowerOn(const char* readerName = NULL, ByteBuilder* pAtr = NULL)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();
        LOGGER(_log<<"读卡器参数:<"<<_strput(readerName)<<">\n");

        ushort delayTime = 0;
        /* 解析配置参数 */
        int tmpNo = 0;
        ArgParser cfg;
        cfg.Parse(readerName);

        if(ArgConvert::FromConfig<int>(cfg, "T", tmpNo) 
            || ArgConvert::FromConfig<int>(cfg, ComContactICCardBaseDevAdapter::DelayTimeKey, tmpNo))
        {
            delayTime = static_cast<ushort>(tmpNo);
        }

        ASSERT_FuncRet(ActiveContactlessICCard(delayTime, NULL, NULL, pAtr));

        return _logRetValue(true);
    }
    //-----------------------------------------------------
    /**
     * @brief 发送指令 
     */
    virtual bool TransCommand(const ByteArray& sendBcd, ByteBuilder& recvBcd)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();
        ASSERT_FuncErr(!sendBcd.IsEmpty(), DeviceError::ArgIsNullErr);
        ASSERT_FuncErr(HasPowerOn(), ComICCardCmdAdapter::ContactlessCardNotActiveErr);

        LOGGER(
        _log.WriteLine("Send Apdu :");
        _log.WriteLine(sendBcd));

        _sendBuffer.Clear();

        DevCommand::FromAscii("32 26", _sendBuffer);
        _sendBuffer += CardNumber;
        _sendBuffer += sendBcd;

        ASSERT_FuncErrRet(_pDev->Write(_sendBuffer), DeviceError::SendErr);
        ASSERT_FuncErrRet(_pDev->Read(recvBcd), DeviceError::RecvErr);

        LOGGER(
        _log.WriteLine("Recv Apdu :");
        _log.WriteLine(recvBcd));

        return _logRetValue(true);
    }
    //-----------------------------------------------------
    /**
     * @brief 交换APDU
     */
    virtual bool Apdu(const ByteArray& sendBcd, ByteBuilder& recvBcd)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        /* 验证是否可操作 */
        ASSERT_Device();
        ASSERT_FuncErr(HasPowerOn(), ComICCardCmdAdapter::ContactlessCardNotActiveErr);

        LOGGER(
        _log.WriteLine("发送 Apdu:");
        _log.WriteLine(sendBcd));

        ASSERT_FuncErrInfoRet(ICCardDevice::Apdu(sendBcd, recvBcd), 
            DeviceError::TransceiveErr, "交换Apdu失败");

        LOGGER(
        _log.WriteLine("接收 Apdu:");
        _log.WriteLine(recvBcd));

        return _logRetValue(true);
    }
    //-----------------------------------------------------
    /**
     * @brief 非接触式IC卡下电 
     */
    virtual bool PowerOff()
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();
        if(HasPowerOn())
        {
            ASSERT_FuncRet(SetHaltStatus());
        }

        return _logRetValue(true);
    }
    //-----------------------------------------------------
    /// 判断当前是否已经上电 
    virtual bool HasPowerOn() const
    {
        return _hasPowerOn;
    }
    //@}
    //-----------------------------------------------------
};
//---------------------------------------------------------
/**
 * @brief 串口接触式IC卡适配器 
 */
 class ComContactICCardDevAdapter : public ComContactICCardBaseDevAdapter
 {
 protected:
     //----------------------------------------------------- 
     /// 直接修改验证范围 
     virtual bool _isVaildCardNumber(byte& cardNum)
     {
        // 只需要判断，不需要转换 
        return (cardNum >= 0x00 && cardNum <= 0x0F);
     }
     //----------------------------------------------------- 
 };
//---------------------------------------------------------
/**
 * @brief 串口PSAM卡适配器(除了_cardNum和接触式完全一样)
 */
class ComPsamICCardDevAdapter : public ComContactICCardBaseDevAdapter
{
protected:
    //----------------------------------------------------- 
    /// 直接修改验证范围
    virtual bool _isVaildCardNumber(byte& cardNum)
    {
        cardNum += 0x10;
        return (cardNum >= 0x10 && cardNum <= 0x1F);
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    ComPsamICCardDevAdapter() : ComContactICCardBaseDevAdapter()
    {
        _cardNum = 0x10;
    }
    //----------------------------------------------------- 
};
//---------------------------------------------------------
} // namespace device
} // namesapce iccard 
} // namespace device 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_COMICCARDDEVADAPTER_H_
//=========================================================