//========================================================= 
/**@file CCID_StorageCardDevAdapter.h 
 * @brief CCID设备存储卡 
 * 
 * @date 2013-08-29   21:52:13 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../../include/Base.h"

#include "../base/ICCardLibrary.h"
using zhou_yb::device::iccard::base::ICCardLibrary;

#include "../base/ICCardAppAdapter.h"
using zhou_yb::device::iccard::base::ICCardAppAdapter;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace device {
namespace iccard {
namespace device {
//--------------------------------------------------------- 
/// CCID 存储卡指令集 
class CCID_ContactStorageCardDevAdapter : public ICCardAppAdapter
{
public:
    //----------------------------------------------------- 
    /// 接触式卡片类型定义 
    enum CardType
    {
        /// 自动识别T0,T1的卡 
        Card_Auto_T0_T1 = 0x00,
        /// AT24C01/02/04/08/16 卡 
        Card_AT24C01_02_04_08_16 = 0x01,
        /// AT24C32/AT24C64
        Card_AT24C32_64 = 0x02,
        /// AT88SC153
        Card_AT88SC153 = 0x03,
        /// AT88SC1608
        Card_AT88SC1608 = 0x04,
        /// SLE4418/SLE4428/SLE5518/SLE5528
        Card_SLE4418_4428_5518_5528 = 0x05,
        /// SLE4432/SLE4442/SLE5532/SLE5542
        Card_SLE4432_4442_5532_5542 = 0x06,
        /// SLE4406/4436/5536
        Card_SLE4406_4436_5536 = 0x07,
        /// SLE4404
        Card_SLE4404 = 0x08,
        /// AT88SC102
        Card_AT88SC101_102_1003 = 0x09,
        /// AT88SC1604
        Card_AT88SC1604 = 0x0A
    };
    //----------------------------------------------------- 
    /// 设备页大小 
    enum PageSize
    {
        SIZE_8   = 0x03,
        SIZE_16  = 0x04,
        SIZE_32  = 0x05,
        SIZE_64  = 0x06,
        SIZE_128 = 0x07
    };
    //----------------------------------------------------- 
    /// 区域标识 
    enum ZoneType
    {
        Zone0 = 0x00,
        Zone1 = 0x01,
        Zone2 = 0x02,
        Zone3 = 0x03,
        /// 熔丝 
        Fuse = 0x04
    };
    //----------------------------------------------------- 
    /// 密钥类型 
    enum KeyType
    {
        WriteKey    = 0x00,
        SecurityKey = 0x01,
        ReadKey     = 0x02
    };
    //----------------------------------------------------- 
protected:
    //----------------------------------------------------- 
    /// 发送缓冲区 
    ByteBuilder _sendBuff;
    /// 接收缓冲区 
    ByteBuilder _recvBuff;
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /**
     * @brief 选择卡类型 
     * @param [in] cardType 卡片类型 
     */ 
    bool SelectCardType(uint cardType)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log<<"卡类型:<"<<_hex(_itobyte(cardType)));

        _sendBuff.Clear();
        _recvBuff.Clear();
        
        DevCommand::FromAscii("FF A4 00 00 01", _sendBuff);
        _sendBuff += static_cast<byte>(cardType);
        
        ASSERT_FuncRet(_apdu(_sendBuff, _recvBuff));

        return _logRetValue(true);
    }
    /**
     * @brief 设置读卡器读取的页大小  
     * @param [in] size 需要设置的页大小枚举值 
     */ 
    bool SetPageSize(PageSize size)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(
        _log<<"页面大小:<"<<(0x01<<static_cast<size_t>(size))<<","<<_hex(_itobyte(size))<<">\n");

        _sendBuff.Clear();
        _recvBuff.Clear();

        DevCommand::FromAscii("FF 01 00 00 01", _sendBuff);
        _sendBuff += static_cast<byte>(size);

        ASSERT_FuncRet(_apdu(_sendBuff, _recvBuff));

        return _logRetValue(true);
    }
    /**
     * @brief 读存储卡   
     * @param [in] adr 读取的起始地址 
     * @param [in] len 需要读取的数据长度 
     * @param [out] data 读取到的数据 
     */ 
    bool ReadMemory(ushort adr, byte len, ByteBuilder& data)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log<<"Adr:<"<<_hex_num(adr)<<">,Len:<"<<_hex_num(len)<<">\n");

        _sendBuff.Clear();
        _recvBuff.Clear();

        DevCommand::FromAscii("FF B0", _sendBuff);
        ByteConvert::FromObject(adr, _sendBuff);
        _sendBuff += len;

        size_t lastlen = data.GetLength();
        bool bRet = _apdu(_sendBuff, data);
        ASSERT_FuncErr(bRet, DeviceError::TransceiveErr);
        
        // 读取失败删除读取到的垃圾数据 
        if(!bRet)
        {
            data.RemoveTail(data.GetLength() - lastlen);
        }
        LOGGER(
        else
        {
            _log.WriteLine("读取到数据:");
            _log.WriteHex(data.SubArray(lastlen))<<endl;
        })
        return _logRetValue(bRet);
    }
    /**
     * @brief 写存储卡  
     * @param [in] adr 需要写入的数据起始地址 
     * @param [in] data 需要写入的数据 
     */ 
    bool WriteMemory(ushort adr, const ByteArray& data)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(
        _log<<"Adr:<"<<_hex_num(adr)<<">,数据:("<<data.GetLength()<<")\n"
            <<data<<endl);

        ASSERT_FuncErr(data.GetLength() <= static_cast<byte>(-1), DeviceError::ArgLengthErr);

        _sendBuff.Clear();
        _recvBuff.Clear();

        byte len = _itobyte(data.GetLength());
        DevCommand::FromAscii("FF D0", _sendBuff);
        ByteConvert::FromObject(adr, _sendBuff);
        _sendBuff += len;
        _sendBuff.Append(data.SubArray(0, len));

        ASSERT_FuncRet(_apdu(_sendBuff, _recvBuff));

        return _logRetValue(true);
    }
    /**
     * @brief 擦除存储卡 
     * @param [in] adr 待擦除的地址  
     * @param [in] len 待擦除的长度
     */ 
    bool EraseMemory(ushort adr, byte len)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log<<"Adr:<"<<_hex(adr)<<">,Len:<"<<_hex(len)<<">\n");

        _sendBuff.Clear();
        _recvBuff.Clear();

        DevCommand::FromAscii("FF D2", _sendBuff);
        ByteConvert::FromObject(adr, _sendBuff);
        _sendBuff += len;

        ASSERT_FuncRet(_apdu(_sendBuff, _recvBuff));

        return _logRetValue(true);
    }
    /**
     * @brief 读AT88SC153卡  
     * @param [in] zone 需要读取的数据区(Zone枚举值) 
     * @param [in] adr 读取数据的起始地址 
     * @param [in] len 需要读取的数据长度 
     * @param [out] data 读取到的数据 
     */ 
    bool ReadMemory(ZoneType zone, byte adr, byte len, ByteBuilder& data)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(
        _log<<"Zone:<";
        switch(zone)
        {
        case Zone0:
        case Zone1:
        case Zone2:
        case Zone3:
            _log<<static_cast<int>(zone)<<">\n";
            break;
        case Fuse:
            _log<<"熔丝>\n";
            break;
        }
        _log<<"Adr:<"<<_hex(adr)<<">,Len:<"<<static_cast<int>(len)<<">\n");

        _sendBuff.Clear();
        _recvBuff.Clear();

        _sendBuff += static_cast<byte>(0x0FF);
        // B0 B1 B2 B3 B4
        _sendBuff += static_cast<byte>((zone & 0x0F) | 0x0B0);
        _sendBuff += static_cast<byte>(0x00);
        _sendBuff += adr;
        _sendBuff += len;

        ASSERT_FuncRet(_apdu(_sendBuff, _recvBuff));
        LOGGER(_log.WriteLine("数据:").WriteLine(_recvBuff));

        data += _recvBuff;

        return _logRetValue(true);
    }
    /**
     * @brief 写AT88SC153卡 
     * @param [in] zone 需要写入的数据区
     * @param [in] adr 写入数据的起始地址 
     * @param [in] data 需要写入的数据 
     */ 
    bool WriteMemory(ZoneType zone, byte adr, const ByteArray& data)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(
        _log<<"Zone:<";
        switch(zone)
        {
        case Zone0:
        case Zone1:
        case Zone2:
        case Zone3:
            _log<<(int)(zone)<<">\n";
            break;
        case Fuse:
            _log<<"熔丝>\n";
            break;
        }
        _log<<"Adr:<"<<_hex(adr)<<">\nData:\n";
        _log.WriteLine(data));

        byte len = _itobyte(data.GetLength());

        _sendBuff.Clear();
        _recvBuff.Clear();

        _sendBuff += static_cast<byte>(0x0FF);
        _sendBuff += static_cast<byte>((zone & 0x0F) | 0x0D0);
        _sendBuff += static_cast<byte>(0x00);
        _sendBuff += static_cast<byte>(adr);
        _sendBuff += len;
        _sendBuff.Append(data.SubArray(0, len));

        ASSERT_FuncRet(_apdu(_sendBuff, _recvBuff));

        return _logRetValue(true);
    }
    /**
     * @brief 验证密码
     *
     * - 接口说明:
     *  - AT88SC153卡  Key=3字节 
     *  - SLE4428/5528 Key=2字节 KeyType==WriteKey
     *  - SLE4442/5542 Key=3字节 KeyType==WriteKey
     * .
     * @param [in] key 密钥 
     * @param [in] keyType [default:WriteKey(0x00)] 密钥类型(枚举值) 
     * @param [out] pCount [default:NULL] 剩余尝试次数 
     */ 
    bool VerifyKey(const ByteArray& key, KeyType keyType = WriteKey, byte* pCount = NULL)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(
        _log<<"密钥类型:<";
        switch(keyType)
        {
        case ReadKey:
            _log<<"读";
            break;
        case WriteKey:
            _log<<"写";
            break;
        case SecurityKey:
            _log<<"安全";
            break;
        }
        _log<<">,密钥:<"<<key<<">\n");

        _sendBuff.Clear();
        _recvBuff.Clear();

        DevCommand::FromAscii("FF 20 00", _sendBuff);
        _sendBuff += static_cast<byte>(keyType);

        byte len = _itobyte(key.GetLength());
        _sendBuff += len;
        _sendBuff.Append(key.SubArray(0, len));
        
        ushort sw = _apdu_s(_sendBuff, _recvBuff);
        ASSERT_FuncErrRet(sw != ICCardLibrary::UnValidSW, DeviceError::TransceiveErr);

        LOGGER(_log<<"SW:<"<<_hex(sw)<<">\n");
        // 判断剩余尝试次数 
        byte count = 0;
        if((sw & ICCardLibrary::SuccessSW) == ICCardLibrary::SuccessSW)
        {
            count = _itobyte(sw & 0x090FF);
            if(pCount != NULL)
            {
                *pCount = count;
            }
        }

        return _logRetValue(count >= 0x0FF);
    }
    /**
     * @brief 校验访问密码 
     * - 卡片参数说明: 
     *  - AT88SC102 带错误计数 Adr=0x080A Key=2字节 
     *  - SLE4428/5528 带错误计数 Adr=0x0000 Key=2字节 
     *  - SLE4442/5542 带错误计数 Adr=0x0000 Key=3字节 
     * .
     * @param [in] adr 密钥地址 
     * @param [in] key 需要校验的密钥 
     * @param [in] hasErrCnt 是否带错误计数 
     */ 
    bool VerifyAssertKey(ushort adr, const ByteArray& key, bool hasErrCnt = true)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(
        _log<<"带错误计数:<"<<hasErrCnt<<">\n"
            <<"密钥:<"<<key<<">\n"
            <<"Adr:<"<<_hex(adr)<<">\n");

        _sendBuff.Clear();
        _recvBuff.Clear();

        _sendBuff += static_cast<byte>(0x0FF);
        _sendBuff += hasErrCnt ? 0x20 : 0x21;
        ByteConvert::FromObject(adr, _sendBuff);

        byte len = _itobyte(key.GetLength());
        _sendBuff += len;
        _sendBuff.Append(key.SubArray(0, len));

        ASSERT_FuncRet(_apdu(_sendBuff, _recvBuff));

        return _logRetValue(true);
    }
    /**
     * @brief 更改卡片密码 
     * @warning 需要校验老密码后才能修改 
     * @param [in] key 新的密钥 
     */ 
    bool ChangeKey(const ByteArray& key)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log.WriteLine("Key:").WriteLine(key));

        _sendBuff.Clear();
        _recvBuff.Clear();

        DevCommand::FromAscii("FF D2 00 01", _sendBuff);

        byte len = _itobyte(key.GetLength());
        _sendBuff += len;
        _sendBuff.Append(key.SubArray(0, len));

        ASSERT_FuncRet(_apdu(_sendBuff, _recvBuff));

        return _logRetValue(true);
    }
    /**
     * @brief 初始化认证  
     * @param [in] authData_8 8字节初始化数据 
     */ 
    bool InitAuth(const ByteArray& authData_8)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(
        _log<<"初始化,长度:<"<<authData_8.GetLength()<<">,数据:";
        _log.WriteLine(authData_8));

        _sendBuff.Clear();
        _recvBuff.Clear();

        DevCommand::FromAscii("FF 84 00 00", _sendBuff);

        byte len = _itobyte(authData_8.GetLength());
        _sendBuff += len;
        _sendBuff.Append(authData_8.SubArray(0, len));

        ASSERT_FuncRet(_apdu(_sendBuff, _recvBuff));

        return _logRetValue(true);
    }
    /**
     * @brief 认证  
     * @param [in] authData_8 8字节认证数据 
     */ 
    bool Auth(const ByteArray& authData_8)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(
        _log<<"认证,长度:<"<<authData_8.GetLength()<<">,数据:";
        _log.WriteLine(authData_8));

        _sendBuff.Clear();
        _recvBuff.Clear();

        DevCommand::FromAscii("FF 82 00 00", _sendBuff);
        byte len = _itobyte(authData_8.GetLength());
        _sendBuff += len;
        _sendBuff.Append(authData_8.SubArray(0, len));

        ASSERT_FuncRet(_apdu(_sendBuff, _recvBuff));

        return _logRetValue(true);
    }
    /**
     * @brief 读错误计数器(SLE4428和SLE5528) 
     * @param [out] errCnt 获取到的错误计数(FF:验证成功,00:验证失败,XX:错误计数) 
     * @param [in] len 错误计数器数据长度 
     * @param [out] pDummy_X [default:NULL] 卡片返回的虚拟数据 
     */ 
    bool ReadErrCount(byte& errCnt, byte len, ByteBuilder* pDummy_X = NULL)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        _sendBuff.Clear();
        _recvBuff.Clear();

        DevCommand::FromAscii("FF B1 00 00", _sendBuff);
        _sendBuff += len;
        ASSERT_FuncRet(_apdu(_sendBuff, _recvBuff));
        ASSERT_FuncErrRet(_recvBuff.GetLength() > 0, DeviceError::RecvFormatErr);

        errCnt = _recvBuff[0];
        if(pDummy_X != NULL)
        {
            _recvBuff.RemoveFront();
            (*pDummy_X) += _recvBuff;
        }

        return _logRetValue(true);
    }
    /**
     * @brief 读保护位  
     * @param [in] adr 需要读取的保护位地址 
     * @param [in] len 需要读取的长度 
     * @param [out] data 读取到的保护位数据 
     */ 
    bool ReadProtectBit(ushort adr, byte len, ByteBuilder& data)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log<<"Adr:<"<<_hex_num(adr)<<">,Len:<"<<_hex_num(len)<<">\n");

        _sendBuff.Clear();
        _recvBuff.Clear();

        DevCommand::FromAscii("FF B2", _sendBuff);
        ByteConvert::FromObject(adr, _sendBuff);
        _sendBuff += len;

        size_t lastlen = data.GetLength();
        bool bRet = _apdu(_sendBuff, data);
        ASSERT_FuncErr(bRet, DeviceError::TransceiveErr);
        
        // 读取失败删除读取到的垃圾数据 
        if(!bRet)
        {
            data.RemoveTail(data.GetLength() - lastlen);
        }
        LOGGER(
        else
        {
            _log.WriteLine("读取到数据:");
            _log.WriteHex(data.SubArray(lastlen))<<endl;
        })
        return _logRetValue(bRet);
    }
    /**
     * @brief 写保护位  
     * @param [in] adr 需要写入的数据起始地址 
     * @param [in] data 需要写入的数据 
     */ 
    bool WriteProtectBit(ushort adr, const ByteArray& data)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(
        _log<<"Adr:<"<<_hex_num(adr)<<">,数据:("<<data.GetLength()<<")\n"
            <<data<<endl);

        ASSERT_FuncErr(data.GetLength() <= static_cast<byte>(-1), DeviceError::ArgLengthErr);

        _sendBuff.Clear();
        _recvBuff.Clear();

        DevCommand::FromAscii("FF D1", _sendBuff);
        ByteConvert::FromObject(adr, _sendBuff);

        byte len = _itobyte(data.GetLength());
        _sendBuff += len;
        _sendBuff.Append(data.SubArray(0, len));

        ASSERT_FuncRet(_apdu(_sendBuff, _recvBuff));

        return _logRetValue(true);
    }
    /**
     * @brief 写熔丝位   
     * @param [in] adr 烧录的地址 
     */ 
    bool WriteFuse(ushort adr)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log<<"Adr:<"<<_hex(adr)<<">\n");

        _sendBuff.Clear();
        _recvBuff.Clear();

        DevCommand::FromAscii("FF 05 00 00 04", _sendBuff);
        ByteConvert::FromObject(adr, _sendBuff);
        DevCommand::FromAscii("01 01", _sendBuff);

        ASSERT_FuncRet(_apdu(_sendBuff, _recvBuff));

        return _logRetValue(true);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/**
 * @brief 非接触式存储卡M1卡操作逻辑  
 */ 
