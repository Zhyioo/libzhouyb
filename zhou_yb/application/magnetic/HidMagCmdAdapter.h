//========================================================= 
/**@file HidMagCmdAdapter.h
 * @brief ���ݲ������Ⱥʹ����ȵĴ���HID������
 * 
 * @date 2016-04-07   16:04:03
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_HIDMAGCMDADAPTER_H_
#define _LIBZHOUYB_HIDMAGCMDADAPTER_H_
//--------------------------------------------------------- 
#include "../../include/Base.h"

#include "../../device/cmd_adapter/HidCmdAdapter.h"
using zhou_yb::device::cmd_adapter::HidCmdAdapter;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace magnetic {
//--------------------------------------------------------- 
/**
 * @brief ����HID��ָ��������
 *
 * 1.֧�ֲ��������ֽڵĴ���ָ��
 * 2.֧��ָ�������N���ֽڵĴ���ָ��
 */
template<class THidDevice>
class HidMagCmdAdapter : public HidFixedCmdAdapter<THidDevice>
{
public:
    virtual bool Read(ByteBuilder& data)
    {
        LOG_FUNC_NAME();
        if(!BaseDevAdapterBehavior<THidDevice>::IsValid())
            return _logRetValue(false);
        // �ȶ���һ����
        ByteBuilder recv(64);
        if(!_pDev->Read(recv))
            return _logRetValue(false);
        
        // �����յ��������Ƿ��Դ�����ָ�ͷ

        return _logRetValue(true);
    }
};
//--------------------------------------------------------- 
} // namespace magnetic
} // namespace application 
} // namespace zhou_yb 
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_HIDMAGCMDADAPTER_H_
//========================================================= 