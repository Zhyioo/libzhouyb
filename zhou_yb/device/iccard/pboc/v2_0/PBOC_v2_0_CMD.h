//========================================================= 
/**@file PBOC_v2_0_CMD.h 
 * @brief PBOC2.0规范的IC卡命令格式
 * 
 * @date 2014-10-17   17:34:39 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../../../include/Base.h"
#include "../../base/ICCardLibrary.h"
//---------------------------------------------------------
namespace zhou_yb {
namespace device {
namespace iccard {
namespace pboc {
namespace v2_0 {
//---------------------------------------------------------
//@{
/**@name
 * @brief 一系列命令构造器 
 */
//---------------------------------------------------------
/// 生成应用密文（GENERATE AC）命令报文
class GenerateACCmd
{
protected:
    GenerateACCmd(){}

    static void _getEmptyCmd(ByteBuilder& cmd, const ByteArray& data)
    {
        //                   CLA INS P1 P2
        DevCommand::FromAscii("80 AE 00 00", cmd);
        // Lc
        cmd += _itobyte(data.GetLength());
        // 数据域
        cmd += data;
        // Le
        //DevCommand::FromAscii("00", cmd);
    }
public:
    //-----------------------------------------------------
    /// 引用控制参数
    enum RefCtlFlag
    {
        /// b8b7 = 00
        AAC = 0x00,
        /// b8b7 = 01
        TC = 0x40,
        /// b8b7 = 10
        ARQC = 0x80,
        /// b8b7 = 11 保留
        Reserve = 0xC0
    };
    //-----------------------------------------------------
    /// 请求标志
    enum RequstFlag
    {
        /// 未明确请求复合动态数据认证/应用密文生成
        NoRequst = 0x00,
        /// 请求复合动态数据认证/应用密文生成
        NeedRequst = 0x10,
    };
    //-----------------------------------------------------
    // 格式
    static void Make(ByteBuilder& cmd, const ByteArray& data, 
        RefCtlFlag refCtl, RequstFlag requst)
    {
        _getEmptyCmd(cmd, data);
        // 设置特殊的P1位
        cmd[ICCardLibrary::P1_INDEX] = static_cast<byte>(refCtl);
        cmd[ICCardLibrary::P1_INDEX] |= static_cast<byte>(requst);
    }
};
//---------------------------------------------------------
/// 取数据（GET DATA）命令报文
class GetDataCmd
{
protected:
    GetDataCmd(){}
public:
    static void Make(ByteBuilder& cmd, const ByteArray& tagData)
    {
        //                   CLA INS P1 P2
        DevCommand::FromAscii("80 CA 00 00", cmd);
        // 设置特殊的tagData
        size_t len = tagData.GetLength();
        if(len < 1)
        {
            // 没有数据不需要拷贝
        }
        else if(len < 2)
        {
            cmd[ICCardLibrary::P2_INDEX] = tagData[0];
        }
        else
        {
            cmd[ICCardLibrary::P1_INDEX] = tagData[0];
            cmd[ICCardLibrary::P2_INDEX] = tagData[1];
        }
        // Lc 不存在
        // 数据域不存在
        // Le
        DevCommand::FromAscii("00", cmd);
    }
};
//---------------------------------------------------------
/// 获取处理选项（GPO）命令报文
class GPOCmd
{
protected:
    GPOCmd(){}
public:
    static void Make(ByteBuilder& cmd, const ByteArray& pdol)
    {
        //                   CLA INS P1 P2
        DevCommand::FromAscii("80 A8 00 00 00", cmd);
        // 设置长度域
        if(pdol.GetLength() > 255)
            return ;
        cmd[cmd.GetLength() - 1] = _itobyte(pdol.GetLength());
        // 设置数据域
        cmd += pdol;
        // Le
        //DevCommand::FromAscii("00", cmd);
    }
};
//---------------------------------------------------------
/// 设置数据（PUT DATA）命令报文
class PutDataCmd
{
protected:
    PutDataCmd(){}
public:
    static void Make(ByteBuilder& cmd, const ByteArray& tagData, const ByteArray& data)
    {
        //                   CLA INS P1 P2
        DevCommand::FromAscii("04 DA 00 00", cmd);
        // 设置特殊的tagData
        size_t len = tagData.GetLength();
        if(len < 1)
        {
            // 没有数据不需要拷贝
        }
        else if(len < 2)
        {
            cmd[ICCardLibrary::P2_INDEX] = tagData[0];
        }
        else
        {
            cmd[ICCardLibrary::P1_INDEX] = tagData[0];
            cmd[ICCardLibrary::P2_INDEX] = tagData[1];
        }
        // Lc 
        cmd += _itobyte(data.GetLength());
        // 数据域
        cmd += data;
        // Le 不存在
    }
};
//--------------------------------------------------------- 
//@}
//--------------------------------------------------------- 
} // namespace v2_0 
} // namespace pboc
} // namespace iccard 
} // namespace device 
} // namespace zhou_yb
//=========================================================