class CCID_Mifare1DevAdapter : public ICCardAppAdapter
{
protected:
    //----------------------------------------------------- 
    /// 发送缓冲区 
    ByteBuilder _sendBuff;
    /// 接收缓冲区 
    ByteBuilder _recvBuff;
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /**
     * @brief 装载认证密钥 
     * @param [in] key 用于认证的密钥(6字节)
     * @param [in] keyNumber [default:0x00] 密钥的序号(设备可以存放一组密钥,Default:00)
     * @param [in] keyStructure [default:0x00] 密钥存放位置(00:memory,other:保留,Default:00)
     */ 
    bool DownloadAuthKey(const ByteArray& key, byte keyNumber = 0x00, byte keyStructure = 0x00)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(
        _log<<"密钥:<"<<key<<">,密钥号:<"<<_hex(keyNumber)<<">,密钥位置:<"
            <<(keyStructure == 0x00 ? "Memory" : "Reserved")<<">\n");

        ASSERT_FuncErrInfo(key.GetLength() == 6, DeviceError::ArgLengthErr, "密钥长度不正确");

        _sendBuff.Clear();
        _recvBuff.Clear();
        
        DevCommand::FromAscii("FF 82 00 00 06", _sendBuff);
        _sendBuff += key;

        _sendBuff[ICCardLibrary::P1_INDEX] = keyStructure;
        _sendBuff[ICCardLibrary::P2_INDEX] = keyNumber;

