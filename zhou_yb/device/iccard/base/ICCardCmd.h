//========================================================= 
/**@file ICCardCmd.h 
 * @brief IC卡基本指令
 * 
 * @date 2012-8-15   21:40:31 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_ICCARDCMD_H_
#define _LIBZHOUYB_ICCARDCMD_H_
//--------------------------------------------------------- 
#include "../../../include/Base.h"
#include "ICCardLibrary.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace device {
namespace iccard {
namespace base {
//--------------------------------------------------------- 
/// PIN码格式化工具 
class PinFormater
{
protected:
    PinFormater() {}
public:
    /**
     * @brief 将字符串的PIN码转换为16进制的PIN码 
     * 
     * @param [in] pinAscii 输入的ASCII码格式PIN码  
     * @param [out] pin 转换后的PIN码  
     * 
     * @return size_t 转换后的PIN码长度 
     */
    static size_t FromAscii(const ByteArray& pinAscii, ByteBuilder& pin)
    {
        size_t cvtLen = ByteConvert::FromAscii(pinAscii, pin);
        // 长度为奇数,比如:12345,后补F
        if(pinAscii.GetLength() != 2 * cvtLen)
        {
            ByteBuilder pinTmp(2);

            pinTmp += static_cast<byte>(pinAscii[pinAscii.GetLength() - 1]);
            pinTmp += static_cast<byte>(0x0F);

            ByteConvert::FromAscii(pinTmp, pin);

            ++cvtLen;
        }
        return cvtLen;
    }
    /// 格式化PIN码 
    static ByteBuilder FromAscii(const ByteArray& pinAscii)
    {
        ByteBuilder pin(8);
        FromAscii(pinAscii, pin);
        return pin;
    }
    /**
     * @brief 将16进制的PIN码转换为ASCII码格式的PIN数据 
     * 
     * @param [in] pin 输入的16进制PIN码  
     * @param [out] pinAscii 转换后的PIN码  
     * 
     * @return size_t 转换后的长度 
     */
    static size_t ToAscii(const ByteArray& pin, ByteBuilder& pinAscii)
    {
        size_t cvtLen = ByteConvert::ToAscii(pin, pinAscii);
        if(StringConvert::LastCharAt(pinAscii, 0) == 'F')
        {
            pinAscii.RemoveTail();
            --cvtLen;
        }

        return cvtLen;
    }
};
/// APPLICATION BLOCK命令报文 
class ApplicationBlockCmd
{
protected:
    ApplicationBlockCmd(){}
public:
    /// mac长度为4字节
    static void Make(ByteBuilder& cmd, const ByteArray& mac)
    {
        //                   CLA INS P1 P2
        DevCommand::FromAscii("84 1E 00 00", cmd);
        // Lc 数据域长度(4)
        cmd += _itobyte(mac.GetLength());
        // 数据域
        cmd += mac;
        // Le 不存在
    }
};
//--------------------------------------------------------- 
/// APPLICATION UNBLOCK命令报文
class ApplicationUnblockCmd
{
protected:
    ApplicationUnblockCmd();
public:
    static void Make(ByteBuilder& cmd, const ByteArray& mac)
    {
        //                   CLA INS P1 P2
        DevCommand::FromAscii("84 18 00 00", cmd);
        // Lc 数据域长度(4)
        cmd += _itobyte(mac.GetLength());
        // 数据域
        cmd += mac;
        // Le 不存在
    }
};
//---------------------------------------------------------
/// CARD BLOCK命令报文
class CardBlockCmd
{
protected:
    CardBlockCmd(){}
public:
    static void Make(ByteBuilder& cmd, const ByteArray& mac)
    {
        //                   CLA INS P1 P2
        DevCommand::FromAscii("84 16 00 00", cmd);
        // Lc 数据域长度(4)
        cmd += _itobyte(mac.GetLength());
        // 数据域
        cmd += mac;
        // Le 不存在
    }
};
//---------------------------------------------------------
/// 外部认证（EXTERNAL AUTHENTICATE）命令报文
class ExternalAuthenticateCmd
{
protected:
    ExternalAuthenticateCmd(){}
public:
    static void Make(ByteBuilder& cmd, const ByteArray& data)
    {
        //                   CLA INS P1 P2
        DevCommand::FromAscii("00 82 00 00", cmd);
        // Lc 数据域长度(8 - 16)
        cmd += _itobyte(data.GetLength());
        // 数据域
        cmd += data;
        // Le 不存在
    }
};
//--------------------------------------------------------- 
/// 取随机数(GET CHALLENGE)命令报文 
class GetChallengeCmd
{
protected:
    GetChallengeCmd() {}
public:
    enum challenge_len { len4 = 0x04, len8 = 0x08 };
    /// 取随机数命令 le:04 08 需要生成的数据长度 
    static void Make(ByteBuilder& cmd, byte le = len4)
    {
        DevCommand::FromAscii("00 84 00 00", cmd);
        cmd += le;
    }
};
//--------------------------------------------------------- 
/// 取相应数据(GET RESPONSE)报文 
class GetResponseCmd
{
protected:
    GetResponseCmd() {}
public:
    static void Make(ByteBuilder& cmd)
    {
        DevCommand::FromAscii("00 C0 00 00 00", cmd);
    }
};
//--------------------------------------------------------- 
/// 内部认证（INTERNAL AUTHENTICATE）命令报文
class InternalAuthenicateCmd
{
protected:
    InternalAuthenicateCmd(){}
public:
    static void Make(ByteBuilder& cmd, const ByteArray& data)
    {
        //                   CLA INS P1 P2
        DevCommand::FromAscii("00 88 00 00", cmd);
        // Lc
        cmd += _itobyte(data.GetLength());
        // 数据域
        cmd += data;
        // Le
        //DevCommand::FromAscii("00", cmd);
    }
};
//--------------------------------------------------------- 
/// PIN CHANGE/UNBLOCK命令报文
class PinChangeUnblockCmd
{
protected:
    PinChangeUnblockCmd(){}
public:
    enum PinMode
    {
        /// PIN复位
        Reset = 0x00,
        /// 修改时使用当前
        UseCurrent = 0x01,
        /// 修改时不使用当前
        UseNew = 0x02
    };
    static void Make(ByteBuilder& cmd, const ByteArray& data, PinMode mode)
    {
        //                   CLA INS P1 P2
        DevCommand::FromAscii("84 24 00 00", cmd);
        // 设置P2
        cmd[ICCardLibrary::P2_INDEX] = _itobyte(mode);
        // Lc
        cmd += _itobyte(data.GetLength());
        // 数据域
        cmd += data;
        // Le 不存在
    }
};
//--------------------------------------------------------- 
/// 读二进制文件(READ BINARY)命令报文  
class ReadBinaryCmd
{
protected:
    ReadBinaryCmd() {}
public:
    static void Make(ByteBuilder& cmd, byte sfi, byte offset, byte len, const ByteArray& mac_4)
    {
        DevCommand::FromAscii("04 B0", cmd);
        cmd += static_cast<byte>(0x80 | (sfi & 0x1F));
        cmd += offset;
        cmd += _itobyte(mac_4.GetLength());
        cmd += mac_4;
        // le
        cmd += _itobyte(len);
    }
    static void Make(ByteBuilder& cmd, byte sfi, byte offset, byte len)
    {
        DevCommand::FromAscii("00 B0", cmd);
        cmd += static_cast<byte>(0x80 | (sfi & 0x1F));
        cmd += offset;
        // lc 不存在 

        // le
        cmd += len;
    }
    static void Make(ByteBuilder& cmd, byte offset, byte len, const ByteArray& mac_4)
    {
        DevCommand::FromAscii("04 B0", cmd);
        cmd += static_cast<byte>(0x00);
        cmd += offset;
        cmd += _itobyte(mac_4.GetLength());
        cmd += mac_4;
        // le
        cmd += _itobyte(len);
    }
    static void Make(ByteBuilder& cmd, byte offset, byte len)
    {
        DevCommand::FromAscii("00 B0", cmd);
        cmd += static_cast<byte>(0x00);
        cmd += offset;
        // lc 不存在 

        // le
        cmd += _itobyte(len);
    }
};
//--------------------------------------------------------- 
/// 读记录（READ RECORD）命令报文
class ReadRecodeCmd
{
protected:
    ReadRecodeCmd(){}
public:
    /**
     * @brief 构造命令
     * @warning sfi必须在[1,30]之间
     */
    static void Make(ByteBuilder& cmd, byte sfi, byte recodeNum)
    {
        // P1引用记录号标识 b3b2b1 = 100
        const byte p1Tag = 0x04; 
        byte p2 = (sfi<<3) | p1Tag;
        //                   CLA INS P1
        DevCommand::FromAscii("00 B2 00", cmd);
        // P1
        cmd[ICCardLibrary::P1_INDEX] = recodeNum;
        // 引用控制参数
        cmd += p2;
        // Lc 不存在
        // 数据域 不存在
        // Le
        DevCommand::FromAscii("00", cmd);
    }
    /**
     * @brief 构造命令
     * @warning sfi必须在[1,30]之间
     * @param [out] cmd 生成的命令  
     * @param [in] sfi 需要读取的短文件标识符 
     * @param [in] recodeNum 记录号 
     * @param [in] mac_X MAC校验码(由应用决定算法和长度) 
     */ 
    static void Make(ByteBuilder& cmd, byte sfi, byte recodeNum, const ByteArray& mac_X)
    {
        // P1引用记录号标识 b3b2b1 = 100
        const byte p1Tag = 0x04; 
        byte p2 = (sfi<<3) | p1Tag;
        //                   CLA INS P1
        DevCommand::FromAscii("04 B2 00", cmd);
        // P1
        cmd[ICCardLibrary::P1_INDEX] = recodeNum;
        // 引用控制参数
        cmd += p2;
        
        cmd += _itobyte(mac_X.GetLength());
        cmd += mac_X;
        // Le
        DevCommand::FromAscii("00", cmd);
    }
    /**
     * @brief 构造命令  
     * @param [out] cmd 生成的命令 
     * @param [in] tag 需要读取的记录标识符 
     */ 
    static void Make(ByteBuilder& cmd, byte tag)
    {
        DevCommand::FromAscii("00 B2 00 00 00", cmd);
        cmd[ICCardLibrary::P1_INDEX] = tag;
    }
};
//--------------------------------------------------------- 
/// 选择（SELECT）命令报文
class SelectCmd
{
protected:
    SelectCmd(){}
public:
    enum SelectMode
    {
        /// 第一个有或仅有一个
        First = 0x00,
        /// 下一个
        Next = 0x02
    };
    enum AidMode
    {
        /// 不通过文件名选择 
        ByAid = 0x00,
        /// 通过文件名选择 
        ByName = 0x04
    };
    static void Make(ByteBuilder& cmd, const ByteArray& fileName, SelectMode mode = First, AidMode aidMode = ByName)
    {
        //                   CLA INS P1 P2
        DevCommand::FromAscii("00 A4 04 00", cmd);
        if(aidMode == ByAid)
        {
            cmd[ICCardLibrary::P1_INDEX] = static_cast<byte>(0x00);
        }
        // P2
        cmd[ICCardLibrary::P2_INDEX] = static_cast<byte>(mode);
        // Lc
        cmd += _itobyte(fileName.GetLength());
        // 数据域
        cmd += fileName;
        // Le
        //DevCommand::FromAscii("00", cmd);
    }
};
//--------------------------------------------------------- 
/// 修改二进制文件(UPDATE BINARY)命令报文 
class UpdateBinaryCmd
{
protected:
    UpdateBinaryCmd() {}
public:
    static void Make(ByteBuilder& cmd, byte sfi, byte offset, const ByteArray& data, const ByteArray& mac_4)
    {
        DevCommand::FromAscii("04 D6", cmd);
        // P1
        cmd += static_cast<byte>(0x80 | (sfi & 0x1F));
        // P2
        cmd += offset;

        size_t len = data.GetLength() + mac_4.GetLength();
        cmd += _itobyte(len);

        cmd += data;
        cmd += mac_4;
    }
    static void Make(ByteBuilder& cmd, byte offset, const ByteArray& data, const ByteArray& mac_4)
    {
        DevCommand::FromAscii("04 D6 00", cmd);
        // P2
        cmd += offset;

        size_t len = data.GetLength() + mac_4.GetLength();
        cmd += _itobyte(len);

        cmd += data;
        cmd += mac_4;
    }
    static void Make(ByteBuilder& cmd, byte sfi, byte offset, const ByteArray& data)
    {
        DevCommand::FromAscii("00 D6", cmd);
        // P1
        cmd += static_cast<byte>(0x80 | (sfi & 0x1F));
        // P2
        cmd += offset;

        cmd += _itobyte(data.GetLength());
        cmd += data;
    }
    static void Make(ByteBuilder& cmd, byte offset, const ByteArray& data)
    {
        DevCommand::FromAscii("00 D6 00", cmd);
        // P2
        cmd += offset;

        cmd += _itobyte(data.GetLength());
        cmd += data;
    }
};
//--------------------------------------------------------- 
/// 修改记录（UPDATE RECORD）命令报文
class UpdateRecodeCmd
{
protected:
    UpdateRecodeCmd(){}
public:
    enum RecodeMode
    {
        First = 0x00,
        Last = 0x01,
        Next = 0x02,
        Prev = 0x03
    };

