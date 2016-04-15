//========================================================= 
/**@file PBOC_v1_0_CMD.h 
 * @brief PBOC1.0指令 
 * 
 * @date 2014-08-15   21:40:49 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_PBOC_V1_0_CMD_H_
#define _LIBZHOUYB_PBOC_V1_0_CMD_H_
//--------------------------------------------------------- 
#include "../../../../include/Base.h"
#include "../../base/ICCardLibrary.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace device {
namespace iccard {
namespace pboc {
namespace v1_0 {
//--------------------------------------------------------- 
/// PBOC1.0中的交易类型标识 
class PBOC_v1_0_TransType
{
protected:
    PBOC_v1_0_TransType() {}
public:
    /// PBOC1.0交易类型
    enum Type
    {
        /// 保留 
        Reserve = 0x00,
        /// ED圈存 
        ED_Load = 0x01,
        /// EP圈存 
        EP_Load = 0x02,
        /// 圈提 
        Unload = 0x03,
        /// ED取款 
        ED_CashWithdeaw = 0x04,
        /// ED消费  
        ED_Purchase = 0x05,
        /// EP消费 
        EP_Purchase = 0x06,
        /// ED修改透支限额 
        ED_Limit = 0x07,
        /// 信用消费 
        Credit_Purchase = 0x08,
        /// 复合消费 
        Composite_Purchase = 0x09
    };
};
//--------------------------------------------------------- 
/// 修改PIN码(CHANGE PIN)命令报文   
class ChangePinCmd
{
protected:    
    ChangePinCmd() {}
public:
    /// 80 5E 01 00 LC OLD_PIN FF NEWPIN
    static void Make(ByteBuilder& cmd, const ByteArray& oldPin, const ByteArray& newPin)
    {
        DevCommand::FromAscii("80 5E 01 00", cmd);

        size_t lc = oldPin.GetLength() + newPin.GetLength() + 1;
        cmd += _itobyte(lc);

        cmd += oldPin;
        cmd += static_cast<byte>(0x0FF);
        cmd += newPin;
    }
};
//--------------------------------------------------------- 
/// 圈存(CREDIT FOR LOAD)命令报文  
class CreditForLoadCmd
{
protected:
    CreditForLoadCmd() {}
public:
    /**
     * @brief 生成圈存指令 
     * @param [out] cmd 生成的命令 
     * @param [in] amtDate_4 4字节交易日期 
     * @param [in] amtTime_3 3字节交易时间 
     * @param [in] mac2_4 4字节MAC2数据 
     */ 
    static void Make(ByteBuilder& cmd, const ByteArray& amtDate_4, const ByteArray& amtTime_3, const ByteArray& mac2_4)
    {
        DevCommand::FromAscii("80 52 00 00 0B", cmd);
        cmd += amtDate_4;
        cmd += amtTime_3;
        cmd += mac2_4;

        cmd += static_cast<byte>(0x04);
    }
};
//--------------------------------------------------------- 
/// 消费(DEBIT FOR PURCHASE/CASH WITHDRAW)命令报文  
class DebitForPurchaseCmd
{
protected:
    DebitForPurchaseCmd() {}
public:
    /**
     * @brief 生成消费指令 
     * @param [out] cmd 生成的命令 
     * @param [in] termAmtNo_4 4字节终端交易序号 
     * @param [in] amtDate_4 4字节交易日期 
     * @param [in] amtTime_3 3字节交易时间 
     * @param [in] mac1_4 4字节MAC1 
     */ 
    static void Make(ByteBuilder& cmd, const ByteArray& termAmtNo_4, const ByteArray& amtDate_4, const ByteArray& amtTime_3, const ByteArray& mac1_4)
    {
        DevCommand::FromAscii("80 54 01 00 0F", cmd);
        cmd += termAmtNo_4;
        cmd += amtDate_4;
        cmd += amtTime_3;
        cmd += mac1_4;
        // LE
        cmd += static_cast<byte>(0x08);
    }
};
//--------------------------------------------------------- 
/// 圈提(DEBIT FOR UNLOAD)命令报文  
class DebitForUnloadCmd
{
protected:
    DebitForUnloadCmd() {}
public:
    /**
     * @brief 生成圈提指令 
     * @param [out] cmd 生成的圈提指令 
     * @param [in] amtDate_4 4字节交易日期 
     * @param [in] amtTime_3 3字节交易时间 
     * @param [in] mac2_4 4字节MAC2 
     */ 
    static void Make(ByteBuilder& cmd, const ByteArray& amtDate_4, const ByteArray& amtTime_3, const ByteArray& mac2_4)
    {
        DevCommand::FromAscii("80 54 03 00 0B", cmd);
        cmd += amtDate_4;
        cmd += amtTime_3;
        cmd += mac2_4;
        // LE 
        cmd += static_cast<byte>(0x04);
    }
};
//--------------------------------------------------------- 
/// 取余额(GET BALANCE)命令报文  
class GetBalanceCmd
{
protected:
    GetBalanceCmd() {}
public:
    //----------------------------------------------------- 
    /// 钱包类型 
    enum PurseType
    {
        /// 保留 
        Reserve = 0x00,
        /// 电子存折 
        ED = 0x01,
        /// 电子钱包 
        EP = 0x02
    };
    //----------------------------------------------------- 
    static void Make(ByteBuilder& cmd, PurseType type)
    {
        DevCommand::FromAscii("80 5C 00 00 04", cmd);
        cmd[ICCardLibrary::P2_INDEX] = static_cast<byte>(type);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/// 取交易认证(GET TRANSACTION PROVE)命令报文  
class GetTransactionProveCmd
{
protected:
    GetTransactionProveCmd() {}
public:
    /**
     * @brief 生成取交易认证命令 
     * @param [out] cmd 生成的命令  
     * @param [in] type 要获取的交易类型标识 
     * @param [in] amtNo_2 2字节脱机交易序号
     */ 
    static void Make(ByteBuilder& cmd, PBOC_v1_0_TransType::Type type, const ByteArray& amtNo_2)
    {
        DevCommand::FromAscii("80 5A 00 00 02", cmd);
        cmd[ICCardLibrary::P2_INDEX] = static_cast<byte>(type);

        cmd += amtNo_2;
        cmd += static_cast<byte>(0x08);
    }
};
//--------------------------------------------------------- 
/// 初始化取现(INITIALIZE FOR CASH WITHDRAW)命令报文  
class InitializeForCashWithdrawCmd
{
protected:
    InitializeForCashWithdrawCmd() {}
public:
    /**
     * @brief 初始化取现命令 
     * @param [out] cmd 生成的命令 
     * @param [in] keyIndex 密钥索引号 
     * @param [in] amtCash_4 4字节交易金额 
     * @param [in] termNo_6 6字节终端机编号 
     */ 
    static void Make(ByteBuilder& cmd, byte keyIndex, const ByteArray& amtCash_4, const ByteArray& termNo_6)
    {
        DevCommand::FromAscii("80 50 02 01 0B", cmd);
        cmd += keyIndex;
        cmd += amtCash_4;
        cmd += termNo_6;

        cmd += static_cast<byte>(0x0F);
    }
};
//--------------------------------------------------------- 
/// 圈存初始化(INITIALIZE FOR LOAD)命令报文  
class InitializeForLoadCmd
{
protected:
    InitializeForLoadCmd() {}
public:
    /**
     * @brief 圈存初始化指令  
     * @param [out] cmd 生成的命令 
     * @param [in] type 圈存类型 
     * @param [in] keyIndex 密钥索引号 
     * @param [in] amtCash_4 4字节交易金额 
     * @param [in] termNo_6 6字节终端机编号 
     */ 
    static void Make(ByteBuilder& cmd, PBOC_v1_0_TransType::Type type, byte keyIndex, const ByteArray& amtCash_4, const ByteArray& termNo_6)
    {
        DevCommand::FromAscii("80 50 00 00 0B", cmd);
        cmd[ICCardLibrary::P2_INDEX] = static_cast<byte>(type);

        cmd += keyIndex;
        cmd += amtCash_4;
        cmd += termNo_6;

        cmd += static_cast<byte>(0x10);
    }
};
//--------------------------------------------------------- 
/// 消费初始化(INITIALIZE FOR PURCHASE)命令报文  
class InitializeForPurchaseCmd
{
protected:
    InitializeForPurchaseCmd() {}
public:
    /**
     * @brief 消费初始化指令  
     * @param [out] cmd 生成的命令 
     * @param [in] type 消费类型 
     * @param [in] keyIndex 密钥索引号 
     * @param [in] amtCash_4 4字节交易金额 
     * @param [in] termNo_6 6字节终端机编号 
     */ 
    static void Make(ByteBuilder& cmd, PBOC_v1_0_TransType::Type type, byte keyIndex, const ByteArray& amtCash_4, const ByteArray& termNo_6)
    {
        DevCommand::FromAscii("80 50 01 00 0B", cmd);
        cmd[ICCardLibrary::P2_INDEX] = static_cast<byte>(type);

        cmd += keyIndex;
        cmd += amtCash_4;
        cmd += termNo_6;

        cmd += static_cast<byte>(0x0F);
    }
};
//--------------------------------------------------------- 
/// 圈提初始化(INITIALIZE FOR UNLOAD)命令报文  
class InitializeForUnloadCmd
{
protected:
    InitializeForUnloadCmd() {}
public:
    /**
     * @brief 圈提初始化指令  
     * @param [out] cmd 生成的命令 
     * @param [in] keyIndex 密钥索引号 
     * @param [in] amtCash_4 4字节交易金额 
     * @param [in] termNo_6 6字节终端机编号 
     */ 
    static void Make(ByteBuilder& cmd, byte keyIndex, const ByteArray& amtCash_4, const ByteArray& termNo_6)
    {
        DevCommand::FromAscii("80 50 05 01 0B", cmd);
        cmd += keyIndex;
        cmd += amtCash_4;
        cmd += termNo_6;

        cmd += static_cast<byte>(0x10);
    }
};
//--------------------------------------------------------- 
/// 修改初始化(INITIALIZE FOR UPDATE)命令报文  
class InitializeForUpdateCmd
{
protected:
    InitializeForUpdateCmd() {}
public:
    /**
     * @brief 修改初始化指令  
     * @param [out] cmd 生成的命令 
     * @param [in] keyIndex 密钥索引号 
     * @param [in] termNo_6 6字节终端机编号 
     */ 
    static void Make(ByteBuilder& cmd, byte keyIndex, const ByteArray& termNo_6)
    {
        DevCommand::FromAscii("80 50 04 01 07", cmd);
        cmd += keyIndex;
        cmd += termNo_6;

        cmd += static_cast<byte>(0x13);
    }
};
//--------------------------------------------------------- 
/// 重置PIN(RELOAD PIN)命令报文  
class ReloadPinCmd
{
protected:
    ReloadPinCmd() {}
public:
    /**
     * @brief 重置PIN 
     * @param [out] cmd 生成的命令 
     * @param [in] pin 2-6字节密钥 
     * @param [in] mac_4 4字节MAC 
     */ 
    static void Make(ByteBuilder& cmd, const ByteArray& pin, const ByteArray& mac_4)
    {
        DevCommand::FromAscii("80 5E 00 00", cmd);
        size_t len = pin.GetLength() + mac_4.GetLength();

        cmd += _itobyte(len);
        cmd += pin;
        cmd += mac_4;
    }
};
//--------------------------------------------------------- 
/// 修改透支限额(UPDATE OVERDRAW LIMIT)命令报文  
class UpdateOverdrawLimitCmd
{
protected:
    UpdateOverdrawLimitCmd() {}
public:
    static void Make(ByteBuilder& cmd, const ByteArray& limit_3, const ByteArray& amtDate_4, const ByteArray& amtTime_3, const ByteArray& mac2_4)
    {
        DevCommand::FromAscii("80 58 00 00 0E", cmd);
        cmd += limit_3;
        cmd += amtDate_4;
        cmd += amtTime_3;
        cmd += mac2_4;
        // LE
        cmd += static_cast<byte>(0x04);
    }
};
//--------------------------------------------------------- 
} // namespace v1_0 
} // namespace pboc
} // namesapce iccard 
} // namespace device 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_PBOC_V1_0_CMD_H_
//========================================================= 