        ASSERT_FuncRet(_apdu(_sendBuff, _recvBuff));

        return _logRetValue(true);
    }
    /**
     * @brief 认证块  
     * @param [in] blockIndex 需要认证的块号 
     * @param [in] pinType 密钥类型(0x60或0x61)
     * @param [in] keyNumber [default:0x00] 密钥序号(Default:00) 
     */ 
    bool AuthBlock(byte blockIndex, byte pinType, byte keyNumber = 0x00)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(
        _log<<"块号:<"<<static_cast<uint>(blockIndex)<<">,密钥类型:<"
            <<_hex(pinType)<<">,密钥号:<"<<_hex(keyNumber)<<">\n");

        _sendBuff.Clear();
        _recvBuff.Clear();

        DevCommand::FromAscii("FF 88 00", _sendBuff);

        _sendBuff += blockIndex;
        _sendBuff += pinType;
        _sendBuff += keyNumber;

        ASSERT_FuncRet(_apdu(_sendBuff, _recvBuff));
        return _logRetValue(true);
    }
    /**
     * @brief 读块 
     * @param [in] blockIndex 块号 
     * @param [out] recvBuff 读取到的数据 
     * @param [in] readlen [default:16(0x00)] 读取的长度(Default:0x10 16字节)
     */ 
    bool ReadBlock(byte blockIndex, ByteBuilder& recvBuff, byte readlen = 0x10)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log<<"块号:<"<<(uint)blockIndex<<">,读取长度:<"<<_hex(readlen)<<">\n");

        _sendBuff.Clear();
        _recvBuff.Clear();

        DevCommand::FromAscii("FF B0 00", _sendBuff);
        _sendBuff += blockIndex;
        _sendBuff += readlen;

        ASSERT_FuncRet(_apdu(_sendBuff, _recvBuff));
        LOGGER(_log.WriteLine("读到数据:").WriteLine(_recvBuff)<<endl);
        recvBuff += _recvBuff;

        return _logRetValue(true);
    }
    /**
     * @brief 写块 
     * @param [in] blockIndex 块号 
     * @param [in] data 需要写入的数据 
     */ 
    bool WriteBlock(byte blockIndex, const ByteArray& data)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log<<"块号:<"<<static_cast<uint>(blockIndex)<<">,数据:\n"<<data<<endl);

        _sendBuff.Clear();
        _recvBuff.Clear();

        byte len = _itobyte(data.GetLength());

        DevCommand::FromAscii("FF D6 00", _sendBuff);
        _sendBuff += blockIndex;
        _sendBuff += len;
        _sendBuff.Append(data.SubArray(0, len));

        ASSERT_FuncRet(_apdu(_sendBuff, _recvBuff));
        return _logRetValue(true);
    }
    /**
     * @brief 初始化值块  
     * @param [in] blockIndex 块号 
     * @param [in] blockVal 值块(32位4字节) 
     */ 
    bool InitValueBlock(byte blockIndex, int blockVal)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log<<"值块号:<"<<static_cast<uint>(blockIndex)<<">,值:<"<<blockVal<<">\n");

        _sendBuff.Clear();
        _recvBuff.Clear();

        DevCommand::FromAscii("FF D7 00 00 05 00", _sendBuff);
        _sendBuff[ICCardLibrary::P2_INDEX] = blockIndex;
        ByteConvert::FromObject(blockVal, _sendBuff);

        ASSERT_FuncRet(_apdu(_sendBuff, _recvBuff));
        return _logRetValue(true);
    }
    /**
     * @brief 加值块 
     * @param [in] blockIndex 值块号 
     * @param [in] val 值 
     */ 
    bool IncreaseValueBlock(byte blockIndex, int val)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log<<"块号:<"<<static_cast<uint>(blockIndex)<<">,值:<"<<val<<">\n");

        _sendBuff.Clear();
        _recvBuff.Clear();

        DevCommand::FromAscii("FF D7 00 00 05 01", _sendBuff);
        _sendBuff[ICCardLibrary::P2_INDEX] = blockIndex;
        ByteConvert::FromObject(val, _sendBuff);

        ASSERT_FuncRet(_apdu(_sendBuff, _recvBuff));
        return _logRetValue(true);
    }
    /**
     * @brief 减值块  
     * @param [in] blockIndex 值块号 
     * @param [in] val 值 
     */ 
    bool DecreaseValueBlock(byte blockIndex, int val)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log<<"块号:<"<<static_cast<uint>(blockIndex)<<">,值:<"<<val<<">\n");

        _sendBuff.Clear();
        _recvBuff.Clear();

        DevCommand::FromAscii("FF D7 00 00 05 02", _sendBuff);
        _sendBuff[ICCardLibrary::P2_INDEX] = blockIndex;
        ByteConvert::FromObject(val, _sendBuff);

        ASSERT_FuncRet(_apdu(_sendBuff, _recvBuff));
        return _logRetValue(true);
    }
    /**
     * @brief 值块拷贝 
     * @param [in] srcBlockIndex 拷贝的源值块号 
     * @param [in] dstBlockIndex 拷贝到的值块号 
     */ 
    bool BlockValCopyTo(byte srcBlockIndex, byte dstBlockIndex)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log<<"源块号:<"<<static_cast<uint>(srcBlockIndex)<<">,目的块号:<"<<static_cast<uint>(dstBlockIndex)<<">\n");

        _sendBuff.Clear();
        _recvBuff.Clear();

        DevCommand::FromAscii("FF D7 00 00 05", _sendBuff);
        _sendBuff[ICCardLibrary::P2_INDEX] = srcBlockIndex;
        _sendBuff += dstBlockIndex;

        ASSERT_FuncRet(_apdu(_sendBuff, _recvBuff));
        return _logRetValue(true);
    }
    /**
     * @brief 读值块操作 
     * @param [in] blockIndex 值块号 
     * @param [out] val 值读取到的值 
     */ 
    bool ReadValueBlock(byte blockIndex, int& val)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log<<"块号:<"<<static_cast<uint>(blockIndex)<<">\n");

        _sendBuff.Clear();
        _recvBuff.Clear();

        DevCommand::FromAscii("FF B1 00 00 04", _sendBuff);
        _sendBuff[ICCardLibrary::P2_INDEX] = blockIndex;

        ASSERT_FuncRet(_apdu(_sendBuff, _recvBuff));

        ByteConvert::ToObject(_recvBuff, val);
        return _logRetValue(true);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace device
} // namespace iccard 
} // namespace device 
} // namespace zhou_yb
//========================================================= 