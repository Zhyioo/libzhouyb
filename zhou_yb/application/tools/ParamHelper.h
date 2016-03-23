//========================================================= 
/**@file ParamHelper.h
 * @brief 参数解析辅助工具 
 * 
 * @date 2015-06-10   20:18:12
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../include/Extension.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace tools {
//--------------------------------------------------------- 
/// 参数解析 
class ParamHelper
{
protected:
    ParamHelper() {}
public:
    /**
     * @brief 解析串口号
     * 
     * @param [in] strCOM 输入的串口号 
     * 
     * - 格式:
     *  - NULL = AUTO
     *  - "" = AUTO
     *  - "1" 数字形式 
     *  - "COM1" 带COM开头 
     *  - "COM0" "0" 自动枚举第一个能够打开的串口 
     *  - "AUTO" 自动枚举第一个能否打开的串口 
     * .
     * 
     * @return uint 获取到的串口号 
     */
    static uint ParseCOM(const char* strCOM)
    {
        uint port = 0;
        list<uint> comlist;
        ByteArray strArray(strCOM);

        if(!strArray.IsEmpty())
        {
            if(StringConvert::StartWith(strArray, "COM", true))
            {
                port = ArgConvert::FromString<uint>(strCOM + 3);
            }
            else if(_is_digital(strCOM[0]))
            {
                port = ArgConvert::FromString<uint>(strCOM);
            }
        }

        // 需要自动枚举第一个能打开的端口  
        if(port < 1)
        {
            ComDevice dev;
            dev.EnumDevice(comlist);
            list<uint>::iterator itr;
            for(itr = comlist.begin();itr != comlist.end(); ++itr)
            {
                if(dev.Open(*itr))
                {
                    port = *itr;
                    dev.Close();

                    break;
                }
            }
        }

        return port;
    }
    /// 解析串口号 "COM1:B,115200" "COM1,115200"
    static uint ParseCOM(const char* strCOM, byte& gate, uint& baud)
    {
        gate = 'A';
        baud = 9600;
        return ParseCOM(strCOM);
    }
    /**
     * @brief 将精确到小数点后两位的金额转换为以分为单位的金额 
     * 
     * @param [in] sMoney 需要转换的金额 "0.13"
     * @param [in] degree [default:2] 小数点后面的精度 
     * 
     * @return uint 转换后以分为单位的金额 "0.13" => "13"
     */
    static uint ParseMoney(const char* sMoney, uint degree = 2)
    {
        uint money = 0;
        uint deg = 1;
        // 整数部分 
        size_t index = StringConvert::IndexOf(sMoney, '.');
        // 获取整数部分 
        ArgConvert::FromString<uint>(sMoney, money);
        for(uint i = 0;i < degree; ++i)
            deg *= 10;
        money *= deg;
        
        if(index != SIZE_EOF)
        {
            uint tmp = 0;
            ByteBuilder tmpBuff(degree);
            tmpBuff = sMoney + index + 1;
            ByteConvert::Fill(tmpBuff, degree, true);
            ArgConvert::FromString<uint>(tmpBuff.GetString(), tmp);

            tmp %= deg;
            money += tmp;
        }

        return money;
    }
};
//--------------------------------------------------------- 
} // namespace tools
} // namespace application
} // namespace zhou_yb
//========================================================= 