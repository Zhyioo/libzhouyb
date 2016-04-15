//========================================================= 
/**@file ComICC_TestContainer.h
 * @brief 串口IC卡接口适配器 
 * 
 * @date 2015-06-24   20:08:21
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_COMICC_TESTCONTAINER_H_
#define _LIBZHOUYB_COMICC_TESTCONTAINER_H_
//--------------------------------------------------------- 
#include "../TestFrame.h"

#include "../../../include/Device.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace test {
//--------------------------------------------------------- 
/// COM层指令集协议适配器 
typedef TestAdapterContainer<ComDevice, IInteractiveTrans, ComICCardCmdAdapter> ComICCardCmdTestContainer;
/// ComDevice转接触式读卡器适配器 
typedef TestAdapterExtractorContainer<ComICCardCmdTestContainer, IICCardDevice, ComContactICCardDevAdapter> ComContactIC_TestContainer;
/// ComDevice转非接触式读卡器适配器 
typedef TestAdapterExtractorContainer<ComICCardCmdTestContainer, IICCardDevice, ComContactlessICCardDevAdapter> ComContactlessIC_TestContainer;
/// ComDevice转PSAM读卡器适配器 
typedef TestAdapterExtractorContainer<ComICCardCmdTestContainer, IICCardDevice, ComPsamICCardDevAdapter> ComPsamIC_TestContainer;
//--------------------------------------------------------- 
/**
 * @brief COM AT88SC102卡接口转换器
 * @param CardNo 卡槽号
 */
template<class TContainer, byte CardNo = 0>
class ComTAT88SC102_TestContainer : public ITestContainer< typename TContainer::DeviceType, IMemoryCard >
{
protected:
    TContainer _testContainer;
    /// 放在内部方便添加日志 
    ComAT88SC102 _at88sc102IC;
public:
    virtual Ref<IMemoryCard> Adapter(const Ref<typename TContainer::DeviceType>& dev, IMemoryCard*, TextPrinter& printer)
    {
        // 识别卡并比较卡片类型 
        ComContactStorageCardDevAdapter::CardType cardType = ComAT88SC102::Card_Auto;
        typename TContainer::TransInterfaceType* pInterface = NULL;
        Ref<typename TContainer::TransInterfaceType> ref = _testContainer.Adapter(dev, pInterface, printer);
        _at88sc102IC.SelectDevice(ref);
        LOGGER(_at88sc102IC.SelectLogger(printer.GetLogger()));
        if(_at88sc102IC.DistinguishCardType(CardNo, &cardType) && cardType == ComAT88SC102::CardType)
            return _at88sc102IC;
        return Ref<IMemoryCard>();
    }
};
/// ComDevice转AT88SC102接口适配器 
typedef ComTAT88SC102_TestContainer<ComICCardCmdTestContainer> ComAT88SC102_TestContainer;
//--------------------------------------------------------- 
} // namespace test 
} // namespace application 
} // namespace zhou_yb 
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_COMICC_TESTCONTAINER_H_
//========================================================= 