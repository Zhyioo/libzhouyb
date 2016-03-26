//========================================================= 
/**@file COM_IC_DevUpdater.h
 * @brief 串口IC卡设备固件升级程序 
 * 
 * @date 2015-05-25   01:00:13
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../DevUpdater.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace updater {
//--------------------------------------------------------- 
/// 串口设备升级连接器
template<class TComDevice>
struct ComUpdateModeTestLinker : public TestLinker<TComDevice>
{

    virtual bool Link(TComDevice& dev, const char*, TextPrinter&)
    {

    }

    virtual bool UnLink(TComDevice& dev, TextPrinter&)
    {
        dev.Close();
        return true;
    }
};
/// 串口读卡器文件行升级器
template<class TComDevice>
class ComUpdaterTestCase : public ITestCase<TComDevice>
{
protected:
    /// 等待状态码
    bool _WaitSW(TComDevice& dev)
    {
        ByteBuilder resp(2);
        resp.Append(static_cast<byte>(0x00), 2);

        while(resp.GetLength() < 2)
        {
            if(!dev.Read(resp))
                return false;
        }
        // 只保留左边2字节 
        StringConvert::Left(resp, 2);
        return ICCardLibrary::IsSuccessSW(ICCardLibrary::GetSW(resp));
    }
public:
    /// 升级行
    virtual bool Test(Ref<TComDevice>& testObj, const ByteArray& testArg, TextPrinter&)
    {

    }
};
/// 串口IC卡读卡器固件升级程序 
class COM_IC_DevUpdater : public DevUpdater
{
protected:
    //----------------------------------------------------- 
    /// 底层的通讯设备 
    ComDevice _dev;
    /// 临时缓冲区 
    ByteBuilder _tmpBuffer;
    /// 加密的随机数 
    ByteBuilder _random;
    /// 已经升级的行数 
    size_t _updateCount;
    /// 每次升级的行数 
    size_t _swCount;
    /// 发送切换设备的指令 
    bool _transUpdateCmd(Ref<IInteractiveTrans>& devTrans)
    {
        ComICCardCmdAdapter cmdAdapter;
        ByteBuilder updateRecv(8);

        cmdAdapter.SelectDevice(devTrans);
        cmdAdapter.Write(DevCommand::FromAscii("31 A0 FF 22 00 00 00"));

        devTrans->Read(updateRecv);
        return true;
    }
    /// 等待状态码 
    bool _WaitSW()
    {
        ByteBuilder resp(2);
        resp.Append(static_cast<byte>(0x00), 2);

        while(resp.GetLength() < 2)
        {
            if(!_dev.Read(resp))
                return false;
        }
        // 只保留左边2字节 
        StringConvert::Left(resp, 2);

        return ICCardLibrary::IsSuccessSW(ICCardLibrary::GetSW(resp));
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /**
     * @brief 升级每一行数据
     */
    virtual bool UpdateLine(const ByteArray& bin)
    {
        // 将随机数与hex数据异或 
        _tmpBuffer.Clear();
        _tmpBuffer += static_cast<byte>(bin.GetLength());
        _tmpBuffer += bin;

        ByteConvert::Xor(_random, _tmpBuffer.SubArray(1));
        // 直接发送数据包 
        if(_dev.Write(_tmpBuffer))
        {
            ++_updateCount;
            // 连续发送N条或最后一条才判断状态码 
            if((_updateCount == _swCount) || DevUpdaterConvert::IsEOF(_tmpBuffer))
            {
                return _WaitSW();
            }
            return true;
        }

        return false;
    }
    /**
     * @brief 连接设备
     * @param [in] sArg 串口号和波特率, 键值:COM,Baud,Count
     */ 
    virtual bool Link(const char* sArg = NULL)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "sArg:<" << sArg << ">\n");

        const size_t MAX_TIMEOUT = 20000;

        uint port = 0;
        uint baud = CBR_115200;

        ArgParser cfg;
        cfg.Parse(sArg);
        ArgConvert::FromConfig<uint>(cfg, ComParam::PortKey, port);
        ArgConvert::FromConfig<uint>(cfg, ComParam::BaudKey, baud);

        // 默认为每次一条HEX数据后都接收状态码   
        _swCount = 1;
        ArgConvert::FromConfig<uint>(cfg, "COUNT", _swCount);
        // 自动枚举串口 
        if(port == 0)
        {
            list<uint> comlist;
            if(_dev.EnumDevice(comlist) < 1)
            {
                LOGGER(_log.WriteLine("没有识别到串口"));
                return _logRetValue(false);
            }
            port = comlist.front();
        }

        // 使用设置的波特率发送升级准备指令 
        if(!_dev.Open(port, baud))
            return false;

        ByteBuilder cmd(8);
        DevCommand::FromAscii("1B 24 49", cmd);
        _dev.Write(cmd);

        ByteBuilder recv(2);
        ComICCardCmdAdapter cmdAdapter;
        cmdAdapter.SelectDevice(_dev);

        cmd.Clear();
        DevCommand::FromAscii("31 A0 FF 22 00 00 00", cmd);
        cmdAdapter.Write(cmd);

        // 1秒内等待回复 
        _dev.SetWaitTimeout(1000);
        _dev.SetOperatorInterval(50);
        _dev.SetReadTX(0x02, 0x03);
        cmdAdapter.Read(recv);

        // 使用设置的通讯波特率尝试连接设备 
        if(baud != CBR_115200)
        {
            _dev.Close();
            _dev.Open(port, CBR_115200);
        }
        _dev.SetReadTX(0x00, 0x00);

        cmd.Clear();
        cmd += static_cast<byte>(0xFF);
        cmd += static_cast<byte>('?');

        Timer timer;
        while(timer.Elapsed() < MAX_TIMEOUT)
        {
            if(_dev.Write(cmd))
            {
                if(_WaitSW())
                    break;
            }
            Timer::Wait(100);
        }

        if(recv.IsEmpty())
        {
            LOGGER(_log.WriteLine("设备切换升级模式失败"));
            return _logRetValue(false);
        }

        recv.Clear();
        cmd.Clear();
        DevCommand::FromAscii("FF CC 00 00 00", cmd);
        if(!_dev.Write(cmd) || !_dev.Read(recv))
        {
            LOGGER(_log.WriteLine("获取随机数失败"));
            return _logRetValue(false);
        }

        if(!ICCardLibrary::IsSuccessSW(ICCardLibrary::GetSW(recv)))
        {
            LOGGER(_log.WriteLine("获取随机数数据错误"));
            return _logRetValue(false);
        }

        ICCardLibrary::RemoveSW(recv);
        LOGGER(_log << "随机数:<" << recv << ">\n");
        _random.Clear();
        LC_Provider::RandomConvert(recv, _random);

        LOGGER(_log << "转换后随机数:<" << _random << ">\n");

        cmd.Clear();
        cmd += static_cast<byte>(0x0FF);
        cmd += static_cast<byte>('S');

        _dev.Write(cmd);
        _dev.SetWaitTimeout(MAX_TIMEOUT);

        if(!_WaitSW())
        {
            LOGGER(_log.WriteLine("[FF S]设备握手连接失败"));
            return _logRetValue(false);
        }

        return _logRetValue(true);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace updater
} // namespace application
} // namespace zhou_yb
//========================================================= 