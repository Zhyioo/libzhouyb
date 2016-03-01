//========================================================= 
/**@file DevHelper.h 
 * @brief 设备操作辅助类
 * 
 * 一些设备传入的参数不是非常友好、方便，该文件提供一些简化这一
 * 操作的类，方便使用
 * 
 * @date 2012-04-04 11:09:52 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../../include/Base.h"
#include "../../../include/BaseDevice.h"
#include "../../../include/Device.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace ability {
//--------------------------------------------------------- 
/// 静态自动变量(在第一次使用时才分配空间,减小不使用到时的内存消耗)
#define AUTO_STATIC_BYTEBUILDER(name, buffsize) \
    static ByteBuilder name; \
    if(name.GetSize() < buffsize) \
    { \
        name.Resize(buffsize); \
    } \
    name.Clear()
//--------------------------------------------------------- 
/// 设备辅助类 
class DevHelper
{
protected:
    DevHelper() {}
public:
    /// 辅助功能函数错误码 
    enum ErrEnum
    {
        /// 操作错误(辅助操作过程中失败) 
        EnumERROR = -1,
        /// 操作成功
        EnumSUCCESS = 0,
        /// 操作失败(操作的代理对象返回失败)
        EnumFAILE = 1
    };
    /// 是否是成功的状态码 
    inline static bool IsHelperSuccess(ErrEnum errCode)
    {
        return errCode == EnumSUCCESS;
    }
    /// 转换操作结果 
    inline static ErrEnum ToHelperResult(bool bResult)
    {
        return bResult ? EnumSUCCESS : EnumFAILE;
    }
    /// 转换操作结果 
    inline static ErrEnum ToHelperResult(int errCode)
    {
        return (errCode == DeviceError::Success ? EnumSUCCESS : EnumFAILE);
    }
};
//--------------------------------------------------------- 
#ifndef NO_INCLUDE_ICCARD_SOURCE
/// IC卡设备辅助类 
class ICCardDeviceHelper
{
protected:
    ICCardDeviceHelper() {}
public:
    /// 交换Apdu,移除状态码,并返回SW是否为9000 
    static DevHelper::ErrEnum Apdu(ITransceiveTrans& dev, const ByteArray& sendBcd, ByteBuilder* recvBcd = NULL)
    {
        AUTO_STATIC_BYTEBUILDER(tmp, DEV_BUFFER_SIZE);

        if(NULL == recvBcd)
        {
            recvBcd = &tmp;
        }

        bool bRet = ICCardDevice::AutoApdu(dev, sendBcd, *recvBcd);
        DevHelper::ErrEnum enumResult = DevHelper::ToHelperResult(bRet);
        if(bRet)
        {
            ushort sw = ICCardLibrary::GetSW(*recvBcd);
            ICCardLibrary::RemoveSW(*recvBcd);
            
            bRet = ICCardLibrary::IsSuccessSW(sw);
            if(!bRet)
            {
                enumResult = DevHelper::EnumERROR;
            }
        }

        return enumResult;
    }
    /// 交换数据(Ascii),移除状态码,并返回SW是否为9000 
    static DevHelper::ErrEnum ApduAscii(ITransceiveTrans& dev, const char* sendAscii, char* recvAscii = NULL)
    {
        AUTO_STATIC_BYTEBUILDER(sendBuff, DEV_BUFFER_SIZE);
        AUTO_STATIC_BYTEBUILDER(recvBuff, DEV_BUFFER_SIZE);

        DevCommand::FromAscii(sendAscii, sendBuff);

        bool bRet = ICCardDevice::AutoApdu(dev, sendBuff, recvBuff);
        DevHelper::ErrEnum enumResult = DevHelper::ToHelperResult(bRet);
        if(bRet)
        {
            ushort sw = ICCardLibrary::GetSW(recvBuff);
            ICCardLibrary::RemoveSW(recvBuff);
            
            bRet = ICCardLibrary::IsSuccessSW(sw);
            if(!bRet)
            {
                enumResult = DevHelper::EnumERROR;
            }
        }
        if(bRet && NULL != recvAscii)
        {
            sendBuff.Clear();
            ByteConvert::ToAscii(recvBuff, sendBuff);
            size_t len = sendBuff.GetLength();
            memcpy(recvAscii, sendBuff.GetBuffer(), len);
            recvAscii[len] = 0;
        }

        return enumResult;
    }
    /// 交换APDU, 返回状态码(接收的数据中不再包含状态码)
    static ushort ApduSW(ITransceiveTrans& dev, const ByteArray& sendBcd, ByteBuilder* recvBcd = NULL)
    {
        AUTO_STATIC_BYTEBUILDER(tmp, DEV_BUFFER_SIZE);

        if(NULL == recvBcd)
        {
            recvBcd = &tmp;
        }

        if(!ICCardDevice::AutoApdu(dev, sendBcd, *recvBcd))
            return ICCardLibrary::UnValidSW;
        ushort sw = ICCardLibrary::GetSW(*recvBcd);
        ICCardLibrary::RemoveSW(*recvBcd);

        return sw;
    }
    /// 交换APDU,返回状态码(接收的数据中不再包含状态码)
    static ushort ApduAsciiSW(ITransceiveTrans& dev, const char* sendAscii, char* recvAscii = NULL)
    {
        AUTO_STATIC_BYTEBUILDER(sendBcd, DEV_BUFFER_SIZE);
        AUTO_STATIC_BYTEBUILDER(recvBcd, DEV_BUFFER_SIZE);

        DevCommand::FromAscii(sendAscii, sendBcd);

        ushort sw = ApduSW(dev, sendBcd, &recvBcd);
        if(0x00 != sw && (NULL != recvAscii))
        {
            sendBcd.Clear();
            ByteConvert::ToAscii(recvBcd, sendBcd);
            size_t len = sendBcd.GetLength();
            memcpy(recvAscii, sendBcd.GetBuffer(), sendBcd.GetLength());
            recvAscii[len] = 0;
        }
        return sw;
    }
};
#endif // NO_INCLUDE_ICCARD_SOURCE
//--------------------------------------------------------- 
#ifndef NO_INCLUDE_PCSC_SOURCE
/// CCID设备操作辅助类 
class CCID_DeviceHelper
{
protected:
    CCID_DeviceHelper() {}
public:
    /** 
     * @brief 给具有类似名称的IC卡设备上电 
     * @param [in] dev 需要代理操作的设备 
     * @param [in] devName 设备的子名称 
     * @param [out] pAtr [default:NULL] 获取到的IC卡ATR(为NULL则表示不需要获取) 
     * @param [in] index [default:0] 包含多个类似名称的索引,如果index索引超过pList范围,则依次上电直到有一个成功为止   
     * @param [in] pList [default:NULL] IC卡名称列表(为NULL则内部自动获取) 
     * @retval -1 查找设备失败 
     * @retval 0 上电成功 
     * @retval 1 上电失败  
     */ 
    static DevHelper::ErrEnum PowerOn(CCID_Device& dev, const char* devName, 
        ByteBuilder* pAtr = NULL, size_t index = 0, list<string>* pList = NULL)
    {
        LOG_OBJ_INIT(dev);
        LOG_FUNC_NAME();
        LOGGER(_log<<"devName:<"<<_strput(devName)<<">\n");

        list<string> _list;
        if(pList == NULL)
        {
            dev.EnumDevice(_list);
            pList = &_list;
        }
        list<string>::iterator itr;
        size_t count = 0;
        size_t devCount = pList->size();
        DevHelper::ErrEnum rlt = DevHelper::EnumERROR;

        for(itr = pList->begin();itr != pList->end(); ++itr)
        {
            if(StringConvert::Contains(itr->c_str(), devName, true))
            {
                LOGGER(_log<<"找到匹配["<<_strput(devName)<<"]的设备:<"<<(*itr)<<">,索引:<"<<count<<">\n");
                if(index == count || index >= devCount)
                {
                    if(dev.PowerOn(itr->c_str(), pAtr))
                    {
                        rlt = DevHelper::EnumSUCCESS;
                        break;
                    }
                    else
                    {
                        rlt = DevHelper::EnumFAILE;
                    }
                }

                ++count;
            }
        }
        
        // 在列表中没有找到指定的设备 
        if(itr == pList->end())
        {
            LOGGER(_log<<"没有找到名称类似["<<_strput(devName)<<"]的设备"<<endl);
            return DevHelper::EnumERROR;
        }

        return rlt;
    }
    /// 返回是否存在指定名称的设备 
    static DevHelper::ErrEnum IsExist(const char* devName, list<string>* pList = NULL)
    {
        list<string> _list;
        list<string>::iterator itr;

        CCID_Device dev;
        if(pList == NULL)
        {
            dev.EnumDevice(_list);
            pList = &_list;
        }

        for(itr = pList->begin();itr != pList->end(); ++itr)
        {
            if(StringConvert::Contains(itr->c_str(), devName, true))
                return DevHelper::EnumSUCCESS;
        }
        return DevHelper::EnumERROR;
    }
    /// 获取指定读卡器的状态
    static DevHelper::ErrEnum SCardStatus(const char* devName, DWORD& currentState, DWORD& eventState, list<string>* pList = NULL)
    {
        list<string> _list;
        list<string>::iterator itr;

        CCID_Device dev;
        if(pList == NULL)
        {
            dev.EnumDevice(_list);
            pList = &_list;
        }

        for(itr = pList->begin();itr != pList->end(); ++itr)
        {
            if(StringConvert::Contains(itr->c_str(), devName, true))
            {
                if(!dev.SCardReaderState(currentState, eventState, itr->c_str()))
                    return DevHelper::EnumFAILE;
                return DevHelper::EnumSUCCESS;
            }
        }
        return DevHelper::EnumERROR;
    }
};
#endif // NO_INCLUDE_PCSC_SOURCE
//--------------------------------------------------------- 
/// 磁条读写器操作逻辑 
class MagneticDevAdapterHelper
{
protected:
    MagneticDevAdapterHelper() {}
public:
    /** 
     * @brief 读取磁条信息 
     * @param [in] devAdapter 磁条读写器逻辑适配器  
     * @param [in] mode 磁条模式 
     * - 磁条模式 组合方式含有相关磁道标识则读取对应磁道 
     *  - "1" 读一磁道 
     *  - "2" 读二磁道 
     *  - "3" 读三磁道 
     *  - 如: "2" 只读取二磁道,"23" 同时读取二三磁道 "123" 同时读取一二三磁道 
     * .
     * @param [out] tr1 获取到的一磁道信息(为NULL表示不需要该磁道数据) 
     * @param [out] tr2 获取到的二磁道信息(为NULL表示不需要该磁道数据) 
     * @param [out] tr3 获取到的三磁道信息(为NULL表示不需要该磁道数据) 
     */ 
    static DevHelper::ErrEnum ReadMagneticCard(MagneticDevAdapter& devAdapter, 
        const char* mode, ByteBuilder* tr1, ByteBuilder* tr2, ByteBuilder* tr3)
    {
        LOG_OBJ_INIT(devAdapter);
        LOG_FUNC_NAME();
        LOGGER(_log<<"Mode:<"<<_strput(mode)<<">\n");

        size_t modelen = _strlen(mode);
        if(modelen < 1)
        {
            LOGGER(_log.WriteLine("磁条模式错误,ArgIsNullErr"));
            return DevHelper::EnumERROR;
        }

        MagneticDevAdapter::MagneticMode readMode = MagneticDevAdapter::UnKnownMode;
        uint tmpMode = 0;
        ByteArray modeArray(mode, modelen);
        if(StringConvert::ContainsChar(modeArray, '1'))
            tmpMode |= static_cast<uint>(MagneticDevAdapter::Track1);
        if(StringConvert::ContainsChar(modeArray, '2'))
            tmpMode |= static_cast<uint>(MagneticDevAdapter::Track2);
        if(StringConvert::ContainsChar(modeArray, '3'))
            tmpMode |= static_cast<uint>(MagneticDevAdapter::Track3);

        readMode = static_cast<MagneticDevAdapter::MagneticMode>(tmpMode);

        bool bRet = devAdapter.ReadTrack(readMode, tr1, tr2, tr3);
        return DevHelper::ToHelperResult(bRet);
    }
    static DevHelper::ErrEnum ReadMagneticCardWithCheck(MagneticDevAdapter& devAdapter, 
        const char* mode, ByteBuilder* tr1, ByteBuilder* tr2, ByteBuilder* tr3)
    {
        LOG_OBJ_INIT(devAdapter);
        LOG_FUNC_NAME();

        bool lastVal = devAdapter.IsCheckRetStatus;
        devAdapter.IsCheckRetStatus = true;
        DevHelper::ErrEnum ret = ReadMagneticCard(devAdapter, mode, tr1, tr2, tr3);
        devAdapter.IsCheckRetStatus = lastVal;

        return ret;
    }
    /** 
     * @brief 写入磁条信息 
     * @param [in] devAdapter 磁条读写器逻辑适配器  
     * @param [in] mode 磁条模式 
     * - 磁条模式 组合方式含有相关磁道标识则读取对应磁道 
     *  - "1" 写一磁道 
     *  - "2" 写二磁道 
     *  - "3" 写三磁道 
     *  - 如: "2" 只写入二磁道,"23" 同时写入二三磁道 "123" 同时写入一二三磁道 
     * .
     * @param [in] tr1 需要写入的一磁道信息(为NULL表示清空该磁道数据) 
     * @param [in] tr2 需要写入的二磁道信息(为NULL表示清空该磁道数据) 
     * @param [in] tr3 需要写入的三磁道信息(为NULL表示清空该磁道数据) 
     */ 
    static DevHelper::ErrEnum WriteMagneticCard(MagneticDevAdapter& devAdapter, 
        const char* mode, const char* tr1, const char* tr2, const char* tr3)
    {
        LOG_OBJ_INIT(devAdapter);
        LOG_FUNC_NAME();
        LOGGER(
        _log<<"Mode:<"<<_strput(mode)<<">\n"
            <<"Tr1:<"<<_strput(tr1)<<">\n"
            <<"Tr2:<"<<_strput(tr2)<<">\n"
            <<"Tr3:<"<<_strput(tr3)<<">\n");

        size_t modelen = _strlen(mode);
        if(modelen < 1)
        {
            LOGGER(_log.WriteLine("磁条模式错误,ArgIsNullErr"));
            return DevHelper::EnumERROR;
        }

        const char* tmp = "";
        const char* tr1Buff = NULL;
        const char* tr2Buff = NULL;
        const char* tr3Buff = NULL;
        ByteArray modeArray(mode, modelen);
        if(StringConvert::ContainsChar(modeArray, '1'))
            tr1Buff = ((tr1 == NULL) ? tmp : tr1);
        if(StringConvert::ContainsChar(modeArray, '2'))
            tr2Buff = ((tr2 == NULL) ? tmp : tr2);
        if(StringConvert::ContainsChar(modeArray, '3'))
            tr3Buff = ((tr3 == NULL) ? tmp : tr3);

        bool bRet = devAdapter.WriteTrack(tr1Buff, tr2Buff, tr3Buff);

        return DevHelper::ToHelperResult(bRet);
    }
    static DevHelper::ErrEnum WriteMagneticCardWithCheck(MagneticDevAdapter& devAdapter, 
        const char* mode, const char* tr1, const char* tr2, const char* tr3)
    {
        LOG_OBJ_INIT(devAdapter);
        LOG_FUNC_NAME();

        bool lastVal = devAdapter.IsCheckRetStatus;
        devAdapter.IsCheckRetStatus = true;
        DevHelper::ErrEnum ret = WriteMagneticCard(devAdapter, mode, tr1, tr2, tr3);
        devAdapter.IsCheckRetStatus = lastVal;

        return ret;
    }
};
//--------------------------------------------------------- 
} // namespace ability 
} // namespace extension 
} // namespace zhou_yb
//========================================================= 
