//========================================================= 
/**@file COM_IC_DevUpdater.h
 * @brief 串口IC卡设备固件升级程序 
 * 
 * @date 2015-05-25   01:00:13
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_COM_IC_DEVUPDATER_H_
#define _LIBZHOUYB_COM_IC_DEVUPDATER_H_
//--------------------------------------------------------- 
#include "../DevUpdater.h"
#include "../../../include/BaseDevice.h"
#include "../../../include/Extension.h"

#include "../../tools/ParamHelper.h"
using zhou_yb::application::tools::ParamHelper;

#include "../../tools/ReaderHelper.h"
using zhou_yb::application::tools::ComIC_ReaderHelper;

#include "../../lc/inc/LC_ComToCCID_CmdAdapter.h"
#include "../../lc/inc/LC_ReaderDevAdapter.h"
using zhou_yb::application::lc::LC_ComToCCID_CmdAdapter;
using zhou_yb::application::lc::LC_ReaderDevAdapter;

#include "../../test_frame/linker/TestLinkerHelper.h"
using zhou_yb::application::test::TestLinkerHelper;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace updater {
//--------------------------------------------------------- 
/// 串口设备检测升级状态连接器
struct ComUpdateModeTestLinker : public TestLinker<ComDevice>
{
    /// 获取加密数据的随机数
    static bool GetRandom(IInteractiveTrans& dev, ByteBuilder& random)
    {
        ByteBuilder cmd(8);
        ByteBuilder recv(8);

        DevCommand::FromAscii("FF CC 00 00 00", cmd);
        if(!dev.Write(cmd) || !dev.Read(recv))
            return false;

        if(!ICCardLibrary::IsSuccessSW(ICCardLibrary::GetSW(recv)))
            return false;

        ICCardLibrary::RemoveSW(recv);
        LC_Provider::RandomConvert(recv, random);
        return true;
    }
    /// 是否处于正常升级状态中
    static bool IsUpgradeReady(IInteractiveTrans& dev)
    {
        byte rdyCmd[2] = { 0xFF, 'S' };
        if(!dev.Write(ByteArray(rdyCmd, 2)))
            return false;
        return ComUpdateModeTestLinker::WaitSW(dev);
    }
    /// 等待状态码
    static bool WaitSW(IInteractiveTrans& dev)
    {
        ByteBuilder resp(2);
        while(resp.GetLength() < 2)
        {
            if(!dev.Read(resp))
                return false;
        }
        // 只保留左边2字节 
        StringConvert::Left(resp, 2);
        return ICCardLibrary::IsSuccessSW(ICCardLibrary::GetSW(resp));
    }
    /// 是否处于升级状态中
    static bool IsUpgradeMode(IInteractiveTrans& dev)
    {
        byte modeCmd[2] = { 0xFF, '?' };
        // FF ?
        if(!dev.Write(ByteArray(modeCmd, 2)))
            return false;
        // 已连接
        return WaitSW(dev);
    }
    /// 切换到升级状态
    static bool ToUpgradeMode(IInteractiveTrans& dev)
    {
        ComICCardCmdAdapter cmdAdapter;
        LC_ComToCCID_CmdAdapter lcCmdAdapter;
        LC_ReaderDevAdapter lcReader;

        WeakRef<IInteractiveTrans> ref(dev);
        cmdAdapter.SelectDevice(ref);
        lcCmdAdapter.SelectDevice(cmdAdapter);
        lcReader.SelectDevice(lcCmdAdapter);

        return lcReader.SetUpdateMode();
    }
    /**
     * @brief 扫描串口,并发送升级切换指令
     * 
     * @param [in] dev 需要操作的设备
     * @param [in] devArg 参数 [COM]
     * @param [in] printer 文本输出器
     */
    virtual bool Link(ComDevice& dev, const char* devArg, TextPrinter& printer)
    {
        LOGGER(printer.TextPrint(TextPrinter::TextLogger, "ComUpdateModeTestLinker::Link"));
        // 如果没有配置则自动去枚举串口
        uint port = 0;
        uint baud = CBR_9600;
        uint upgradeBaud = CBR_115200;

        ArgParser cfg;
        string devName = devArg;
        list<uint> comlist;
        if(cfg.Parse(devArg) < 1)
        {
            dev.EnumDevice(comlist);
        }
        else
        {
            byte gate = 0;
            devName = cfg["COM"].To<string>("AUTO");
            port = ParamHelper::ParseCOM(devName.c_str(), gate, baud);
            if(port > 0)
            {
                comlist.push_back(port);
            }
        }
        // 没有检测到串口
        if(comlist.size() < 1)
            return false;

        // 轮询每个端口判断连接状态
        list<uint>::iterator itr;
        dev.SetWaitTimeout(DEV_TEST_INTERVAL);
        for(itr = comlist.begin();itr != comlist.end(); ++itr)
        {
            if(ComDeviceHelper::OpenDevice(dev, *itr, upgradeBaud) == DevHelper::EnumSUCCESS)
            {
                if(IsUpgradeMode(dev))
                    return true;
                dev.Close();
            }
        }
        /* 设备处于正常模式 */
        // 查找设备正常连接的串口
        port = ComIC_ReaderHelper::ScanReader(baud, 500);
        if(port < 1)
            return false;

        if(ComDeviceHelper::OpenDevice<ComDevice>(dev, port, upgradeBaud) != DevHelper::EnumSUCCESS)
            return false;
        if(!TestLinkerHelper::LinkTimeoutBehavior(dev, cfg, printer))
            return false;
        if(!TestLinkerHelper::LinkCommand(dev, cfg, printer))
            return false;
        if(ToUpgradeMode(dev))
        {
            if(IsUpgradeMode(dev))
                return true;
        }
        dev.Close();
        return false;
    }
    /// 关闭串口
    virtual bool UnLink(ComDevice& dev, TextPrinter&)
    {
        dev.Close();
        return true;
    }
};
//--------------------------------------------------------- 
/// 串口读卡器文件行升级器
class ComUpdaterTestCase : public ITestCase<IInteractiveTrans>
{
protected:
    /// 已升级的文件行数
    size_t _updateCount;
    /// 发送N条后接收状态码
    size_t _swCount;
public:
    ComUpdaterTestCase(size_t swCount = 1) : _swCount(swCount) {}
    /// 升级行
    virtual bool Test(Ref<IInteractiveTrans>& testObj, const ByteArray& testArg, TextPrinter&)
    {
        // 所有数据已经发送完
        if(testArg.IsEmpty())
            return ComUpdateModeTestLinker::WaitSW(testObj);
        
        // 先发送长度
        byte len = _itobyte(testArg.GetLength());
        // 直接发送数据包 
        if(testObj->Write(ByteArray(&len, 1)) && testObj->Write(testArg))
        {
            ++_updateCount;
            // 连续发送N条或最后一条才判断状态码 
            if((_updateCount >= _swCount))
            {
                _updateCount = 0;
                return ComUpdateModeTestLinker::WaitSW(testObj);
            }
            return true;
        }
        return false;
    }
};
//--------------------------------------------------------- 
/// 蓝牙转串口读卡器文件行升级器
class BluetoothToComUpdaterTestCase : public ITestCase<IInteractiveTrans>
{
protected:
    ByteBuilder _cacheBuff;
    size_t _cacheSize;
public:
    BluetoothToComUpdaterTestCase(size_t cacheSize = FILE_K(1)) : _cacheSize(cacheSize) {}
    /// 升级行
    virtual bool Test(Ref<IInteractiveTrans>& testObj, const ByteArray& testArg, TextPrinter&)
    {
        // 所有数据已经发送完
        if(testArg.IsEmpty())
        {
            if(!_cacheBuff.IsEmpty())
            {
                if(!testObj->Write(_cacheBuff))
                    return false;
                _cacheBuff.Clear();
            }
            return ComUpdateModeTestLinker::WaitSW(testObj);
        }

        size_t len = testArg.GetLength() + 1;
        if(_cacheBuff.GetLength() + len >= _cacheSize)
        {
            if(!testObj->Write(_cacheBuff))
                return false;
            _cacheBuff.Clear();
            return ComUpdateModeTestLinker::WaitSW(testObj);
        }
        _cacheBuff += _itobyte(testArg.GetLength());
        _cacheBuff += testArg;

        return true;
    }
};
//--------------------------------------------------------- 
/// 串口IC卡读卡器固件升级程序 
template<
    class TDevice,
    class TInterface,
    class TLinker, 
    class TContainer = TestContainer<TDevice, TInterface>, 
    class TDecoder = UpdateDecoder>
