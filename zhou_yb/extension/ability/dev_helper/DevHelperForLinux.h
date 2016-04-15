//========================================================= 
/**@file DevHelperForLinux.h 
 * @brief Linux下设备操作辅助类 
 * 
 * @date 2012-04-04 11:09:52 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_DEVHELPERFORLINUX_H_
#define _LIBZHOUYB_DEVHELPERFORLINUX_H_
//---------------------------------------------------------  
#include "DevHelper.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace ability {
//--------------------------------------------------------- 
#ifndef NO_INCLUDE_USB_SOURCE
//--------------------------------------------------------- 
/// Linux下USB设备辅助类 
class UsbDeviceHelper
{
protected:
    UsbDeviceHelper() {}
public:
    /**
     * @brief 打开指定VID，PID的设备
     * @param [in] dev 需要代理操作的设备 
     * @param [in] vid 设备的VID
     * @param [in] pid 设备的PID 
     * @param [in] index [default:0] 连接多个设备时的索引号,如果超过pList范围则依次查找直到成功为止
     * @param [in] varId 3个字节部分组成: configId [default:0] 配置描述符 interfaceId [default:0] 接口描述符 altsetting [default:0] 接口配置描述符号
     * @param [in] pList [default:NULL] 设备列表,如果为NULL则自动重新枚举
     */
    template<class TUsbDevice>
    static DevHelper::ErrEnum OpenDevice(TUsbDevice& dev, ushort vid, ushort pid, int varId = 0, size_t index = 0, list<typename TUsbDevice::device_info>* pList = NULL)
    {
        LOG_OBJ_INIT(dev);
        LOG_FUNC_NAME();
        LOGGER(_log<<"VID:<"<<_hex(vid)<<">,PID:<"<<_hex(pid)<<">\n");

        list<typename TUsbDevice::device_info> tmplist;
        if(pList == NULL)
        {
            dev.EnumDevice(tmplist);
            pList = &tmplist;
        }

        bool isEqual = false;
        bool retCode = false;
        size_t devCount = pList->size();
        size_t count = 0;
        DevHelper::ErrEnum rlt = DevHelper::EnumERROR;
        typename list<typename TUsbDevice::device_info>::iterator itr;
        for(itr = pList->begin();itr != pList->end(); ++itr)
        {
            isEqual = (USB_VID(*itr) == vid) && (USB_PID(*itr) == pid);
            if(isEqual)
            {
                LOGGER(_log << "找到VID[" << _hex(vid) << "],PID[" << _hex(pid) << "]的设备,索引:<" << count << ">\n");
                if(count == index || index >= devCount)
                {
                    string sArg = UsbHandlerFactory::ToArg(*itr, varId);
                    if(dev.Open(sArg.c_str()))
                    {
                        rlt = DevHelper::EnumSUCCESS;
                        retCode = true;
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
        LOGGER(_log<<"打开VID["<<_hex(vid)<<"],PID["<<_hex(pid)<<"],VarID:["<<_hex(varId)<<"]的设备";_log<<(retCode?"成功\n":"失败\n"));

        return rlt;
    }
    /**
     * @brief 打开指定名词的设备
     * @param [in] dev 需要代理操作的设备 
     * @param [in] devName 设备名称(部分名称或全名) 
     * @param [in] index [default:0] 连接多个设备时的索引号,如果超过pList范围则依次查找直到成功为止 
     * @param [in] pList [default:NULL] 设备列表,如果为NULL则自动重新枚举 
     */
    template<class TUsbDevice>
    static DevHelper::ErrEnum OpenDevice(TUsbDevice& dev, const char* devName, size_t index = 0, list<typename TUsbDevice::device_info>* pList = NULL)
    {
        LOG_OBJ_INIT(dev);
        LOG_FUNC_NAME();
        LOGGER(_log<<"devName:<"<<_strput(devName)<<">\n");

        bool retCode = false;
        list<typename TUsbDevice::device_info> tmplist;
        if(pList == NULL)
        {
            dev.EnumDevice(tmplist);
            pList = &tmplist;
        }

        usb_dev_handle *udev = NULL;

        int ret = 0;
        char strMsg[256];
        size_t devCount = pList->size();
        size_t count = 0;
        DevHelper::ErrEnum rlt = DevHelper::EnumERROR;

        typename list<typename TUsbDevice::device_info>::iterator itr;
        for(itr = pList->begin();itr != pList->end(); ++itr)
        {
            udev = usb_open(*itr);
            if(NULL == udev)
            {
                LOGGER(_log<<"打开设备:VID<"<<_hex(USB_VID(*itr))<<">,PID<"<<_hex(USB_PID(*itr))<<">失败\n");
                continue;
            }

            LOGGER(_log<<"iProduct:<"<<_hex((*itr)->descriptor.iProduct)<<">\n");
            if((*itr)->descriptor.iProduct)
            {
                strMsg[0] = 0;
                ret = usb_get_string_simple(udev, (*itr)->descriptor.iProduct, strMsg, sizeof(strMsg));
                LOGGER(_log<<"ret=["<<ret<<"],设备名称:<"<<(strMsg)<<">\n");
            }

            usb_close(udev);
            udev = NULL;

            if (ret > 0 && StringConvert::Contains(strMsg, devName, true))
            {
                LOGGER(_log << "找到匹配[" << _strput(devName) << "]的设备<" << (strMsg) << ">,索引:<" << count << ">\n");
                if(index == count || index >= devCount)
                {
                    string sArg = UsbHandlerFactory::ToArg(*itr);
                    if(dev.Open(sArg.c_str()))
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
        if(itr == pList->end())
        {
            LOGGER(_log<<"没有找到类似名称["<<_strput(devName)<<"]的设备"<<endl);
            return DevHelper::EnumERROR;
        }

        return rlt;
    }
};
//--------------------------------------------------------- 
#endif // NO_INCLUDE_USB_SOURCE
//--------------------------------------------------------- 
} // namespace ability 
} // namespace extension 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_DEVHELPERFORLINUX_H_
//========================================================= 
