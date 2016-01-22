//========================================================= 
/**@file DevUpdaterConvert.h 
 * @brief 设备升级功能所使用的一些算法
 *
 * dev 文件格式说明:
 * 1.每个hex文件为一个 ":16进制数据"
 * 2.将冒号剔除
 * 3.后面的数据转为一个 ByteBuilder 数据 
 * 4.选定一个 "dev文件的说明(字符串,比如版本号,时间等)"(tagInfo) 和 "行数据加密的密钥(字符串)"(keyInfo)
 * 5.创建二进制的dev文件,将tagInfo的长度以TLV的长度格式写到文件中
 * 6.将tagInfo写到文件中 
 * 7.hex文件中每一行的数据与tagInfo异或,然后再与keyInfo异或
 * 8.第7步骤中的数据再以该行的长度的第一个字节进行异或 
 * 9.将得到的数据长度以TLV的长度格式写入文件 
 *10.将得到的数据拼为71标签写到文件(每个数据中,可以有多个71标签) 
 * 
 * @date 2014-8-17   19:19:28 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../include/Base.h"

#include "../../device/iccard/pboc/base/TlvElement.h"
using zhou_yb::device::iccard::pboc::base::TlvConvert;
using zhou_yb::device::iccard::pboc::base::TlvElement;
//---------------------------------------------------------
namespace zhou_yb {
namespace application {
namespace updater {
//--------------------------------------------------------- 
/// 设备更新数据转换  
class DevUpdaterConvert
{
private:
    //----------------------------------------------------- 
    DevUpdaterConvert() {}
    //----------------------------------------------------- 
    enum UpdateTag
    {
        TagHEX = ':',
        TagDEV = 0x071
    };
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /// 返回hex文件行BIN数据校验是否正常 
    static bool IsValidBIN(const ByteArray& bin)
    {
        /* HEX文件行,每个字节数据累加和为0 */
        if(bin.GetLength() < 5)
            return false;

        // 是否所有的数据都为0 
        bool isZero = true;
        byte sum = 0;
        for(size_t i = 0; i < bin.GetLength(); ++i)
        {
            if(bin[i] != 0)
                isZero = false;
            sum += bin[i];
        }

        return (!isZero) && (sum == 0);
    }
    /**
     * @brief 返回是否是HEX文件BIN数据的结束行
     * @param [in] bin 需要判断的BIN数据行 
     *
     * @warning bin长度必须大于0 
     */
    inline static bool IsEOF(const ByteArray& bin)
    {
        return bin[0] == 0x00;
    }
    //----------------------------------------------------- 
    /// HEX文件格式转升级BIN数据行 
    static bool HEXtoBIN(const ByteArray& hex, ByteBuilder& bin)
    {
        /* 过滤冒号,然后将数据转为16进制 */
        if(hex.GetLength() < 1 || hex[0] != static_cast<byte>(TagHEX))
            return false;
        ByteConvert::FromAscii(hex.SubArray(1), bin);

        return true;
    }
    /// 升级BIN数据行转DEV文件格式行(加密) 
    static bool BINtoDEV(const ByteArray& bin, ByteBuilder& dev, const ByteArray& key = "", const ByteArray& info = "")
    {
        static ByteBuilder tmp(64);
        static ByteBuilder recode(32);

        byte lenByte = 0x00;
        tmp = bin;

        recode.Clear();
        recode += static_cast<byte>(TagDEV);

        TlvConvert::ToLengthBytes(tmp.GetLength(), recode);
        lenByte = recode[1];

        ByteConvert::Xor(info, tmp);
        ByteConvert::Xor(key, tmp);

        for(size_t i = 0;i < tmp.GetLength(); ++i)
            recode += static_cast<byte>(tmp[i] ^ lenByte);

        tmp.Clear();
        TlvConvert::ToLengthBytes(recode.GetLength(), tmp);
        dev += tmp;
        dev += recode;

        return true;
    }
    /// 升级BIN数据行转HEX文件行(解密) 
    static bool DEVtoBIN(const ByteArray& dev, ByteBuilder& bin, const ByteArray& key = "", const ByteArray& info = "")
    {
        byte lenByte = 0x00;
        ByteBuilder tmp(2);
        TlvConvert::ToLengthBytes(dev.GetLength(), tmp);
        lenByte = tmp[0];

        size_t len = dev.GetLength();
        size_t lastlen = bin.GetLength();
        for(size_t i = 0;i < len; ++i)
            bin += static_cast<byte>(dev[i] ^ lenByte);

        ByteArray subArray = bin.SubArray(lastlen);
        ByteConvert::Xor(key, subArray);
        ByteConvert::Xor(info, subArray);

        return true;
    }
    /// DEV文件格式行转升级BIN数据行 
    static bool BINtoHEX(const ByteArray& bin, ByteBuilder& hex)
    {
        hex += static_cast<byte>(TagHEX);
        ByteConvert::ToAscii(bin, hex);

        return true;
    }
    //----------------------------------------------------- 
    /// HEX文件格式行转DEV文件格式行 
    static bool HEXtoDEV(const ByteArray& hex, ByteBuilder& dev, const ByteArray& key = "", const ByteArray& info = "") 
    {
        static ByteBuilder bin(64);
        bin.Clear();

        HEXtoBIN(hex, bin);
        BINtoDEV(bin, dev, key, info);

        return true;
    }
    /// DEV文件格式行转HEX文件格式行 
    static bool DEVtoHEX(const ByteArray& dev, ByteBuilder& hex, const ByteArray& key = "", const ByteArray& info = "")
    {
        static ByteBuilder bin(64);
        bin.Clear();

        DEVtoBIN(dev, bin, key, info);
        BINtoHEX(bin, hex);

        return true;
    } 
    //----------------------------------------------------- 
    /**
     * @brief 打开*.dev文件 
     * @param [out] fin 打开后的文件流 
     * @param [in] srcPath *.dev文件的路径 
     * @param [out] pTagInfo [default:NULL] 文件描述信息(为NULL表示不需要) 
     */ 
    static bool OpenDEV(ifstream& fin, const char* srcPath, ByteBuilder* pTagInfo = NULL)
    {
        fin.open(srcPath, ios::binary);
        if(fin.fail())
            return false;

        ByteBuilder devline(128);
        // 第一行为文件描述信息 
        if(!ReadDEV(fin, devline))
        {
            fin.close();
            return false;
        }
        if(pTagInfo != NULL)
            (*pTagInfo) += devline;

        return true;
    }
    /// 读取dev文件中的一行(一个tag标签) 
    static bool ReadDEV(istream& fin, ByteBuilder& devline)
    {
        size_t len = 0;
        char lenbuff[16] = {0};
        size_t lastLen = devline.GetLength();

        fin.read(lenbuff, 1);

        len = static_cast<size_t>(lenbuff[0]);
        // 长度为多字节标签 
        if((lenbuff[0] & 0x80) == 0x80)
        {
            size_t tmplen = static_cast<size_t>(lenbuff[0] & 0x7F);
            fin.read(lenbuff, 1);
            len = static_cast<size_t>(lenbuff[0]);
            for(size_t i = 1;i < tmplen; ++i)
            {
                len <<= sizeof(char) * BIT_OFFSET;
                fin.read(lenbuff, 1);
                len += static_cast<size_t>(lenbuff[0]);
            }
        }
        if(len > 0)
        {
            devline.Append(static_cast<byte>(0x00), len);
            fin.read(const_cast<char*>(devline.GetString() + lastLen), len);
        }

        return !(fin.eof());
    }
    /// 解析 *.dev 行中的子行数据 
    static bool ParseDEV(const ByteArray& devline, list<ByteBuilder>& subDevLine)
    {
        TlvElement tagElement = TlvElement::Parse(devline);
        if(tagElement.IsEmpty())
            return false;

        size_t count = 0;
        TlvElement subElement = tagElement.SelectAfter(static_cast<ushort>(TagDEV));
        while(!subElement.IsEmpty())
        {
            subDevLine.push_back(ByteBuilder());
            ++count;
            subElement.GetValue(subDevLine.back());
            subElement = tagElement.SelectAfter(static_cast<ushort>(TagDEV));
        }

        return (count > 0);
    }
    /// 打开*.hex文件 
    static bool OpenHEX(ifstream& fin, const char* srcPath)
    {
        fin.open(srcPath);
        return !fin.fail();
    }
    /// 读取hex文件的非空行 
    static bool ReadHEX(istream& fin, ByteBuilder& hexline)
    {
        char c = 0x00;
        bool bRead = false;
        while(!fin.eof())
        {
            c = static_cast<char>(fin.get());

            // 遇到换行符,如果不是空行则当前行读取完毕 
            if(c == '\n' || c == EOF)
            {
                if(bRead)
                    break;
                continue;
            }
            
            hexline += static_cast<byte>(c);
            bRead = true;
        }
        return bRead;
    }
    /// 关闭打开的dev或者hex文件 
    static inline void Close(ifstream& fin)
    {
        fin.close();
    }
    //----------------------------------------------------- 
    /**
     * @brief 将*.hex文件保存成*.dev文件 
     * @param [in] hex 需要转换的hex文件 
     * @param [in] dev [default:NULL] 转换后的文件路径(为NULL表示转换为同名文件)
     * @param [in] key [default:""] 加密的密钥 
     * @param [in] info [default:""] 文件描述信息 
     */ 
    static bool SaveHEXtoDEV(const char* hex, const char* dev = NULL, const ByteArray& key = "", const ByteArray& info = "")
    {
        ifstream fin;
        ofstream fout;

        if(!OpenHEX(fin, hex))
            return false;

        if(_is_empty_or_null(dev))
        {
            ByteBuilder path = hex;
            path.RemoveTail(4);
            path += ".dev";

            fout.open(path.GetString(), ios::binary);
        }
        else
        {
            fout.open(dev, ios::binary);
        }
        if(fout.fail())
            return false;

        ByteBuilder hexline(64);
        ByteBuilder devline(64);

        TlvConvert::ToLengthBytes(info.GetLength(), devline);
        fout.write(devline.GetString(), devline.GetLength());
        if(!devline.IsEmpty())
            fout.write(info.GetString(), info.GetLength());
        devline.Clear();

        while(ReadHEX(fin, hexline))
        {
            if(!HEXtoDEV(hexline, devline, key, info))
                return false;

            fout.write(devline.GetString(), devline.GetLength());

            hexline.Clear();
            devline.Clear();
        }
        fout.close();
        fin.close();

        return true;
    }
    /**
     * @brief 将*.dev文件保存成*.hex文件 
     * @param [in] dev 
     * @param [in] hex [default:NULL] 转换后的文件路径(为NULL表示转换为同名文件)
     * @param [in] key [default:""] 加密的密钥 
     * @param [out] pInfo [default:NULL] 文件描述信息 
     */ 
    static bool SaveDEVtoHEX(const char* dev, const char* hex = NULL, const ByteArray& key = "", ByteBuilder* pInfo = NULL)
    {
        ifstream fin;
        ofstream fout;

        ByteBuilder info(16);
        if(!OpenDEV(fin, dev, &info))
            return false;

        if(_is_empty_or_null(hex))
        {
            ByteBuilder path = dev;
            path.RemoveTail(4);
            path += ".hex";

            fout.open(path.GetString());
        }
        else
        {
            fout.open(hex);
        }
        if(fout.fail())
            return false;

        ByteBuilder hexline(64);
        ByteBuilder devline(64);

        list<ByteBuilder> subline;

        while(ReadDEV(fin, devline))
        {
            subline.clear();

            if(!ParseDEV(devline, subline))
                return false;

            devline.Clear();
            for(list<ByteBuilder>::iterator itr = subline.begin();itr != subline.end(); ++itr)
            {
                hexline.Clear();
                DEVtoHEX(*itr, hexline, key, info);
                hexline += "\n";
                fout.write(hexline.GetString(), hexline.GetLength());
            }
        }

        if(pInfo != NULL)
        {
            (*pInfo) += info;
        }

        fin.close();
        fout.close();

        return true;
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace updater 
} // namespace application 
} // namespace zhou_yb 
//========================================================= 
