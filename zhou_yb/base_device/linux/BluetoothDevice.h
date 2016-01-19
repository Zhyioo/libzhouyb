//========================================================= 
/**@file BluetoothDevice.h 
 * @brief Linux下蓝牙bluez设备操作逻辑 
 * 
 * 时间: 2015-06-10   19:07 
 * 作者: Zhyioo  
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../include/Base.h"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <linux/if.h>
#include <linux/if_bridge.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>
//--------------------------------------------------------- 
namespace zhou_yb {
namespace base_device {
namespace env_linux {
//--------------------------------------------------------- 
/// Linux下通过bluez方式操作的Bluethooth设备 
class BluetoothDevice
{
protected:
public:
	size_t EnumDevice(list<string>& devName, LoggerAdapter& _log)
	{
		size_t count = 0;

		struct hci_dev_list_req* pdl;
		struct hci_dev_req* pdr;
		struct hci_dev_info di;

		pdl = (struct hci_dev_list_req*)malloc(HCI_MAX_DEV * sizeof(struct hci_dev_req) + sizeof(int));
		if(pdl == NULL)
			return count;

		pdl->dev_num = HCI_MAX_DEV;
		pdr = pdl->dev_req;

		int ctl = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
		if(ctl < 0)
			goto out;

		if(ioctl(ctl, HCIGETDEVLIST, (void*)pdl) < 0)
			goto out;

		_log<<"dev num:"<<pdl->dev_num<<endl;

		for(int i = 0;i < pdl->dev_num; ++i)
		{
			di.dev_id = (pdr + i)->dev_id;
			if(ioctl(ctl, HCIGETDEVINFO, (void*)&di) < 0)
				goto out;
			_log<<"Name:"<<di.name<<endl;
		}
out:
		free(pdl);
		return count;
	}
};
//--------------------------------------------------------- 
} // namespace env_linux
} // namespace base_device
} // namespace zhou_yb
//========================================================= 
