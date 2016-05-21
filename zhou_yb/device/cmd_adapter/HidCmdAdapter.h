//========================================================= 
/**@file HidCmdAdapter.h 
 * @brief 设备收发数据基础处理类
 * 
 * @date 2014-05-02   18:24:10 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_HIDCMDADAPTER_H_
#define _LIBZHOUYB_HIDCMDADAPTER_H_
//--------------------------------------------------------- 
#include "../../include/BaseDevice.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace device {
namespace cmd_adapter {
//--------------------------------------------------------- 
/**
 * @brief HID设备基础指令适配器 
 * 
 * - 接收数据遇到0x00则停止接收 
 * - 发送的数据如果超过整包大小,分包自动发送,不足一个整包的由底层设备自己处理 
 */ 
template<class THidDevice>
class HidFixedCmdAdapter :
    public IInteractiveTrans,
    public BaseDevAdapterBehavior<THidDevice>,
    public LoggerBehavior,
    public RefObject
{
public:
    //----------------------------------------------------- 
    /// 返回数据包是否为0包 
    static bool IsZero(const ByteArray& pkg)
    {
        const size_t PKG_SIZE = 64;
        byte zero_pkg[PKG_SIZE] = { 0 };
        size_t times = static_cast<size_t>(pkg.GetLength() / PKG_SIZE);
        for(size_t i = 0;i < times; ++i)
        {
            if(memcmp(pkg.GetBuffer(PKG_SIZE * i), zero_pkg, PKG_SIZE) != 0)
                return false;
        }

        // 比较整包之外剩余的字节 
        size_t len = pkg.GetLength() % PKG_SIZE;
        if(len > 0)
        {
            if(memcmp(pkg.GetBuffer(pkg.GetLength() - len), zero_pkg, len) != 0)
                return false;
        }

        return true;
    }
    //----------------------------------------------------- 
protected:
    //----------------------------------------------------- 
    /// 根据长度标识发送数据
    bool _FixedTransmit(const ByteArray& data, size_t fixedLen)
    {
        LOG_FUNC_NAME();
        ByteBuilder tmp(8);
        size_t packSize = BaseDevAdapterBehavior<THidDevice>::_pDev->GetSendLength();
        packSize -= fixedLen;
        LOGGER(_log.WriteLine("Send:"));
        // 先整包的发送
        size_t packCount = 0;
        size_t lastCount = 0;
        size_t packLen = 0;
        size_t sizeoffset = _min(fixedLen, sizeof(size_t));
        while(packCount < data.GetLength())
        {
            lastCount = data.GetLength() - packCount;
            tmp.Clear();

            packLen = _min(lastCount, packSize);
            // 如果长度>sizeof(size_t)则前补0x00
            if(fixedLen > sizeof(size_t))
            {
                tmp.Append(0x00, fixedLen - sizeof(size_t));
            }
            // 组数据长度
            for(size_t i = sizeoffset;i > 1; --i)
            {
                tmp += _itobyte(packLen >> ((i - 1) * BIT_OFFSET));
            }
            tmp += _itobyte(packLen);

            ByteArray subdata = data.SubArray(packCount, packLen);
            tmp += subdata;
            LOGGER(_log.WriteLine(tmp));

            if(!BaseDevAdapterBehavior<THidDevice>::_pDev->Write(tmp))
                return false;

            packCount += packSize;
        }

        return true;
    }
    /// 按照字符串的格式来发送(直接拆包)
    bool _StringTransmit(const ByteArray& data)
    {
        LOG_FUNC_NAME();
        size_t packSize = BaseDevAdapterBehavior<THidDevice>::_pDev->GetSendLength();
        LOGGER(_log.WriteLine("Send:"));
        // 刚好只需要发送一个数据包 
        if(data.GetLength() <= packSize)
        {
            LOGGER(_log.WriteLine(data));
            return BaseDevAdapterBehavior<THidDevice>::_pDev->Write(data);
        }
        // 需要多个数据包 
        size_t packCount = 0;
        size_t lastCount = 0;
        while(packCount < data.GetLength())
        {
            lastCount = data.GetLength() - packCount;
            ByteArray subdata = data.SubArray(packCount, _min(lastCount, packSize));
            LOGGER(_log.WriteLine(subdata));
            if(!BaseDevAdapterBehavior<THidDevice>::_pDev->Write(subdata))
                return _logRetValue(false);

            packCount += packSize;
        }

        return true;
    }
    /// 根据长度标识读取数据 
    bool _FixedReceive(ByteBuilder& data, size_t fixedLen)
    {
        LOG_FUNC_NAME();
        LOGGER(_log.WriteLine("Recv:"));
        ByteBuilder tmp(8);
        /* 读取第一个数据包 */
        if(!BaseDevAdapterBehavior<THidDevice>::_pDev->Read(tmp))
            return false;
        // 读取成功,处理后续的分包 
        /* 前N个字节为所有数据长度 */
        size_t packLen = 0;
        for(size_t i = 0;i < fixedLen - 1; ++i)
        {
            packLen += static_cast<size_t>(tmp[i] << BIT_OFFSET);
        }
        packLen += static_cast<size_t>(tmp[fixedLen - 1]);
        // 输入包大小 - 数据长度标识位长度 
        size_t validDataLen = BaseDevAdapterBehavior<THidDevice>::_pDev->GetRecvLength() - fixedLen;
        LOGGER(_log << "Count:<" << packLen << ">\n");
        ByteArray subdata = tmp.SubArray(fixedLen, _min(packLen, validDataLen));
        LOGGER(_log.WriteLine(subdata));
        data.Append(subdata);
        // 没有后续的包,全部数据已经接收  
        if(packLen <= validDataLen)
            return true;

        // 继续接收剩下的数据 
        packLen -= validDataLen;
        validDataLen = BaseDevAdapterBehavior<THidDevice>::_pDev->GetRecvLength();
        while(packLen > 0)
        {
            tmp.Clear();
            if(!BaseDevAdapterBehavior<THidDevice>::_pDev->Read(tmp))
                return false;

            // 最后一个包 
            if(packLen <= validDataLen)
            {
                subdata = tmp.SubArray(0, packLen);
                LOGGER(_log.WriteLine(subdata));
                data.Append(subdata);
                break;
            }
            else
            {
                LOGGER(_log.WriteLine(tmp));
                data += tmp;
            }

            packLen -= validDataLen;
        }

        return true;
    }
    /// 按照字符串的格式来接收 
    bool _StringReceive(ByteBuilder& data)
    {
        LOG_FUNC_NAME();
        LOGGER(_log.WriteLine("Recv:"));
        ByteBuilder tmp(8);
        size_t slen = 0;
        while(BaseDevAdapterBehavior<THidDevice>::_pDev->Read(tmp))
        {
            slen = strlen(tmp.GetString());
            ByteArray subdata = tmp.SubArray(0, slen);
            LOGGER(_log.WriteLine(subdata));
            data.Append(subdata);

            // 最后一个包 
            if(slen < tmp.GetLength())
                return true;
            tmp.Clear();
        }
        return slen > 0;
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    HidFixedCmdAdapter(size_t slen = 0, size_t rlen = 0) : BaseDevAdapterBehavior<THidDevice>() 
    {
        FixedOutput = slen;
        FixedInput = rlen;
    }
    //----------------------------------------------------- 
    /// 接收数据中长度标识所占用的最大字节数
    size_t FixedInput;
    /// 发送数据中长度标识所占用的最大字节数
    size_t FixedOutput;
    //----------------------------------------------------- 
    /// 读数据 
    virtual bool Read(ByteBuilder& data)
    {
        LOG_FUNC_NAME();
        if(!BaseDevAdapterBehavior<THidDevice>::IsValid())
            return _logRetValue(false);

        size_t packSize = BaseDevAdapterBehavior<THidDevice>::_pDev->GetRecvLength();
        if(FixedInput >= packSize)
        {
            FixedInput = 0;
        }
        if(FixedInput > 0)
            return _logRetValue(_FixedReceive(data, FixedInput));
        return _logRetValue(_StringReceive(data));
    }
    /// 写数据 
    virtual bool Write(const ByteArray& data)
    {
        LOG_FUNC_NAME();
        if(!BaseDevAdapterBehavior<THidDevice>::IsValid())
            return _logRetValue(false);

        size_t packSize = BaseDevAdapterBehavior<THidDevice>::_pDev->GetSendLength();
        if(FixedOutput >= packSize)
        {
            FixedOutput = 0;
        }
        if(FixedOutput > 0)
            return _logRetValue(_FixedTransmit(data, FixedOutput));
        return _logRetValue(_StringTransmit(data));
    }
    //----------------------------------------------------- 
    /// 清空数据(一直读到数据为0包为止) 
    bool Clean()
    {
        LOG_FUNC_NAME();
        if(!BaseDevAdapterBehavior<THidDevice>::IsValid())
            return _logRetValue(false);

        ByteBuilder tmp(8);
        while(BaseDevAdapterBehavior<THidDevice>::_pDev->Read(tmp))
        {
            if(IsZero(tmp))
                return _logRetValue(true);
            tmp.Clear();
        }

        return _logRetValue(false);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/// 使用模板做控制参数的HID指令协议适配器
template<class THidDevice, size_t SendSize, size_t RecvSize>
class HidCmdAdapter : public HidFixedCmdAdapter<THidDevice>
{
public:
    //----------------------------------------------------- 
    HidCmdAdapter() : HidFixedCmdAdapter<THidDevice>()
    {
        HidFixedCmdAdapter<THidDevice>::FixedOutput = SendSize;
        HidFixedCmdAdapter<THidDevice>::FixedInput = RecvSize;
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace cmd_adapter
} // namespace device 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_HIDCMDADAPTER_H_
//========================================================= 