    static void Make(ByteBuilder& cmd, byte sfi, byte recodeNum, const ByteArray& data, const ByteArray& mac_4)
    {
        //                   CLA INS P1 P2
        DevCommand::FromAscii("04 DC 00 00", cmd);
        // P1
        cmd[ICCardLibrary::P1_INDEX] = recodeNum;
        // P2
        // P1引用记录号标识 b3b2b1 = 100
        const byte p1Tag = 0x04; 
        byte p2 = (sfi<<3) | p1Tag;
        cmd[ICCardLibrary::P2_INDEX] = p2;
        // Lc
        byte lc = _itobyte(data.GetLength() + mac_4.GetLength());
        cmd += lc;
        // 数据域
        cmd += data;
        cmd += mac_4;
        // Le 不存在
    }
    static void Make(ByteBuilder& cmd, byte sfi, byte recodeNum, const ByteArray& data)
    {
        //                   CLA INS P1 P2
        DevCommand::FromAscii("00 DC 00 00", cmd);
        // P1
        cmd[ICCardLibrary::P1_INDEX] = recodeNum;
        // P2
        // P1引用记录号标识 b3b2b1 = 100
        const byte p1Tag = 0x04; 
        byte p2 = (sfi<<3) | p1Tag;
        cmd[ICCardLibrary::P2_INDEX] = p2;
        // Lc
        byte lc = _itobyte(data.GetLength());
        cmd += lc;
        // 数据域
        cmd += data;
        // Le 不存在
    }
    static void Make(ByteBuilder& cmd, byte sfi, RecodeMode recodeMode, const ByteArray& data, const ByteArray& mac_4)
    {
        //                   CLA INS P1 P2
        DevCommand::FromAscii("04 DC 00 00", cmd);
        // P2
        // P1引用记录号标识 b3b2b1 = 100
        const byte p1Tag = static_cast<byte>(recodeMode);
        byte p2 = (sfi<<3) | p1Tag;
        cmd[ICCardLibrary::P2_INDEX] = p2;
        // Lc
        byte lc = _itobyte(data.GetLength() + mac_4.GetLength());
        cmd += lc;
        // 数据域
        cmd += data;
        cmd += mac_4;
        // Le 不存在
    }
    static void Make(ByteBuilder& cmd, byte sfi, RecodeMode recodeMode, const ByteArray& data)
    {
        //                   CLA INS P1 P2
        DevCommand::FromAscii("00 DC 00 00", cmd);
        // P2
        // P1引用记录号标识 b3b2b1 = 100
        const byte p1Tag = static_cast<byte>(recodeMode);
        byte p2 = (sfi<<3) | p1Tag;
        cmd[ICCardLibrary::P2_INDEX] = p2;
        // Lc
        byte lc = _itobyte(data.GetLength());
        cmd += lc;
        // 数据域
        cmd += data;
        // Le 不存在
    }
};
//--------------------------------------------------------- 
/// 验证（VERIFY）命令报文
class VerifyCmd
{
protected:
    VerifyCmd(){}
public:
    static void Make(ByteBuilder& cmd, const ByteArray& pin)
    {
        //                   CLA INS P1 P2
        DevCommand::FromAscii("00 20 00 00", cmd);
        // Lc
        cmd += _itobyte(pin.GetLength());
        // 数据域
        cmd += pin;
        // Le 不存在
    }
};
//--------------------------------------------------------- 
} // namespace base
} // namesapce iccard 
} // namespace device 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_ICCARDCMD_H_
//========================================================= 