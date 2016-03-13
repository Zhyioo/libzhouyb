//========================================================= 
/**@file WinSdtApiInvoker.h 
 * @brief 身份证阅读区SDTApi.dll封装 
 * 
 * 接口返回的错误码为SDT标准的错误码(不是DeviceError的枚举类型)
 * 
 * @date 2014-04-14   16:21:04 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//--------------------------------------------------------- 
#include "../../../device/idcard/ISdtApi.h"
using zhou_yb::device::idcard::ISdtApi;
//--------------------------------------------------------- 
namespace zhou_yb {
namespace extension {
namespace ability {
//--------------------------------------------------------- 
/// stdapi.dll 功能封装 
class WinSdtApiInvoker : 
    public IBaseDevice, 
    public ISdtApi, 
    public DeviceBehavior,
    public RefObject
{
    //----------------------------------------------------- 
    /// 禁用拷贝构造函数 
    UnenableObjConsturctCopyBehavior(WinSdtApiInvoker);
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /// 转换错误码为文字信息 
    virtual const char* TransErrToString(int errCode) const
    {
        return ISdtApi::TransErrToString(errCode);
    }
    //----------------------------------------------------- 
    /* stdapi.dll 接口函数声明 */
    typedef int(__stdcall *DLL_FUNCTION(SDT_GetCOMBaud))(int iPort, uint* puiBaudRate);
    typedef int(__stdcall *DLL_FUNCTION(SDT_SetCOMBaud))(int iPort, uint uiCurrBaud, uint uiSetBaud);
    typedef int(__stdcall *DLL_FUNCTION(SDT_OpenPort))(int iPort);
    typedef int(__stdcall *DLL_FUNCTION(SDT_ClosePort))(int iPort);
    typedef int(__stdcall *DLL_FUNCTION(SDT_ResetSAM))(int iPort, int iIfOpen);
    typedef int(__stdcall *DLL_FUNCTION(SDT_SetMaxRFByte))(int iPort, byte ucByte, int bIfOpen);
    typedef int(__stdcall *DLL_FUNCTION(SDT_GetSAMStatus))(int iPort, int iIfOpen);
    typedef int(__stdcall *DLL_FUNCTION(SDT_GetSAMID))(int iPort, byte* pucSAMID, int iIfOpen);
    typedef int(__stdcall *DLL_FUNCTION(SDT_GetSAMIDToStr))(int iPort, char* pcSAMID, int iIfOpen);

    typedef int(__stdcall *DLL_FUNCTION(SDT_StartFindIDCard))(int iPort, byte* pucManaInfo, int iIfOpen);
    typedef int(__stdcall *DLL_FUNCTION(SDT_SelectIDCard))(int iPort, byte* pucManaMsg, int iIfOpen);
    typedef int(__stdcall *DLL_FUNCTION(SDT_ReadMngInfo))(int iPort, byte* pucManageMsg, int iIfOpen);
    typedef int(__stdcall *DLL_FUNCTION(SDT_ReadBaseMsg))(int iPort, byte* pucCHMsg, uint* puiCHMsgLen, byte* pucPHMsg, uint* puiPHMsgLen, int iIfOpen);
    typedef int(__stdcall *DLL_FUNCTION(SDT_ReadNewAppMsg))(int iPort, byte* pucAppMsg, uint* puiAppMsgLen, int iIfOpen);
    //----------------------------------------------------- 
protected:
    HMODULE _hDll;
    DEF_FUNCTION(SDT_GetCOMBaud);
    DEF_FUNCTION(SDT_SetCOMBaud);
    DEF_FUNCTION(SDT_OpenPort);
    DEF_FUNCTION(SDT_ClosePort);
    DEF_FUNCTION(SDT_ResetSAM);
    DEF_FUNCTION(SDT_SetMaxRFByte);
    DEF_FUNCTION(SDT_GetSAMStatus);
    DEF_FUNCTION(SDT_GetSAMID);
    DEF_FUNCTION(SDT_GetSAMIDToStr);
    DEF_FUNCTION(SDT_StartFindIDCard);
    DEF_FUNCTION(SDT_SelectIDCard);
    DEF_FUNCTION(SDT_ReadMngInfo);
    DEF_FUNCTION(SDT_ReadBaseMsg);
    DEF_FUNCTION(SDT_ReadNewAppMsg);

    ByteBuilder _dllpath;
    int _port;
    bool _isOpen;

    inline bool _is_success(int iRet)
    {
        return iRet == ISdtApi::SDT_Success || iRet == ISdtApi::SDT_FindCardSuccess;
    }
    void _init()
    {
        _hDll = NULL;
        _isOpen = false;
        _port = 0;

        SDT_GetCOMBaud = NULL;
        SDT_SetCOMBaud = NULL;
        SDT_OpenPort = NULL;
        SDT_ClosePort = NULL;
        SDT_ResetSAM = NULL;
        SDT_SetMaxRFByte = NULL;
        SDT_GetSAMStatus = NULL;
        SDT_GetSAMID = NULL;
        SDT_GetSAMIDToStr = NULL;
        SDT_StartFindIDCard = NULL;
        SDT_SelectIDCard = NULL;
        SDT_ReadMngInfo = NULL;
        SDT_ReadBaseMsg = NULL;
        SDT_ReadNewAppMsg = NULL;

        _lasterr = DeviceError::OperatorErr;
    }
