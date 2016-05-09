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
/// ͨ��COM��Э����HID�����豸�Ƿ����
class HidComUpdateModeTestLinker : public TestLinker<FixedHidTestDevice>
{
protected:
    WinHidTestLinker _hidLinker;
public:
    /**
     * @brief ɨ�贮��,�����������л�ָ��
     * 
     * @param [in] dev ��Ҫ�������豸
     * @param [in] devArg ���� 
     * @param [in] printer �ı������
     */
    virtual bool Link(FixedHidTestDevice& dev, IArgParser<string, string>& arg, TextPrinter& printer)
    {
        LOGGER(printer.TextPrint(TextPrinter::TextLogger, "HidComUpdateModeTestLinker::Link"));

        string reader = "";
        string upgrade = "";

        ArgParser cfg;
        cfg.Parse(arg["Updater"].Value.c_str());
        upgrade = cfg["Name"].To<string>();

        cfg.Clear();
        cfg.Parse(arg["Reader"].Value.c_str());
        reader = cfg["Name"].To<string>();

        LOGGER(StringLogger stringlogger;
        stringlogger << "Updater:<" << upgrade
            << ">,Reader:<" << reader;
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
        if(!_hidLinker.Link(dev, cfg, printer))
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
/// ��������ģʽ�µ��豸
class HidComUpdateTestLinker : public WinHidTestLinker
{
public:
    virtual bool Link(FixedHidTestDevice& dev, IArgParser<string, string>& arg, TextPrinter& printer)
    {
        ArgParser cfg;
        cfg.Parse(arg["Updater"].Value.c_str());
        if(WinHidTestLinker::Link(dev, cfg, printer))
        {
            if(ComUpdateModeTestLinker::IsUpgradeMode(dev))
                return true;
            WinHidTestLinker::UnLink(dev, printer);
        }
        return false;
    }
};
//--------------------------------------------------------- 
class HidComUpdaterTestCase : public ITestCase<FixedHidTestDevice>
{
protected:
    /// ���������ļ�����
    size_t _updateCount;
    /// ����N�������״̬��
    size_t _swCount;
    /// ���͵���������
    ByteBuilder _updateBin;
public:
    HidComUpdaterTestCase(size_t swCount = 1) : _swCount(swCount) {}
    /// ������
    virtual bool Test(Ref<FixedHidTestDevice>& testObj, const ByteArray& testArg, TextPrinter&)
    {
        /* ��ʼ�������� */
        byte len = _itobyte(testArg.GetLength());
        _updateBin.Clear();
        _updateBin += len;
        _updateBin += testArg;
        bool bUpdate = testObj->Write(_updateBin);
        if(bUpdate)
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
} // namespace updater
} // namespace application
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_HIDCOMIC_DEVUPDATER_H_
//========================================================= 
