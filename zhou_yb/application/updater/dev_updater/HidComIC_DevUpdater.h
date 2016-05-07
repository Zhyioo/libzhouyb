//========================================================= 
/**@file HidComIC_DevUpdater.h
 * @brief 
 * 
 * @date 2016-05-07   16:15:58
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_HIDCOMIC_DEVUPDATER_H_
#define _LIBZHOUYB_HIDCOMIC_DEVUPDATER_H_
//--------------------------------------------------------- 
#include "COM_IC_DevUpdater.h"
#include "../../test_frame/linker/WinHidTestLinker.h"
using zhou_yb::application::test::WinHidTestLinker;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace updater {
//--------------------------------------------------------- 
class HidComUpdateModeTestLinker : public TestLinker<FixedHidTestDevice>
{
protected:
    WinHidTestLinker _hidLinker;
public:
    /**
     * @brief ɨ�贮��,�����������л�ָ��
     * 
     * @param [in] dev ��Ҫ�������豸
     * @param [in] devArg ���� [COM]
     * @param [in] printer �ı������
     */
    virtual bool Link(FixedHidTestDevice& dev, const char* devArg, TextPrinter& printer)
    {
        LOGGER(printer.TextPrint(TextPrinter::TextLogger, "HidComUpdateModeTestLinker::Link"));
        string reader = "";
        string upgrade = "";
        string mode = "";

        ArgParser cfg;
        if(cfg.Parse(devArg))
        {
            cfg.GetValue("Boot", upgrade);
            cfg.GetValue("Name", reader);
            cfg.GetValue("TransmitMode", mode);
        }
        else
        {
            upgrade = _strput(devArg);
        }

        LOGGER(StringLogger stringlogger;
        stringlogger << "Updater:<" << upgrade
            << ">,Reader:<" << reader
            << ">,TransmitMode:<" << mode << ">";
        printer.TextPrint(TextPrinter::TextLogger, stringlogger.String().c_str()));

        list<HidDevice::device_info> devlist;
        list<HidDevice::device_info>::iterator itr;
        dev.Base().EnumDevice(devlist);

        for(itr = devlist.begin();itr != devlist.end(); ++itr)
        {
            if(StringConvert::Contains(ByteArray(itr->Name.c_str(), itr->Name.length()),
                ByteArray(upgrade.c_str(), upgrade.length()), true))
            {
                LOGGER(printer.TextPrint(TextPrinter::TextLogger, "Contains Updater"));
                return true;
            }
        }
        LOGGER(printer.TextPrint(TextPrinter::TextLogger, "Open Reader"));
        if(!_hidLinker.Link(dev, devArg, printer))
            return false;

        LOGGER(printer.TextPrint(TextPrinter::TextLogger, "Change Reader To Updater"));

        if(ComUpdateModeTestLinker::ToUpgradeMode(dev))
        {
            if(ComUpdateModeTestLinker::IsUpgradeMode(dev))
                return true;
        }
        dev.Base().Close();
        return false;
    }
    /// �رմ���
    virtual bool UnLink(FixedHidTestDevice& dev, TextPrinter&)
    {
        dev.Base().Close();
        return true;
    }
};
//--------------------------------------------------------- 
class HidComUpdaterTestCase : public ITestCase<IInteractiveTrans>
{
protected:
    /// ���������ļ�����
    size_t _updateCount;
    size_t _swCount;
public:
    HidComUpdaterTestCase(size_t swCount = 1) : _swCount(swCount) {}
    /// ������
    virtual bool Test(Ref<IInteractiveTrans>& testObj, const ByteArray& testArg, TextPrinter&)
    {
        // ���������Ѿ�������
        if(testArg.IsEmpty())
            return ComUpdateModeTestLinker::WaitSW(testObj);

        // �ȷ��ͳ���
        byte len = _itobyte(testArg.GetLength());
        // ֱ�ӷ������ݰ� 
        if(testObj->Write(ByteArray(&len, 1)) && testObj->Write(testArg))
        {
            ++_updateCount;
            // ��������N�������һ�����ж�״̬�� 
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
/// ����Э��HID�ӿ�IC���������̼��������� 
template<
    class TLinker, 
    class TContainer = TestContainer<FixedHidTestDevice, IInteractiveTrans>, 
    class TDecoder = UpdateDecoder>
class HidComIC_DevUpdater : public DevUpdater<FixedHidTestDevice, IInteractiveTrans, TLinker, TContainer, TDecoder>
{
protected:
    //----------------------------------------------------- 
    /// ���ܵ������ 
    ByteBuilder _random;
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    virtual bool PreTest(const char* preArg)
    {
        if(!DevUpdater::PreTest(preArg))
            return false;
        // ״̬׼��
        if(!ComUpdateModeTestLinker::IsUpgradeMode(_testInterface))
        {
            TextPrint(TextPrinter::TextError, "�豸״̬����");
            return false;
        }

        // ��ȡ�����
        _random.Clear();
        if(!ComUpdateModeTestLinker::GetRandom(_testInterface, _random))
        {
            TextPrint(TextPrinter::TextError, "��ȡ�����Կʧ��");
            return false;
        }
        
        // ͨѶ����
        if(!ComUpdateModeTestLinker::IsUpgradeReady(_testInterface))
        {
            TextPrint(TextPrinter::TextError, "ͨѶ����ʧ��");
            return false;
        }

        // ������������
        list<ByteBuilder>::iterator itr;
        for(itr = DevUpdater::_updateList.begin();itr != DevUpdater::_updateList.end(); ++itr)
        {
            ByteConvert::Xor(_random, *itr);
            if(!Interrupter.IsNull() && Interrupter->InterruptionPoint())
            {
                TextPrint(TextPrinter::TextError, "������ȡ��");
                return false;
            }
        }
        // ����һ�����б���ļ���������
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
#endif // _LIBZHOUYB_HIDCOMIC_DEVUPDATER_H_
//========================================================= 