public:
    WinSdtApiInvoker(const char* sdtapi = NULL) 
    {
        _init(); 
        iIfOpen = 0;

        Open(sdtapi);
    }
    virtual ~WinSdtApiInvoker() 
    {
        Close();
    }
    //----------------------------------------------------- 
    /** 
     * @brief 打开身份证设备 
     * @param [in] sArg [default:NULL] 
     * - 参数格式:
     *  - NULL 表示按照上一次的配置打开
     *  - SdtApi.dll文件路径 
     * .
     */
    virtual bool Open(const char* sArg = NULL)
    {
        LOG_FUNC_NAME();
        LOGGER(_log << "Open.Arg:<" << _strput(sArg) << ">\n");

        Close();
        
        if(_strlen(sArg) < 1 && _dllpath.IsEmpty())
            _dllpath = "sdtapi.dll";
        else
            _dllpath = sArg;

        CharConverter cvt;
        _hDll = LoadLibrary(cvt.to_char_t(_dllpath.GetString()));
        if(!IsOpen())
        {
            _errinfo = "加载库";
            _errinfo += _dllpath.GetString();
            _errinfo += "失败";

            _logErr(ISdtApi::SDT_ArgErr, "加载库失败");

            return _logRetValue(false);
        }

        LOAD_FUNCTION(_hDll, SDT_GetCOMBaud);
        LOAD_FUNCTION(_hDll, SDT_SetCOMBaud);
        LOAD_FUNCTION(_hDll, SDT_OpenPort);
        LOAD_FUNCTION(_hDll, SDT_ClosePort);
        LOAD_FUNCTION(_hDll, SDT_ResetSAM);
        LOAD_FUNCTION(_hDll, SDT_SetMaxRFByte);
        LOAD_FUNCTION(_hDll, SDT_GetSAMStatus);
        LOAD_FUNCTION(_hDll, SDT_GetSAMID);
        LOAD_FUNCTION(_hDll, SDT_GetSAMIDToStr);
        LOAD_FUNCTION(_hDll, SDT_StartFindIDCard);
        LOAD_FUNCTION(_hDll, SDT_SelectIDCard);
        LOAD_FUNCTION(_hDll, SDT_ReadMngInfo);
        LOAD_FUNCTION(_hDll, SDT_ReadBaseMsg);
        LOAD_FUNCTION(_hDll, SDT_ReadNewAppMsg);

        LOGGER(
        _log << "SDT_GetCOMBaud:<" << SDT_GetCOMBaud << ">\n"
            << "SDT_SetCOMBaud:<" << SDT_SetCOMBaud << ">\n"
            << "SDT_OpenPort:<" << SDT_OpenPort << ">\n"
            << "SDT_ClosePort:<" << SDT_ClosePort << ">\n"
            << "SDT_ResetSAM:<" << SDT_ResetSAM << ">\n"
            << "SDT_SetMaxRFByte:<" << SDT_SetMaxRFByte << ">\n"
            << "SDT_GetSAMStatus:<" << SDT_GetSAMStatus << ">\n"
            << "SDT_GetSAMID:<" << SDT_GetSAMID << ">\n"
            << "SDT_GetSAMIDToStr:<" << SDT_GetSAMIDToStr << ">\n"
            << "SDT_StartFindIDCard:<" << SDT_StartFindIDCard << ">\n"
            << "SDT_SelectIDCard:<" << SDT_SelectIDCard << ">\n"
            << "SDT_ReadMngInfo:<" << SDT_ReadMngInfo << ">\n"
            << "SDT_ReadBaseMsg:<" << SDT_ReadBaseMsg << ">\n"
            << "SDT_ReadNewAppMsg:<" << SDT_ReadNewAppMsg << ">\n");

        /* 加载函数 */
        if(NULL == SDT_GetCOMBaud ||
            NULL == SDT_SetCOMBaud ||
            NULL == SDT_OpenPort ||
            NULL == SDT_ClosePort ||
            NULL == SDT_ResetSAM ||
            NULL == SDT_SetMaxRFByte ||
            NULL == SDT_GetSAMStatus ||
            NULL == SDT_GetSAMID ||
            NULL == SDT_GetSAMIDToStr ||
            NULL == SDT_StartFindIDCard ||
            NULL == SDT_SelectIDCard ||
            NULL == SDT_ReadMngInfo ||
            NULL == SDT_ReadBaseMsg ||
            NULL == SDT_ReadNewAppMsg)
        {
            _errinfo = "加载函数失败";
            Close();

            _logErr(ISdtApi::SDT_ArgErr, "加载函数失败");

            return _logRetValue(false);
        }

        _errinfo = "成功";
        return _logRetValue(true);
    }
    /// 返回设备是否已经打开 
    virtual bool IsOpen()
    {
        return IsValid();
    }
    // 关闭设备 
    virtual void Close()
    {
        LOG_FUNC_NAME();

        if(IsValid())
        {
            FreeLibrary(_hDll);
        }
        _init();
        _logRetValue(true);
    }
    /// 是否有效 
    inline bool IsValid() const 
    {
        return _hDll != NULL;
    }
    //----------------------------------------------------- 
    virtual bool OpenPort(int nPort = 0)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log << "nPort:<" << nPort << ">\n");
        _port = nPort;

        int iRet = SDT_OpenPort(nPort);
        ASSERT_FuncErrInfoRet(_is_success(iRet), iRet, "SDT_OpenPort");

        return _logRetValue(true);
    }
    virtual bool ClosePort()
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        int iRet = SDT_ClosePort(_port);
        ASSERT_FuncErrInfoRet(_is_success(iRet), iRet, "SDT_ClosePort");

        return _logRetValue(true);
    }
    virtual bool ResetSAM()
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        int iRet = SDT_ResetSAM(_port, iIfOpen);
        ASSERT_FuncErrInfoRet(_is_success(iRet), iRet, "SDT_ResetSAM");

        return _logRetValue(true);
    }
    virtual bool CheckSAM() 
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        int iRet = SDT_GetSAMStatus(_port, iIfOpen);
        ASSERT_FuncErrInfoRet(_is_success(iRet), iRet, "SDT_GetSAMStatus");

        return _logRetValue(true);
    }
    virtual bool GetSamID(ByteBuilder& samID)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();
        
        // 16 
        byte id[32] = {0};
        int iRet = SDT_GetSAMID(_port, id, iIfOpen);
        ASSERT_FuncErrInfoRet(_is_success(iRet), iRet, "SDT_GetSAMID");
        
        ByteArray idArray(id, 16);
        samID.Append(idArray);
        
        LOGGER(
        _log<<"SAM ID:<";
        _log.WriteStream(idArray) << ">\n");

        return _logRetValue(true);
    }
    virtual bool FindIDCard(ByteBuilder* pManaInfo = NULL) 
    {
        LOG_FUNC_NAME();
        ASSERT_Device();
        
        byte manaInfoBuff[128] = {0};
        int iRet = SDT_StartFindIDCard(_port, manaInfoBuff, iIfOpen);
        ASSERT_FuncErrInfoRet(_is_success(iRet), iRet, "SDT_StartFindIDCard");
        
        ByteArray manaArray(manaInfoBuff, 16);
        if(pManaInfo != NULL)
            pManaInfo->Append(manaArray);
        
        LOGGER(
        _log<<"ManaInfo:<";
        _log.WriteStream(manaArray) << ">\n");

        return _logRetValue(true);
    }
    virtual bool SelectIDCard(ByteBuilder* pManaInfo = NULL) 
    {
        LOG_FUNC_NAME();
        ASSERT_Device();
        
        byte manaInfoBuff[128] = {0};
        int iRet = SDT_SelectIDCard(_port, manaInfoBuff, iIfOpen);
        ASSERT_FuncErrInfoRet(_is_success(iRet), iRet, "SDT_SelectIDCard");
        
        ByteArray manaArray(manaInfoBuff, 16);
        if(pManaInfo != NULL)
            pManaInfo->Append(manaArray);
        
        LOGGER(
        _log<<"ManaInfo:<";
        _log.WriteStream(manaArray) << ">\n");

        return _logRetValue(true);
    }
    virtual bool ReadBaseMsg(ByteBuilder* pTxtInfo, ByteBuilder* pPhotoInfo) 
    {
        LOG_FUNC_NAME();
        ASSERT_Device();
        
        // 256
        byte chMsg[300] = {0};
        uint chMsgLen = 0;
        // 1024
        byte phMsg[1048] = {0};
        uint phMsgLen = 0;
        
        int iRet = SDT_ReadBaseMsg(_port, chMsg, &chMsgLen, phMsg, &phMsgLen, iIfOpen);
        ASSERT_FuncErrInfoRet(_is_success(iRet), iRet, "SDT_ReadBaseMsg");
        
        ByteArray chMsgArray(chMsg, chMsgLen);
        ByteArray pgMsgArray(phMsg, phMsgLen);

        if(pTxtInfo != NULL)
            pTxtInfo->Append(chMsgArray);
        if(pPhotoInfo != NULL)
            pPhotoInfo->Append(pgMsgArray);
        
        LOGGER(
        _log<<"ChMsgLen:<"<<chMsgLen<<">,ChMsg:"<<endl;
        _log.WriteHex(chMsgArray) << "\n";
        _log<<"PhMsgLen:<"<<phMsgLen<<">,PhMsg:"<<endl;
        _log.WriteHex(pgMsgArray) << "\n");

        return _logRetValue(true);
    }
    virtual bool ReadAppendMsg(ByteBuilder& idAppendInfo) 
    {
        LOG_FUNC_NAME();
        ASSERT_Device();
        
        byte appMsg[512] = {0};
        uint appMsgLen = 0;
        
        int iRet = SDT_ReadNewAppMsg(_port, appMsg, &appMsgLen, iIfOpen);
        ASSERT_FuncErrInfoRet(_is_success(iRet), iRet, "SDT_ReadNewAppMsg");
        
        ByteArray appMsgArray(appMsg, appMsgLen);
        idAppendInfo.Append(appMsgArray);
        
        LOGGER(
        _log<<"AppendMsgLen:<"<<appMsgLen<<">,AppendMsg:"<<endl;
        _log.WriteHex(appMsgArray) << "\n");

        return _logRetValue(true);
    }
    //----------------------------------------------------- 
    /// 获取串口的波特率 
    bool GetCOMBaud(uint& baud)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        int iRet = SDT_GetCOMBaud(_port, &baud);
        ASSERT_FuncErrInfoRet(_is_success(iRet), iRet, "SDT_GetCOMBaud");

        LOGGER(_log << "Baud:<" << baud << ">\n");

        return _logRetValue(true);
    }
    /// 设置串口波特率 
    bool SetCOMBaud(uint uiCurrBaud, uint uiSetBaud)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log << "CurrentBaud:<" << uiCurrBaud << ">,SetBaud:<" << uiSetBaud << ">\n");

        int iRet = SDT_SetCOMBaud(_port, uiCurrBaud, uiSetBaud);
        ASSERT_FuncErrInfoRet(_is_success(iRet), iRet, "SDT_SetCOMBaud");

        return _logRetValue(true);
    }
    bool SetMaxRFByte(byte ucByte)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log << "RfByte:<" << ucByte << ">\n");
        int iRet = SDT_SetMaxRFByte(_port, ucByte, iIfOpen);
        ASSERT_FuncErrInfoRet(_is_success(iRet), iRet, "SDT_SetMaxRFByte");

        return _logRetValue(true);
    }
    /// 获取SAM V状态 
    bool GetSAMStatus()
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        int iRet = SDT_GetSAMStatus(_port, iIfOpen);
        ASSERT_FuncErrInfoRet(_is_success(iRet), iRet, "SDT_GetSAMStatus");

        return _logRetValue(true);
    }
    /// 获取SAM V ID
    bool GetSAMIDToStr(ByteBuilder& samIdAscii)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        char samID[64] = { 0 };
        int iRet = SDT_GetSAMIDToStr(_port, samID, iIfOpen);
        ASSERT_FuncErrInfoRet(_is_success(iRet), iRet, "SDT_GetSAMIDToStr");
        LOGGER(_log<<"SAM ID:<"<<samID<<">\n");

        samIdAscii += samID;

        return _logRetValue(true);
    }
    bool ReadMngInfo(ByteBuilder& manageMsg)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        byte msg[128] = { 0 };
        int iRet = SDT_ReadMngInfo(_port, msg, iIfOpen);
        ASSERT_FuncErrInfoRet(_is_success(iRet), iRet, "SDT_ReadMngInfo");

        ByteArray msgArray(msg, 32);
        manageMsg.Append(msgArray);

        LOGGER(
        _log<<"Mng Info:";
        _log.WriteLine(msgArray));

        return _logRetValue(true);
    }
    /// SDT接口中是否需要重新打开的标志 
    int iIfOpen;
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace ability
} // namespace extension
} // namespace zhou_yb
//========================================================= 