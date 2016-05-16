//========================================================= 
/**@file DevHelperForWin32.h 
 * @brief Windows下一些设备操作辅助类 
 * 
 * @date 2012-04-04   11:09:35 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_DEVHELPERFORWIN32_H_
#define _LIBZHOUYB_DEVHELPERFORWIN32_H_
//--------------------------------------------------------- 
#include "DevHelper.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace ability {
//--------------------------------------------------------- 
// 是否包含串口的头文件目录 
#ifndef NO_INCLUDE_COM_SOURCE
/// 串口辅助类 
class ComDeviceHelper
{
protected:
    ComDeviceHelper() {}
public:
    /**
     * @brief 打开串口 
     * 
     * @param [in] dev 需要操作的设备 
     * @param [in] port 需要打开的串口号
     * @param [in] baud 打开串口的波特率 
     */
    template<class TComDevice>
    static DevHelper::ErrEnum OpenDevice(TComDevice& dev, uint port, uint baud = CBR_9600)
    {
        LOG_OBJ_INIT(dev);
        LOG_FUNC_NAME();
        LOGGER(_log << "串口号:<" << port << ">,波特率:<" << baud << ">\n");

        if(dev.Open(port))
        {
            dev.SetProperty(baud);
            dev.SetComTimeout(DEV_OPERATOR_INTERVAL, 0, 0);
            return DevHelper::EnumSUCCESS;
        }

        return DevHelper::EnumFAILE;
    }
};
#endif
//--------------------------------------------------------- 
// 是否包含Socket头文件目录 
#ifdef INCLUDE_SOCKET_SOURCE
/// 蓝牙设备辅助类 
class BluetoothDeviceHelper
{
protected:
    BluetoothDeviceHelper() {}
public:
    /**
     * @brief 打开指定名称的蓝牙设备 
     * 
     * @param [in] dev 需要操作的设备 
     * @param [in] devName 需要连接的设备名称
     * @param [in] devPwd [default:NULL] 蓝牙配对的PIN码 
     * @param [in] timeoutMs [default:DEV_WAIT_TIMEOUT] 扫描远程蓝牙的超时时间
     * @param [in] isFullName [default:false] 名称是否完全匹配 
     */
    template<class TBluetoothDevice>
    static DevHelper::ErrEnum OpenDevice(BluetoothDevice& dev, const char* devName, const char* devPwd = NULL, uint timeoutMs = DEV_WAIT_TIMEOUT, bool isFullName = false)
    {
        LOG_OBJ_INIT(dev);
        LOG_FUNC_NAME();
        LOGGER(_log << "Name:<" << _strput(devName) << ">\n"
            << "Pin:<" << _strput(devPwd) << ">\n"
            << "IsFullName:<" << isFullName << ">\n");

        DevHelper::ErrEnum rlt = DevHelper::EnumSUCCESS;
        if(isFullName)
        {
            string arg = BluetoothParam::ToConfig(devName, devPwd);
            if(!dev.Open(arg.c_str()))
            {
                rlt = DevHelper::EnumFAILE;
            }
        }
        else
        {
            // 枚举本地蓝牙 
            list<BluetoothDevice::device_info> localdevlist;
            if(dev.EnumLocalDevice(localdevlist, true) < 1)
            {
                rlt = DevHelper::EnumERROR;
                LOGGER(_log.WriteLine("没有枚举到本地蓝牙设备"));
                return rlt;
            }
            // 枚举所有的远程设备 
            list<BluetoothDevice::device_info> devlist;
            // 没有枚举到设备 
            if(dev.EnumRemoteDevice(devlist, NULL, timeoutMs) < 1)
            {
                rlt = DevHelper::EnumERROR;
                LOGGER(_log.WriteLine("没有枚举到远程蓝牙设备"));
                return rlt;
            }

            list<BluetoothDevice::device_info>::iterator itr;
            ByteArray bthName(devName);
            for(itr = devlist.begin();itr != devlist.end(); ++itr)
            {
                if(StringConvert::Contains(ByteArray(itr->Name.c_str(), itr->Name.length()), bthName, true))
                    break;
            }

            if(itr == devlist.end())
            {
                rlt = DevHelper::EnumERROR;
                LOGGER(_log.WriteLine("没有找到匹配指定名称的蓝牙设备"));
                return rlt;
            }
            if(!dev.Open(localdevlist.front().hRadio, itr->Address, devPwd))
                rlt = DevHelper::EnumFAILE;
        }

        LOGGER(if(rlt != DevHelper::EnumSUCCESS) _log.WriteLine("打开设备失败"));
        return rlt;
    }
};
#endif
//--------------------------------------------------------- 
// 是否包含DDK的头文件目录 
#ifndef NO_INCLUDE_USB_SOURCE
/// HID设备辅助类 
class HidDeviceHelper
{
protected:
    HidDeviceHelper() {}
public:
    /**
     * @brief 打开指定VID,PID,版本号的HID设备 
     * @param [in] dev 需要代理操作的设备
     * @param [in] vid 设备的VID 
     * @param [in] pid 设备的PID 
     * @param [in] index [default:0] 连接多个设备时的索引号,如果超过pList范围则依次查找直到成功为止
     * @param [in] pList [default:NULL] 设备列表,如果为NULL则自动重新枚举
     */ 
    template<class THidDevice>
    static DevHelper::ErrEnum OpenDevice(THidDevice& dev, ushort vid, ushort pid,
        size_t index = 0, list<HidDevice::device_info>* pList = NULL)
    {
        LOG_OBJ_INIT(dev);
        LOG_FUNC_NAME();
        LOGGER(_log<<"VID:<"<<_hex(vid)<<">,PID:<"<<_hex(pid)<<">\n");

        list<HidDevice::device_info> devlist;
        if(pList == NULL)
        {
            dev.EnumDevice(devlist);
            pList = &devlist;
        }

        DevHelper::ErrEnum rlt = DevHelper::EnumERROR;
        size_t count = 0;
        size_t devCount = pList->size();
        list<HidDevice::device_info>::iterator itr;

        for(itr = pList->begin();itr != pList->end(); ++itr)
        {
            if((itr->Vid == vid) && (itr->Pid == pid))
            {
                LOGGER(_log << "找到匹配VID[" << _hex(vid)
                    << "],PID[" << _hex(pid) << "]的设备,索引:<" << count << ">\n");
                if(count == index || index >= devCount)
                {
                    if(dev.Open(itr->Path.c_str()))
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
        LOGGER(if(itr == pList->end())
        {
            if(rlt == DevHelper::EnumERROR)
                _log << "没有找到VID[" << _hex(vid) << "],PID[" << _hex(pid) << "]的设备" << endl;
            else
                _log.WriteLine("打开设备失败");
        });

        return rlt;
    }
    /**
     * @brief 打开指定名词的设备
     * @param [in] dev 需要代理操作的设备
     * @param [in] devName 设备名称(部分名称或全名)
     * @param [in] index [default:0] 连接多个设备时的索引号,如果超过pList范围则依次查找直到成功为止
     * @param [in] pList [default:NULL] 设备列表,如果为NULL则自动重新枚举 
     */
    template<class THidDevice>
    static DevHelper::ErrEnum OpenDevice(THidDevice& dev, const char* devName, size_t index = 0,
        list<HidDevice::device_info>* pList = NULL)
    {
        LOG_OBJ_INIT(dev);
        LOG_FUNC_NAME();
        LOGGER(_log<<"devName:<"<<_strput(devName)<<">\n");

        list<THidDevice::device_info> _list;
        if(pList == NULL)
        {
            dev.EnumDevice(_list);
            pList = &_list;
        }
        size_t devCount = pList->size();
        size_t count = 0;
        DevHelper::ErrEnum rlt = DevHelper::EnumERROR;
        list<THidDevice::device_info>::iterator itr;
        if(_is_empty_or_null(devName))
        {
            itr = pList->begin();
        }
        else
        {
            for(itr = pList->begin();itr != pList->end(); ++itr)
            {
                if(StringConvert::Contains(itr->Name.c_str(), devName, true))
                {
                    LOGGER(_log << "找到匹配[" << _strput(devName) << "]的设备<" << (itr->Name) << ">,索引:<" << count << ">\n");
                    if(index == count || index >= devCount)
                    {
                        if(dev.Open(itr->Path.c_str()))
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
        }
        // 在列表中没有找到指定的设备 
        if(itr == pList->end())
        {
            LOGGER(_log<<"没有找到类似名称["<<_strput(devName)<<"]的设备"<<endl);
            return DevHelper::EnumERROR;
        }

        return rlt;
    }
    /// 返回是否存在指定名称的设备 
    static DevHelper::ErrEnum IsExist(const char* devName, list<HidDevice::device_info>* pList = NULL)
    {
        HidDevice dev;
        list<HidDevice::device_info> _list;
        list<HidDevice::device_info>::iterator itr;

        if(pList == NULL)
        {
            dev.EnumDevice(_list);
            pList = &_list;
        }
        for(itr = pList->begin();itr != pList->end(); ++itr)
        {
            if(StringConvert::Contains(itr->Name.c_str(), devName, true))
                return DevHelper::EnumSUCCESS;
        }
        return DevHelper::EnumERROR;
    }
    /// 返回是否存在指定VID,PID的设备 
    static DevHelper::ErrEnum IsExist(ushort vid, ushort pid, list<HidDevice::device_info>* pList = NULL)
    {
        HidDevice dev;
        list<HidDevice::device_info> _list;
        list<HidDevice::device_info>::iterator itr;

        if(pList == NULL)
        {
            dev.EnumDevice(_list);
            pList = &_list;
        }

        for(itr = pList->begin();itr != pList->end(); ++itr)
        {
            if(itr->Vid == vid && itr->Pid == pid)
                return DevHelper::EnumSUCCESS;
        }
        return DevHelper::EnumERROR;
    }
};
#endif // ! NO_INCLUDE_USB_SOURCE
//--------------------------------------------------------- 
} // namespace ability 
} // namespace extension 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_DEVHELPERFORWIN32_H_
//========================================================= 