class COM_IC_DevUpdater : public DevUpdater<TDevice, TInterface, TLinker, TContainer, TDecoder>
{
protected:
    //----------------------------------------------------- 
    /// 加密的随机数 
    ByteBuilder _random;
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    virtual bool PreTest(const char* preArg)
    {
        if(!DevUpdater::PreTest(preArg))
            return false;
        // 获取随机数
        _random.Clear();
        if(!ComUpdateModeTestLinker::GetRandom(_testInterface, _random))
        {
            TextPrint(TextPrinter::TextError, "获取随机密钥失败");
            return false;
        }
        
        // 通讯握手
        if(!ComUpdateModeTestLinker::IsUpgradeReady(_testInterface))
        {
            TextPrint(TextPrinter::TextError, "通讯握手失败");
            return false;
        }

        // 处理升级数据
        list<ByteBuilder>::iterator itr;
        for(itr = DevUpdater::_updateList.begin();itr != DevUpdater::_updateList.end(); ++itr)
        {
            ByteConvert::Xor(_random, *itr);
            if(!Interrupter.IsNull() && Interrupter->InterruptionPoint())
            {
                TextPrint(TextPrinter::TextError, "操作被取消");
                return false;
            }
        }
        // 增加一个空行标记文件升级结束
        _updateList.push_back(ByteBuilder());
        return true;
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace updater
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_COM_IC_DEVUPDATER_H_
//========================================================= 