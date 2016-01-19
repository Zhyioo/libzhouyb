//========================================================= 
/**@file HID_IC_DevUpdater.h 
 * @brief HID IC卡固件升级程序 
 * 
 * @date 2015-01-03   11:13:01 
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
/// HID IC卡固件升级程序 
class HID_IC_DevUpdater : public DevUpdater
{
protected:
    //----------------------------------------------------- 
    /// 底层的通讯设备 
    HidDevice _dev;
    /// 初始化设备  
    inline void _init_dev()
    {
        _dev.SetOperatorInterval(10);
    }
    /// 临时缓冲区 
    ByteBuilder _tmpBuffer;
    //----------------------------------------------------- 
    /// 发送切换设备的指令 
    bool _transUpdateCmd(Ref<IInteractiveTrans>& devTrans)
    {
        ComICCardCmdAdapter cmdAdapter;
        ByteBuilder updateRecv(8);

        cmdAdapter.SelectDevice(devTrans);
        cmdAdapter.Write(DevCommand::FromAscii("31 15"));
        
        return devTrans->Read(updateRecv);
    }
    //----------------------------------------------------- 
    /// 连接设备 
    bool _PreLink(const char* sArg)
    {
        // 升级模式下的设备名称 
        string UPGRADE_NAME;
        // 正常模式下的设备名称 
        string NORMAL_NAME;

        /* 解析设备名称 */
        list<string> arglist;
        list<string>::iterator argitr;
        StringHelper::Split(sArg, arglist, SPLIT_CHAR);
        switch(arglist.size())
        {
        case 0:
            UPGRADE_NAME = "HID Boot";
            break;
        case 1:
            UPGRADE_NAME = arglist.front();
            break;
        case 2:
        default:
            UPGRADE_NAME = arglist.front();
            argitr = list_helper<string>::index_of(arglist, 1);
            NORMAL_NAME = (*argitr);
            break;
        }

        list<HidDevice::device_info> devlist;
        _dev.EnumDevice(devlist);

        _init_dev();

        bool bUpdateStatus = false;
        /* 只查找第一个识别到的设备 */
        // 升级模式 
        if(HidDeviceHelper::OpenDevice(_dev, UPGRADE_NAME.c_str(), 0, &devlist) != DevHelper::EnumSUCCESS)
        {
            // 正常模式,需要发送指令切换状态 
            if(NORMAL_NAME.length() > 0 && HidDeviceHelper::OpenDevice(_dev, NORMAL_NAME.c_str(), 0, &devlist) == DevHelper::EnumSUCCESS)
            {
                LOGGER(_log.WriteLine("尝试将设备切换为升级模式"));
                Ref<IInteractiveTrans> refDev = _dev;
                _transUpdateCmd(refDev);

                _dev.Close();
            }
        }
        else
        {
            bUpdateStatus = true;
        }

        if(!bUpdateStatus)
        {
            _logErr(DeviceError::DevConnectErr, "连接设备失败");
        }

        return bUpdateStatus;
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /**
     * @brief 转换升级数据(将多行数据压缩打包以提高效率)
     */
    virtual bool ParseLine(const ByteArray& bin, ByteBuilder& updateBin)
    {
        /* 将多个bin数据直接拼成HID设备的整包 */
        size_t len = updateBin.GetLength() + bin.GetLength();
        if(len >= _dev.GetSendLength())
            return true;

        updateBin += bin;
        return false;
    }
    /**
     * @brief 升级每一行数据
     * 
     * - 设备返回包的第一个字节:
     *  - 90:成功 
     *  - 00:当前升级未完成,需要等待 
     *  - 63:升级失败,需要重新升级 
     * .
     */ 
    virtual bool UpdateLine(const ByteArray& bin)
    {
        // 直接发送数据包 
        if(_dev.Write(bin))
        {
            Timer timer;
            while(timer.Elapsed() < DEV_WAIT_TIMEOUT)
            {
                _tmpBuffer.Clear();
                if(!_dev.Read(_tmpBuffer))
                    return false;

                switch(_tmpBuffer[0])
                {
                case 0x00:
                    break;
                case 0x90:
                    return true;
                case 0x63:
                default:
                    return false;
                }
            }
        }

        return false;
    }
    //----------------------------------------------------- 
    /**
     * @brief 连接设备
     * @param [in] sArg 读卡器名称
     */
    virtual bool Link(const char* sArg = NULL)
    {
        LOG_FUNC_NAME();
        LOGGER(_dev.SelectLogger(GetLogger()));

        return _logRetValue(_PreLink(sArg));
    }
    /// 设备是否连接 
    virtual bool IsValid()
    {
        return _dev.IsOpen();
    }
    /* 使用DevUpdater提供的默认功能即可
    /// 准备升级数据
    virtual bool PreUpdate(const char* sPath) = 0;
    /// 更新固件数据
    virtual bool Update() = 0;
    */
    // 断开设备 
    virtual bool UnLink()
    {
        LOG_FUNC_NAME();
        _dev.Close();

        LOGGER(_dev.ReleaseLogger());

        return _logRetValue(true);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace updater
} // namespace application
} // namespace zhou_yb
//========================================================= 