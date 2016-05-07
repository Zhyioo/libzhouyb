//========================================================= 
/**@file TestLinkerHelper.h
 * @brief ������������������(���ض��Ľӿڸ�ֵ)
 * 
 * @date 2016-04-30   14:05:58
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_TESTLINKERHELPER_H_
#define _LIBZHOUYB_TESTLINKERHELPER_H_
//--------------------------------------------------------- 
#include "../TestFrame.h"
#include "../../../include/BaseDevice.h"
#include "../../../include/Device.h"
//--------------------------------------------------------- 
namespace zhou_yb {
namespace application {
namespace test {
//--------------------------------------------------------- 
/// ����ʽ�����ֽڵ�HID�豸
typedef TestDevice<HidDevice, HidFixedCmdAdapter<HidDevice> > FixedHidTestDevice;
//--------------------------------------------------------- 
/// ��������ƵĲ���������
class TestLinkerHelper
{
protected:
    TestLinkerHelper() {}
public:
    /**
     * @brief ��������ʱ������ָ��
     * @date 2016-04-30 14:21
     * 
     * @param [in] dev ��Ҫ�������豸
     * @param [in] arg ���������
     * - ����
     *  - SendComand [default:""] ���ӳɹ�����Ҫ���͵Ŀ���ָ��
     *  - RecvCommand [default:""] ���Ϳ���ָ���Ļ�Ӧ��
     * .
     * @param [in] printer �ı������
     */
    static bool LinkCommand(IInteractiveTrans& dev, IArgParser<string, string>& arg, TextPrinter& printer)
    {
        string cmd = arg["SendCommand"].To<string>();
        ByteBuilder cmdHex(8);
        DevCommand::FromAscii(cmd.c_str(), cmdHex);
        if(!cmdHex.IsEmpty())
        {
            if(dev.Write(cmdHex))
            {
                cmd = arg["RecvCommand"].To<string>();
                cmdHex.Clear();
                DevCommand::FromAscii(cmd.c_str(), cmdHex);
                if(!cmdHex.IsEmpty())
                {
                    // �ȽϽ��յ������Ƿ�һ��
                    ByteBuilder recv(8);
                    if(!dev.Read(recv))
                    {
                        LOGGER(printer.TextPrint(TextPrinter::TextLogger, "���ջ�Ӧ����ʧ��"));
                        return false;
                    }
                    if(!StringConvert::Compare(recv, cmdHex))
                    {
                        LOGGER(printer.TextPrint(TextPrinter::TextLogger, "��Ӧ���ݲ�ƥ��"));
                        return false;
                    }
                }
            }
        }
        return true;
    }
    /**
     * @brief ���ó�ʱʱ��Ͳ������
     * @date 2016-04-30 14:19
     * 
     * @param [in] dev ��Ҫ�������豸
     * @param [in] arg ��������� 
     * - ����
     *  - WaitTime [default:DEV_WAIT_TIMEOUT] ��ȡ�ĳ�ʱʱ��
     *  - OperatorInterval [default:DEV_OPERATOR_INTERVAL] ÿ�ζ�ȡ�ļ��
     * .
     * @param [in] printer �ı������
     */
    static bool LinkTimeoutBehavior(ITimeoutBehavior& dev, IArgParser<string, string>& arg, TextPrinter& printer)
    {
        uint waitTimeout = arg["WaitTime"].To<uint>(DEV_WAIT_TIMEOUT);
        uint operatorInterval = arg["OperatorInterval"].To<uint>(DEV_OPERATOR_INTERVAL);

        dev.SetOperatorInterval(operatorInterval);
        dev.SetWaitTimeout(waitTimeout);
        return true;
    }
};
//--------------------------------------------------------- 
} // namespace test 
} // namespace application 
} // namespace zhou_yb 
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_TESTLINKERHELPER_H_
//========================================================= 