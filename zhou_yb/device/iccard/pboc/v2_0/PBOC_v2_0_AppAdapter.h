//========================================================= 
/**@file PBOC_v2_0_AppAdapter.h 
 * @brief PBOC 2.0 IC卡应用适配器 
 * 
 * PBOC 2.0 IC卡应用交互适配器(常用的通用操作)
 * 
 * @date 2014-10-17   17:31:55 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_PBOC_V2_0_APPADAPTER_H_
#define _LIBZHOUYB_PBOC_V2_0_APPADAPTER_H_
//--------------------------------------------------------- 
#include "../../base/ICCardAppAdapter.h"
#include "PBOC_v2_0_CMD.h"

#include "../v1_0/PBOC_v1_0_Library.h"
#include "PBOC_v2_0_Library.h"
using namespace zhou_yb::device::iccard::pboc::PBOC_Library;
//---------------------------------------------------------
namespace zhou_yb {
namespace device {
namespace iccard {
namespace pboc {
namespace v2_0 {
//---------------------------------------------------------
/* PBOC2.0中所定义的标签 */
/// PSE
#define SYS_PBOC_V2_0_PSE      "1PAY.SYS.DDF01"
/// PPSE(非接扩展应用)
#define SYS_PBOC_V2_0_PPSE     "2PAY.SYS.DDF01"
/// 金融主AID
#define SYS_PBOC_V2_0_BASE_AID "A000000333"
/// 终端默认数据
#define SYS_PBOC_V2_0_TERMINAL_TAGVALUE "9F1A 02 0156 95 05 0000008000 9F66 04 F4000000 9F71 01 01 9F7B 06 000001000000 9F33 02 0000"
/// PBOC2.0中能通过GET DATA命令获取的标签
#define SYS_PBOC_V2_0_GETDATA_TAGLIST \
    "9F36 9F13 9F17 9F4F 9F79 9F77 9F74 9F78 9F6D 9F42 9F51 9F52 9F53 9F72 9F54 9F75 9F5C 9F73 9F56 5F28 9F57 9F14 9F58 9F23 9F59 9F76"
/// PBOC2.0中,55域必选项 
#define SYS_PBOC_V2_0_DEFAULT_ARQC_TAGLIST \
    "9F26 9F27 9F10 9F37 9F36 95 9F13 9A 9C 9F02 5F2A 5A 5F34 82 9F1A 9F03 9F33"
/// PBOC2.0中,写卡后返回的55域必选项 
#define SYS_PBOC_V2_0_DEFAULT_TC_TAGLIST \
    "9F26 9F27 9F10 9F37 9F36 95 9F13 9A 9C 9F02 5F2A 5A 5F34 82 9F1A 9F03 9F33"
//--------------------------------------------------------- 
/* 如果用户需要增加其他的标签,则通过重新定义以下宏实现 */
/*
#define PBOC_V2_0_GETDATA_TAGLIST \
    SYS_PBOC_V2_0_GETDATA_TAGLIST \ 
    "DF4F"
*/
/// 金融IC主应用AID 
#ifndef PBOC_V2_0_BASE_AID
#   define PBOC_V2_0_BASE_AID SYS_PBOC_V2_0_BASE_AID
#endif 
//--------------------------------------------------------- 
/// IC卡终端常用数据默认值
#ifndef PBOC_V2_0_TERMINAL_TAGVALUE
#   define PBOC_V2_0_TERMINAL_TAGVALUE SYS_PBOC_V2_0_TERMINAL_TAGVALUE
#endif
//--------------------------------------------------------- 
/// IC卡中需要用取数据指令获取的数据标签 
#ifndef PBOC_V2_0_GETDATA_TAGLIST
#   define PBOC_V2_0_GETDATA_TAGLIST SYS_PBOC_V2_0_GETDATA_TAGLIST
#endif
//--------------------------------------------------------- 
/// IC卡中银联规范定义的55域报文标签必选项 
#ifndef PBOC_V2_0_DEFAULT_ARQC_TAGLIST
#   define PBOC_V2_0_DEFAULT_ARQC_TAGLIST SYS_PBOC_V2_0_DEFAULT_ARQC_TAGLIST
#endif
//--------------------------------------------------------- 
/// IC卡中银联规范定义的55域写卡结果报文标签必选项 
#ifndef PBOC_V2_0_DEFAULT_TC_TAGLIST
#   define PBOC_V2_0_DEFAULT_TC_TAGLIST SYS_PBOC_V2_0_DEFAULT_TC_TAGLIST
#endif
//--------------------------------------------------------- 
/**
 * @brief IC卡应用交互适配器
 *
 * 流程控制(控制指令的收发和解析)->只允许继承使用
 */
class PBOC_v2_0_BaseAdapter : public ICCardAppAdapter
{
public:
    //-----------------------------------------------------
    /// 错误枚举
    enum ICCardBaseErr
    {
        /// SW错误
        SW_Err = DeviceError::ENUM_MAX(ErrorCode),
        /// 标签不存在
        TagNotExistErr,
        /// 找不到记录
        NoRecodeErr,
        /// 应用被锁定 
        AidIsLockedErr,
        /// 错误最大值(在嵌套的设备适配器中叠加使用)
        ENUM_MAX(ICCardBaseErr)
    };
    //-----------------------------------------------------
    /// 获取错误代码所表示的错误信息
    virtual const char* TransErrToString(int errCode) const
    {
        if(errCode < DeviceError::ENUM_MAX(ErrorCode))
            return DeviceError::TransErrToString(errCode);

        switch(errCode)
        {
        case SW_Err:
            return "SW错误";
        case TagNotExistErr:
            return "没有指定的标签";
        case NoRecodeErr:
            return "没有指定的记录";
        default:
            return "无法识别的错误";
        }
    }
    //-----------------------------------------------------
protected:
    //-----------------------------------------------------
    /// PSE
    ByteBuilder _defaultPSE;
    //-----------------------------------------------------
    //@{
    /**@name
     * @brief 读指定记录号的记录recodenum=[1,254]
     */
    /// 读单独的一条记录 
    bool _read_recode(byte sfi, byte recodenum, ByteBuilder& recode)
    {
        ByteBuilder cmd(64);
        /* 读记录命令 */ 
        ReadRecodeCmd::Make(cmd, sfi, recodenum);

        bool bRet = _apdu(cmd, recode);

        return bRet;
    }
    /// 读多条记录，结果保存到一个大的缓冲区中，返回成功读取的记录数
    size_t _read_recode_s(byte sfi, byte start, 
        byte end, ByteBuilder& recode)
    {
        ByteBuilder cmd(8);
        size_t count = 0;
        /* 读记录命令 */ 
        ByteBuilder buf(DEV_BUFFER_SIZE);
        ReadRecodeCmd::Make(cmd, sfi, start);

        for(byte i = start;i <= end; ++i)
        {
            cmd[ICCardLibrary::P1_INDEX] = i;
            if(_apdu(cmd, buf))
            {
                recode.Append(buf);
                ++count;
            }
            else
            {
                LOGGER(_log<<"记录号:<"<<static_cast<ushort>(i)<<">读取失败\n");
            }
            buf.Clear();
        }
        return count;
    }
    /// 读多条记录，结果按照顺序保存在一个链表中，返回成功读取的记录数
    size_t _read_recode_l(byte sfi, byte start, 
        byte end, list<ByteBuilder>& _list)
    {
        LOGGER(_log.WriteLine("获取标签[1,255)"));

        ByteBuilder cmd(64);
        size_t count = 0;
        /* 读记录命令 */ 
        ReadRecodeCmd::Make(cmd, sfi, start);
        _list.push_back(ByteBuilder(DEV_BUFFER_SIZE));

        for(byte i = start;i <= end; ++i)
        {
            LOGGER(_log<<"取标签:<"<<static_cast<ushort>(i)<<">\n");

            cmd[ICCardLibrary::P1_INDEX] = i;
            if(_apdu(cmd, _list.back()))
            {
                _list.push_back(ByteBuilder(DEV_BUFFER_SIZE));
                ++count;
            }
            else
            {
                _list.back().Clear();
                break;
            }
        }
        // 最坏的情况下所有记录都读取失败
        if(count < 1)
            _list.pop_back();

        return count;
    }
    /// 解析AID 
    bool _parse_template_aid(TlvElement& tagElement, list<PBOC_Library::AID>& _list)
    {
        TlvHeader tmpHeader = PBOC_v2_0_TagMap::GetHeader(PBOC_v2_0_TagMap::AID_Symbol);
        TlvElement subElement = tagElement.Select(tmpHeader);
        if(subElement.IsEmpty())
            return false;

        // 有标签的话再尝试获取应用优先级
        _list.push_back(PBOC_Library::AID());
        subElement.GetValue(_list.back().aid);

        LOGGER(_log << "获取到AID:\n" << _list.back().aid << endl);

        ByteBuilder aidPriority(8);
        tmpHeader = PBOC_v2_0_TagMap::GetHeader(PBOC_v2_0_TagMap::AID_Priority);
        subElement = tagElement.Select(tmpHeader);
        if(!subElement.IsEmpty())
        {
            subElement.GetValue(aidPriority);

            _list.back().priority = ((aidPriority.GetLength() > 0) ? aidPriority[0] : 0);
            LOGGER(_log.WriteLine("AID优先级:");
            _log_aid_property(_list.back().priority));
        }

        tmpHeader = PBOC_v2_0_TagMap::GetHeader(PBOC_v2_0_TagMap::ApplicationLabels);
        subElement = tagElement.Select(tmpHeader);
        if(!subElement.IsEmpty())
        {
            subElement.GetValue(_list.back().name);
            LOGGER(_log << "AID名称:\n" << _list.back().name << "\n");
        }

        return true;
    }
    //@}
    //----------------------------------------------------- 
    // PBOC第六部分操作流程
    //@{
    /**@name
     * @brief 应用选择
     */
    /// 获取应用列表(从目录下进行选择)
    bool pboc_get_pse_aid(const ByteArray& pse, list<PBOC_Library::AID>& _list)
    {
        LOG_FUNC_NAME();
        LOGGER(
        _log.WriteLine("PSE:");
        _log.WriteLine(pse));

        /* 选文件 */
        ByteBuilder cmd(64);
        SelectCmd::Make(cmd, pse, SelectCmd::First);

        /* 交换APDU */
        ByteBuilder recv(DEV_BUFFER_SIZE);
        LOGGER(_log.WriteLine("选文件..."));
        ASSERT_FuncInfo(_apdu(cmd, recv), "目录选AID列表");

        /* 查找SFI标签 */
        TlvElement root = TlvElement::Parse(recv);
        // 解析外部缓冲区中的标签
        LOGGER(_log.WriteLine("查找SFI..."));
        TlvHeader sfiHeader = PBOC_v2_0_TagMap::GetHeader(PBOC_v2_0_TagMap::SFI);
        ByteBuilder sfi(16);
        TlvElement tagElement = root.Select(sfiHeader);
        bool bRet = !tagElement.IsEmpty();
        if(bRet)
        {
            tagElement.GetValue(sfi);
            LOGGER(_log << "SFI:<";
            _log.WriteStream(sfi) << ">\n");
        }
        // 找不到标签或标签值为空
        if(!bRet || sfi.IsEmpty())
        {
            LOGGER(_log<<"标签:<"<<TlvConvert::ToHeaderAscii(sfiHeader)<<">\n");
            _logErr(TagNotExistErr, "找不到SFI标签:(88)");
            return false;
        }

        /* 读记录 */
        LOGGER(_log.WriteLine("读记录..."));
        list<ByteBuilder> recodeList;
        bRet = (_read_recode_l(sfi[0], 1, 254, recodeList) > 0);
        //  获取列表失败
        ASSERT_FuncInfo(bRet, "获取记录列表");
        /* 解析出AID */
        LOGGER(_log.WriteLine("解析AID..."));
        list<ByteBuilder>::iterator itr;
        size_t aidCount = 0;
        TlvHeader tmpHeader;
        TlvHeader templateHeader = PBOC_v2_0_TagMap::GetHeader(PBOC_v2_0_TagMap::ApplicationTemplate);
        for(itr = recodeList.begin();itr != recodeList.end(); ++itr)
        {
            root = TlvElement::Parse(*itr);
            // 查找模板
            tagElement = root.SelectAfter(templateHeader);
            if(!tagElement.IsEmpty())
            {
                while(_parse_template_aid(tagElement, _list))
                {
                    ++aidCount;
                    tagElement = root.SelectAfter(templateHeader);
                    if(tagElement.IsEmpty())
                        break;
                }
            }
            else
            {
                // 不符合61模板
                if(_parse_template_aid(root, _list))
                    ++aidCount;
            }

            // 嵌套的DDF入口
            do
            {
                LOGGER(_log.WriteLine("DDF入口获取AID..."));
                ByteBuilder tagDDFName(64);
                tmpHeader = PBOC_v2_0_TagMap::GetHeader(PBOC_v2_0_TagMap::DDF);
                tagElement = root.Select(tmpHeader);
                bRet = !tagElement.IsEmpty();

                if(bRet)
                {
                    tagElement.GetValue(tagDDFName);
                    pboc_get_pse_aid(tagDDFName, _list);
                }
            } while (bRet);
        }
        
        LOGGER(_log<<"获取到的AID数目:<"<<aidCount<<">\n");

        return (aidCount > 0);
    }
    /**
     * @brief 获取应用列表(根据终端AID选择,支持部分文件名选择) 
     * @param [in] aid 获取AID列表的入口AID名称 
     * @param [out] _list 获取到的AID列表 
     * @param [out] pAidData [default:NULL] 选取AID后返回的数据
     * @param [in] allowLocked [default:true] 是否允许选择被锁的AID 
     * @param [in] needSame [default:false] 是否需要进行完全匹配选择 
     * @param [in] equalAny [default:false] 是否在找到第一个匹配的AID后返回 
     */
    bool pboc_get_child_aid(const ByteArray& aid, 
        list<PBOC_Library::AID>& _list, 
        list<ByteBuilder>* pAidData = NULL,
        bool allowLocked = true,
        bool needSame = false,
        bool equalAny = false)
    {
        /* 输出入口参数 */
        LOG_FUNC_NAME();
        LOGGER(
        _log<<"查找的AID:"<<endl;
        _log.WriteLine(aid);
        _log<<"查找模式: \n"
            <<"是否完全匹配:<"<<needSame<<">\n"
            <<"是否找到第一个后退出:<"<<equalAny<<">"<<endl);

        const ushort LockedAidSW = ICCardLibrary::SelectedFileNotVaild;

        // 选文件
        ByteBuilder cmd(16);
        ByteBuilder recv(DEV_BUFFER_SIZE);
        ByteBuilder tmpAid(16);
        ushort sw = 0;
        bool compareVal = false;
        size_t aidCount = 0;
        TlvHeader tmpHeader;

        SelectCmd::Make(cmd, aid, SelectCmd::First);
        LOGGER(_log.WriteLine("选文件..."));
        sw = _apdu_s(cmd, recv);
        // 有对应应用
        while(sw == LockedAidSW || ICCardLibrary::IsSuccessSW(sw))
        {
            /* 有的卡选AID后只返回62 83,需要用单独的指令获取,而有的卡返回前面的数据+6283状态码 */
            // 应用被锁定,并且只返回状态码62 83,尝试用00 C0 00 00 00再次获取SFI信息
            if(allowLocked && sw == LockedAidSW && recv.GetLength() < 1)
            {
                recv.Clear();
                cmd.Clear();
                LOGGER(_log.WriteLine("卡片应用被锁定,获取AID信息..."));
                ByteConvert::FromAscii("00C0000000", cmd);
                _apdu_s(cmd, recv);
            }
            LOGGER(_log.WriteLine("解析AID..."));
            // 解析数据
            TlvElement root = TlvElement::Parse(recv);
            if(root.IsEmpty())
            {
                _logErr(DeviceError::RecvFormatErr, "选AID");
                break;
            }
            // 查找应用标签
            TlvElement tagElement = root.Select(PBOC_v2_0_TagMap::GetHeader(PBOC_v2_0_TagMap::DF_Name));
            if(tagElement.IsEmpty())
            {
                _logErr(NoRecodeErr, "选AID 0x84");
                break;
            }
            /* 到此为止已经选择了一个AID,获取到AID的FCI,sw标识了AID是否被锁定 */
            tmpAid.Clear();
            tagElement.GetValue(tmpAid);

            LOGGER(_log.WriteLine("AID匹配..."));
            compareVal = StringConvert::StartWith(tmpAid, aid);
            // 部分匹配上 
            if(compareVal)
            {
                LOGGER(_log.WriteLine("部分匹配..."));
                // 需要进行完全匹配,但是不匹配或只有部分匹配 
                if(needSame && tmpAid.GetLength() != aid.GetLength())
                    break;
                /* 此时AID为部分匹配 */
                if(allowLocked || sw != LockedAidSW)
                {
                    _list.push_back(PBOC_Library::AID());
                    _list.back().aid = tmpAid;
                    ++aidCount;

                    tmpHeader = PBOC_v2_0_TagMap::GetHeader(PBOC_v2_0_TagMap::AID_Priority);
                    tagElement = root.SelectAfter(tmpHeader);
                    if(!tagElement.IsEmpty())
                    {
                        tmpAid.Clear();
                        tagElement.GetValue(tmpAid);

                        _list.back().priority = ((tmpAid.GetLength() > 0) ? tmpAid[0] : 0);

                        LOGGER(_log.WriteLine("AID优先级:");
                        _log_aid_property(_list.back().priority));
                    }

                    tmpHeader = PBOC_v2_0_TagMap::GetHeader(PBOC_v2_0_TagMap::ApplicationLabels);
                    tagElement = root.SelectAfter(tmpHeader);
                    if(!tagElement.IsEmpty())
                    {
                        tagElement.GetValue(_list.back().name);
                        LOGGER(_log<<"AID名称:\n"<<_list.back().name<<"\n");
                    }

                    /* 拷贝选AID后的数据 */
                    if(NULL != pAidData)
                    {
                        pAidData->push_back(recv);
                        // 补上删除的状态码,如果应用被锁定的话状态码被还原成6283 
                        ByteConvert::FromObject(sw, pAidData->back());
                    }

                    LOGGER(
                    _log.WriteLine("添加AID(匹配):");
                    _log.WriteLine(_list.back().aid));
                    // 找到第一个匹配的AID项  
                    if(equalAny)
                        break;
                }
            }

            /* 下一个部分匹配的AID */
            LOGGER(_log.WriteLine("选择下一个AID..."));
            cmd.Clear();
            SelectCmd::Make(cmd, aid, SelectCmd::Next);

            recv.Clear();
            sw = _apdu_s(cmd, recv);
        }// while

        LOGGER(_log<<"获取到的AID数目:<"<<aidCount<<">\n");

        return (aidCount > 0);
    }
    /**
     * @brief 通过指定的AID进行部分匹配获取应用列表 
     * @param [in] aid 传入的AID,为空则表示从PSE开始选择
     * @param [out] _list 获取到的AID列表 
     * @param [out] pAidData [default:NULL] 选AID时返回的数据,如果有,则顺序与_list中一致  
     */ 
    bool pboc_enum_aid_list(const ByteArray& aid, 
        list<PBOC_Library::AID>& _list, list<ByteBuilder>* pAidData = NULL)
    {
        LOG_FUNC_NAME();

        bool bRet = false;
        if(aid.IsEmpty())
        {
            LOGGER(_log.WriteLine("PSE DDF枚举..."));
            bRet = pboc_get_pse_aid(_defaultPSE, _list);
            if(!bRet)
            {
                LOGGER(
                _log.WriteLine("PSE DDF枚举应用列表失败,尝试从SYS_PBOC_AID枚举..."));

                ByteBuilder pbocAid(8);
                DevCommand::FromAscii(PBOC_V2_0_BASE_AID, pbocAid);
                bRet = pboc_get_child_aid(pbocAid, _list, pAidData);
            }
        }
        else
        {
            LOGGER(_log.WriteLine("子AID枚举..."));
            bRet = pboc_get_child_aid(aid, _list, pAidData);
        }
        return bRet;
    }
    /**
     * @brief 选择AID,根据传入的AID列表获取最后选择的AID 
     * @param [in] aidlist 外部传入的AID列表 
     * @param [out] selectAid 最后选择的AID 
     * @param [out] pAidBuff [default:NULL] 选AID后返回的数据(默认不需要) 
     * @param [in] allowLocked [default:true] 是否允许选择被锁的AID  
     */ 
    bool pboc_final_select_aid(const list<PBOC_Library::AID>& aidlist, 
        ByteBuilder& selectAid, ByteBuilder* pAidBuff = NULL, bool allowLocked = true)
    {
        LOG_FUNC_NAME();
        LOGGER(_log_aid_list(aidlist));

        list<PBOC_Library::AID>::const_iterator itr;
        list<PBOC_Library::AID> childAidList;
        list<ByteBuilder> childAidData;
        list<ByteBuilder>::iterator aidDataItr;
        const ByteBuilder* pAid = NULL;

        LOGGER(_log.WriteLine("AID列表选择..."));
    
        for(itr = aidlist.begin();itr != aidlist.end(); ++itr)
        {
            childAidList.clear();
            childAidData.clear();

            // 名称为空则用PSE进行选择 
            pAid = &(itr->aid);
            if(itr->aid.IsEmpty())
            {
                LOGGER(_log.WriteLine("AID为空,采用PSE..."));
                pAid = &_defaultPSE;
            }

            if(pboc_get_child_aid(*pAid, childAidList, 
                (pAidBuff == NULL ? NULL : &childAidData),
                allowLocked, false, true))
            {
                // childAidList一定有至少一个AID数据 
                selectAid += childAidList.front().aid;
                LOGGER(_log.WriteLine("最后选择的AID:").WriteLine(childAidList.front().aid));

                if(NULL != pAidBuff)
                {
                    *pAidBuff += childAidData.front();
                    LOGGER(
                    _log.WriteLine("AID数据:");
                    _log.WriteLine(childAidData.front()));
                }

                return true;
            }

            LOGGER(_log.WriteLine());
        }
        return false;
    }
    //@}
    //@{
    /**@name
     * @brief 应用初始化(获取应用的AIP和AFL列表)
     */
    /**
     * @brief 根据选AID后的AID数据获取GPO数据信息 
     * @param [in] aidData 选应用后返回的数据 
     * @param [in] tagTerminal 终端数据 
     * @param [out] token 返回的GPO报文结构 
     * @param [out] pDOL [default:NULL] GPO时的DOL数据域 
     */ 
    bool pboc_init_gpo(const ByteArray& aidData, TlvElement& tagTerminal, PBOC_Library::GPO_Token& token, ByteBuilder* pDOL = NULL)
    {
        LOG_FUNC_NAME();

        ByteBuilder pdol(DEV_BUFFER_SIZE / 2);
        // 取9F38标签数据 0x9F38 = PBOC_v2_0_TagMap::GetHeader(PBOC_v2_0_TagMap::PDOL);
        LOGGER(_log.WriteLine("查找PDOL标签..."));
        bool contains = PBOC_Library::GetTagValue(aidData, PBOC_v2_0_TagMap::GetHeader(PBOC_v2_0_TagMap::PDOL), &pdol);
        /* 对于有的卡片返回的是DDF模板,找不到0x9F38标签,直接83 00即可 */
        //ASSERT_FuncErrInfo(contains, TagNotExistErr, "找不到PDOL标签(9F38)");
        /* GPO */
        ByteBuilder recv(DEV_BUFFER_SIZE);
        ByteBuilder cmd(16);
        recv.Clear();
        LOGGER(_log.WriteLine("组DOL数据..."));
        // 0x83
        recv.Append(static_cast<byte>(PBOC_v2_0_TagMap::GetHeader(PBOC_v2_0_TagMap::CommandTemplate)));
        recv.Append(static_cast<byte>(0x00));// 长度,不可能大于253

        recv[1] = _itobyte(PBOC_Library::PackPDOL(pdol, tagTerminal, recv));
        cmd.Clear();
        LOGGER(_log.WriteLine("生成GPO命令..."));
        LOGGER(ByteBuilder dolFormat(32);
        ByteBuilder dataFormat(32);
        PBOC_Library::FormatTLV(pdol, recv.SubArray(2), dolFormat, dataFormat);
        _log.WriteLine("GPO DOL数据:") << "     " << dolFormat.GetString() << endl;
        _log.WriteStream(recv.SubArray(0, 2)) << ' ';
        _log.WriteLine(dataFormat.GetString()));

        if(pDOL != NULL)
            pDOL->Append(recv);

        GPOCmd::Make(cmd, recv);
        ASSERT_FuncErrInfoRet(_apdu(cmd, recv), DeviceError::OperatorErr, "发送GPO命令失败");
        /* 处理返回的报文 */
        LOGGER(_log.WriteLine("解析GPO报文..."));
        contains = PBOC_Library::UnpackGPO_ReMessage(recv, token);
        ASSERT_FuncErrInfo(contains, DeviceError::RecvFormatErr, "GPO返回报文格式错误");

        LOGGER(_log_gpo_token(token));

        return true;
    }
    /**
     * @brief 读应用数据,根据相应的GPO返回数据取应用数据(返回是否读取全部的记录)
     * @param [in] token GPO返回的报文数据 
     * @param [out] data 获取到的应用数据 
     * @param [out] pAuthData [default:NULL] 用于认证的数据(默认不需要) 
     */
    bool pboc_get_appdata(const PBOC_Library::GPO_Token& token, 
        ByteBuilder& data, ByteBuilder* pAuthData = NULL)
    {
        LOG_FUNC_NAME();

        // 需要读取的全部记录数 
        size_t totalCount = 0;
        // 实际读取的记录数  
        size_t count = 0;
        byte currentLen = 0;
        ByteBuilder recode(DEV_BUFFER_SIZE);
        list<PBOC_Library::AFL>::const_iterator itr;
        LOGGER(_log.WriteLine("取AFL数据..."));
        for(itr = token.AFL_List.begin();itr != token.AFL_List.end(); ++itr)
        {
            currentLen = 0;
            for(byte i = itr->Start;i <= itr->End; ++i)
            {
                ++currentLen;
                ++totalCount;
                recode.Clear();
                LOGGER(_log<<"读取记录组 SFI:<"<<_hex(itr->Sfi)<<"> ["<<static_cast<int>(itr->Start)<<","<<static_cast<int>(itr->End)<<"]\n");
                // 检查AFL中是否读取到所有的记录
                if(_read_recode(itr->Sfi, i, recode) == false)
                {
                    _logErr(NoRecodeErr);
                    LOGGER(_log<<"SFI:<"<<_hex(itr->Sfi)<<">,RecodeNum:<"<<_hex(i)<<">\n");
                    continue;
                }
                // 实际读到的记录数目 
                ++count;

                data.Append(recode);
                // 将数据添加到需要认证的缓冲区中
                if(NULL != pAuthData && currentLen < itr->Len)
                {
                    // 标签头不参与认证
                    if(itr->Sfi >= 1 && itr->Sfi <= 10)
                    {
                        // 0x70
                        PBOC_Library::GetTagValue(recode, PBOC_v2_0_TagMap::GetHeader(PBOC_v2_0_TagMap::Recode), pAuthData);
                    }
                    else
                    {
                        pAuthData->Append(recode);
                    }
                }
            }
        }

        LOGGER(
        _log<<"需要读取的记录数:<"<<totalCount
            <<">,实际读取的记录数:<"<<count<<">\n");

        /* 不输出，数据太长了
        LOGGER(
        _log.WriteLine("获取到的应用数据:");
        _log.WriteLine(data));
        */

        ASSERT_FuncInfo(count >= totalCount, "获取AFL全部记录数据失败");

        return true;
    }
    //@}
    //----------------------------------------------------- 
    LOGGER(
    /// 记录AID属性字节信息 
    void _log_aid_property(byte _property)
    {
        BitConvert b(_property);
        _log<<"选择时持卡人确认:<"<<(b[7]!=0)<<">\n";
        _log<<"保留:<"<<b[6]<<b[5]<<b[4]<<">\n";
        _log<<"优先级:<"<<static_cast<int>(_property & 0x0F)<<">\n";
    })
    LOGGER(
    /// 记录GPO_Token的信息
    void _log_gpo_token(const PBOC_Library::GPO_Token& token)
    {
        LOGGER(
        _log.WriteLine("GPO返回报文:");
        _log<<"AIP:<"<<token.AIP<<">\n");

        if(token.AFL_List.empty())
        {
            LOGGER(_log.WriteLine("AFL列表为空"));
            return ;
        }

        LOGGER(
        _log<<"AFL列表长度:<"<<token.AFL_List.size()<<">\n";

        list<PBOC_Library::AFL>::const_iterator itr;
        for(itr = token.AFL_List.begin();itr != token.AFL_List.end(); ++itr)
        {
            _log<<"AFL:<";
            _log.WriteHex(ByteArray(reinterpret_cast<byte*>(const_cast<PBOC_Library::AFL*>(&(*itr))), sizeof(PBOC_Library::AFL)));
            _log<<">\n";
        });
    })
    LOGGER(
    /// 输出AID列表
    void _log_aid_list(const list<PBOC_Library::AID>& _list)
    {
        /* Log */
        _log.WriteLine("AID列表:");
        list<PBOC_Library::AID>::const_iterator itrAid;
        size_t index = 0;
        for(itrAid = _list.begin();itrAid != _list.end();++itrAid)
        {
            ++index;
            _log<<"序号:<"<<index<<"> 优先级:<"<<static_cast<ushort>(_list.back().priority)<<">\n";
            if(itrAid->aid.IsEmpty())
                _log.WriteLine("PSE");
            else
                _log.WriteLine(itrAid->aid);
        };
    })
    //-----------------------------------------------------
    /* 设置为保护,禁用构造函数,只允许继承使用 */
    PBOC_v2_0_BaseAdapter() : ICCardAppAdapter() {}
    //-----------------------------------------------------
public:
    /// 一些默认终端数据(构造DOL时，全零的话卡片可能不执行)
    ByteBuilder TerminalValue;
    //-----------------------------------------------------
};
//---------------------------------------------------------
/// PBOC2.0交易流程适配器(每一个单独的步骤,标签值采用写死的方式) 
class PBOC_v2_0_StepAdapter : public PBOC_v2_0_BaseAdapter
{
protected:
    //----------------------------------------------------- 
    /// 初始化数据成员 
    void _init()
    {
        _defaultPSE = SYS_PBOC_V2_0_PSE; 
        DevCommand::FromAscii(PBOC_V2_0_TERMINAL_TAGVALUE, TerminalValue);
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    PBOC_v2_0_StepAdapter() : PBOC_v2_0_BaseAdapter() { _init(); }
    //----------------------------------------------------- 
    /**
     * @brief 获取或设置PSE 
     */ 
    inline ByteBuilder& PSE()
    {
        return _defaultPSE;
    }
    /// 在终端数据中生成随机数 
    ByteArray Random()
    {
        TlvElement root = TlvElement::Parse(TerminalValue);

        const ushort TAG_RANDOM = 0x9F37;
        size_t randomLen = 4;
        ByteBuilder tmp(8);
        ByteArray ranArray;

        TlvElement tagElement = root.Select(TAG_RANDOM);
        if(!tagElement.IsEmpty())
        {
            randomLen = tagElement.GetLength();
            ByteConvert::Random(tmp, randomLen);
            ranArray = ByteArray(tmp.GetBuffer(), tagElement.SetValue(tmp.GetBuffer()));
        }
        else
        {
            TlvConvert::ToHeaderBytes(TAG_RANDOM, TerminalValue);
            TlvConvert::ToLengthBytes(randomLen, TerminalValue);
            ByteConvert::Random(TerminalValue, randomLen);
            ranArray = TerminalValue.SubArray(TerminalValue.GetLength() - randomLen, randomLen);
        }
        return ranArray;
    }
    /**
     * @brief 枚举卡中的AID列表 
     * @param [in] rootAid 输入的父AID名称(为空则表示从PSE获取,PSE获取失败则从SYS_PBOC_AID获取)
     * @param [out] _list 获取到的应用列表 
     * @param [out] pAidData [default:NULL] 获取到的对应AID数据(默认不需要) 
     */ 
    bool EnumAid(const ByteArray& rootAid, list<PBOC_Library::AID>& _list, 
        list<ByteBuilder>* pAidData = NULL)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(size_t lastSize = _list.size();
        _log<<"枚举子AID的根目录:";
        if(rootAid.IsEmpty()) _log<<_defaultPSE;
        else _log<<rootAid;
        _log << endl);
        
        bool bRet = pboc_enum_aid_list(rootAid, _list, pAidData);

        LOGGER(
        /* 记录返回的数据 */
        if(bRet)
        {
            /* 获取最后增加的AID偏移量 */
            list<PBOC_Library::AID>::iterator itr = _list.begin();
            size_t offsetTmp = lastSize;
            while(offsetTmp-- > 0)
            {
                ++itr;
            }
            list<ByteBuilder>::iterator dataItr;
            size_t aidCount = _list.size() - lastSize;
            size_t i = 0;
            if(NULL != pAidData)
            {
                dataItr = pAidData->end();
                for(i = 0;i < aidCount; ++i)
                    --dataItr;
            }

            for(i = 1;itr != _list.end(); ++itr, ++i)
            {
                _log<<"枚举到的AID,数目:<"<<_list.size() - lastSize<<">\n"
                    <<i<<". AID:<"<<ArgConvert::ToString(itr->aid)<<">优先级:<"<<static_cast<int>(itr->priority)<<">";
                if(NULL != pAidData)
                {
                    ushort sw = ICCardLibrary::GetSW(*dataItr);
                    _log<<",是否被锁:<"<<ICCardLibrary::IsSuccessSW(sw)<<">";
                }
                _log.WriteLine();
            }
        });

        return _logRetValue(bRet);
    }
    /**
     * @brief 选择金融应用(按照应用列表选择第一个匹配的应用) 
     * @param [in] _aidList 输入的应用列表(金融AID) 
     * @param [out] pAidData [default:NULL] 选应用后的数据(默认不需要) 
     * @param [out] pSelectAid [default:NULL] 最后选择的AID(默认不需要) 
     * @param [in] allowLocked [default:true] 是否允许选择被锁定的AID
     */ 
    bool SelectAid(const list<PBOC_Library::AID>& _aidList, ByteBuilder* pAidData = NULL, 
        ByteBuilder* pSelectAid = NULL, bool allowLocked = true)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(
        _log.WriteLine("输入的应用列表:");
        list<PBOC_Library::AID>::const_iterator aidItr = _aidList.begin();
        for(aidItr = _aidList.begin();aidItr != _aidList.end(); ++aidItr)
        {
            _log.WriteLine("AID:").WriteLine(aidItr->aid);
        });

        ByteBuilder selectAid(16);
        bool bRet = pboc_final_select_aid(_aidList, selectAid, pAidData, allowLocked);

        if(bRet && (NULL != pSelectAid))
        {
            pSelectAid->Append(selectAid);
        }

        return _logRetValue(bRet);
    }
    /**
     * @brief 选择金融应用(为空则为金融基本AID A000000333) 
     * @param [in] aid [default:""] 需要选择的AID(为空表示从PBOC_V2_0_BASE_AID选择)
     * @param [out] pAidData [default:NULL] 选应用后相关的AID数据(默认不需要) 
     * @param [out] pFullAid [default:NULL] 完整的AID名称(默认不需要) 
     * @param [in] allowLocked [default:true] 是否允许选择被锁定的应用 
     */ 
    bool SelectAid(const ByteArray& aid = "", ByteBuilder* pAidData = NULL, 
        ByteBuilder* pFullAid = NULL, bool allowLocked = true)
    {
        list<PBOC_Library::AID> aidlist;
        aidlist.push_back(PBOC_Library::AID());
        if(aid.IsEmpty())
        {
            DevCommand::FromAscii(PBOC_V2_0_BASE_AID, aidlist.back().aid);
        }
        else
        {
            aidlist.back().aid = aid;
        }
        return SelectAid(aidlist, pAidData, pFullAid, allowLocked);
    }
    /**
     * @brief GPO初始化应用,GPO的数据源将从TerminalValue获取 
     * @param [in] aidData 选择AID时返回的报文数据 
     * @param [out] gpoToken 返回的GPO报文结构 
     * @param [out] pDOL [default:NULL] GPO时的DOL数据 
     */ 
    bool GPO(const ByteArray& aidData, PBOC_Library::GPO_Token& gpoToken, ByteBuilder* pDOL = NULL)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log<<"AidData:\n"<<aidData<<endl);

        TlvElement tagElement = TlvElement::Parse(TerminalValue);
        ASSERT_FuncErrInfoRet(!tagElement.IsEmpty(), DeviceError::ArgFormatErr, "解析终端数据失败");

        bool bRet = pboc_init_gpo(aidData, tagElement, gpoToken, pDOL);
        return _logRetValue(bRet);
    }
    /**
     * @brief 读取卡片全部数据 
     * @param [in] gpoToken 选AID后返回的GPO数据 
     * @param [out] appData 获取到的卡记录全部数据 
     * @param [out] pAuthData [default:NULL] 和认证相关的数据(默认不需要) 
     */ 
    bool GetAllAppData(const PBOC_Library::GPO_Token& gpoToken, ByteBuilder& appData,
        ByteBuilder* pAuthData = NULL)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();
        LOGGER(_log_gpo_token(gpoToken));

        size_t lastlen = appData.GetLength();
        PBOC_Library::GPO_TokenToTag(gpoToken, appData);

        bool bRet = pboc_get_appdata(gpoToken, appData, pAuthData);
        // 获取失败时删除原始数据 
        if(!bRet)
        {
            appData.RemoveTail(appData.GetLength() - lastlen);
        }
        ASSERT_FuncErrInfoRet(bRet, DeviceError::OperatorErr, "取应用数据失败");

        return _logRetValue(true);
    }
    /**
     * @brief 读取卡片部分数据,找到指定数据后退出
     * @param [in] gpoToken 选AID后返回的GPO数据
     * @param [in] tagList 取部分数据需要提取的标签列表(必选标签,没找到读取新数据) 
     * @param [in] subtagList 取部分数据需要提取的标签列表(可选标签,没找到不读取新数据)
     * @param [in] aidData 选AID后返回的数据 
     * @param [out] appData 获取到的数据(包括必须的标签和额外的一些标签) 
     * @param [out] pCount [default:NULL] 实际获取到的必选标签数 
     */ 
    bool GetSubAppData(const PBOC_Library::GPO_Token& gpoToken, 
        const list<TlvHeader>& tagList, const list<TlvHeader>& subtagList,
        ByteBuilder& appData, size_t* pCount = NULL)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log.WriteLine("需要读取的必选标签:");
        list<TlvHeader>::const_iterator tagItr = tagList.begin();
        for(;tagItr != tagList.end(); ++tagItr)
        {
            _log<<TlvConvert::ToHeaderAscii(*tagItr)<<' ';
        }
        _log.WriteLine();
        _log.WriteLine("需要读取的可选标签:");
        tagItr = subtagList.begin();
        for(;tagItr != subtagList.end(); ++tagItr)
        {
            _log << TlvConvert::ToHeaderAscii(*tagItr) << ' ';
        }
        _log.WriteLine());

        size_t count = 0;
        size_t subcount = 0;
        size_t lastlen = appData.GetLength();
        ByteBuilder recode(DEV_BUFFER_SIZE);
        list<PBOC_Library::AFL>::const_iterator itr;
        list<TlvHeader>* pSelectList = NULL;

        LOGGER(_log.WriteLine("循环读取记录查找标签..."));
        for(itr = gpoToken.AFL_List.begin();itr != gpoToken.AFL_List.end(); ++itr)
        {
            for(byte i = itr->Start;i <= itr->End; ++i)
            {
                LOGGER(_log<<"SFI:<"<<_hex(itr->Sfi)<<">,RecodeNum:<"<<_hex(i)<<">\n");
                /* 逐条记录读取 */
                if(_read_recode(itr->Sfi, i, recode) == false)
                {
                    appData.RemoveTail(appData.GetLength() - lastlen);

                    _logErr(NoRecodeErr);
                    return _logRetValue(false);
                }
                /* 解析必选标签 */
                LOGGER(list<TlvHeader> subList;
                pSelectList = &subList);

                count += PBOC_Library::SelectTagToBuffer(tagList, recode, appData, pSelectList);
                LOGGER(if(subList.size() > 0)
                {
                    list<TlvHeader>::iterator itrSubList;
                    for(itrSubList = subList.begin();itrSubList != subList.end(); ++itrSubList)
                    {
                        _log<<"找到标签(必选):<"<<TlvConvert::ToHeaderAscii(*itrSubList)<<">\n";
                    }
                });
                /* 解析可选标签 */
                LOGGER(subList.clear();
                pSelectList = &subList);

                subcount += PBOC_Library::SelectTagToBuffer(subtagList, recode, appData, pSelectList);
                LOGGER(if(subList.size() > 0)
                {
                    list<TlvHeader>::iterator itrSubList;
                    for(itrSubList = subList.begin();itrSubList != subList.end(); ++itrSubList)
                    {
                        _log << "找到标签(可选):<" << TlvConvert::ToHeaderAscii(*itrSubList) << ">\n";
                    }
                });

                if(count >= tagList.size())
                {
                    // 为了退出外围的for循环 
                    itr = gpoToken.AFL_List.end();
                    --itr;
                    break;
                }

                recode.Clear();
            }// for i
        }// for itr
        /* 记录实际查找到的必选标签数 */
        if(NULL != pCount)
        {
            *pCount = count;
        }
        LOGGER(_log << "需要读取的标签数:<" << tagList.size() << ">\n"
            << "实际读取并查找到的标签数 必选:<" << count << ">,可选:<" << subcount << ">\n");

        return _logRetValue(true);
    }
    /**
     * @brief 发送GPO读取IC卡数据,找到指定数据后退出,GPO的数据源将从TerminalValue获取
     * @param [in] tagList 取部分数据需要提取的标签列表(必选标签)
     * @param [in] subtagList 取部分数据需要提取的标签列表(可选标签)
     * @param [in] aidData 选AID后返回的数据
     * @param [out] appData 获取到的数据(包括必须的标签和额外的一些标签)
     * @param [out] pCount [default:NULL] 实际获取到的标签数
     */
    bool GPO_GetSubAppData(const list<TlvHeader>& tagList, const list<TlvHeader>& subtagList, const ByteArray& aidData,
        ByteBuilder& appData, size_t* pCount = NULL)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log << "AidData:\n" << aidData << endl);
        LOGGER(_log.WriteLine("需要读取的标签:");
        list<TlvHeader>::const_iterator tagItr = tagList.begin();
        for(;tagItr != tagList.end(); ++tagItr)
        {
            _log << TlvConvert::ToHeaderAscii(*tagItr) << ' ';
        }
        _log.WriteLine());

        size_t count = 0;
        size_t getdata_count = 0;
        ByteBuilder recode(DEV_BUFFER_SIZE);

        /* 如果AidData中已经有需要的标签则提取 */
        LOGGER(_log.WriteLine("从已有数据中查找标签..."));
        list<TlvHeader> selectList;
        LOGGER(list<TlvHeader>::iterator itr);
        count += PBOC_Library::SelectTagToBuffer(tagList, aidData, appData, &selectList);
        LOGGER(for(itr = selectList.begin();itr != selectList.end(); ++itr)
        {
            _log << "找到标签:<" << TlvConvert::ToHeaderAscii(*itr) << ">\n";
        });
        if(count < tagList.size())
        {
            /* 优先将需要用GET DATA命令读取的标签读取并解析掉 */
            LOGGER(_log.WriteLine("GET_DATA获取标签..."));
            GetDataTagValue(tagList, appData, &getdata_count);
            count += getdata_count;
        }
        // 通过GET DATA取数据还有部分数据没有取完  
        if(count < tagList.size())
        {
            LOGGER(_log.WriteLine("剩余标签需要读取记录..."));
            PBOC_Library::GPO_Token gpoToken;
            LOGGER(_log.WriteLine("GPO..."));
            bool bRet = GPO(aidData, gpoToken);
            ASSERT_FuncErrInfoRet(bRet, DeviceError::OperatorErr, "应用初始化GPO失败");

            LOGGER(_log.WriteLine("处理GPO中返回的标签..."));
            recode.Clear();
            PBOC_Library::GPO_TokenToTag(gpoToken, recode);
            LOGGER(_log.WriteLine("GPO返回标签数据:").WriteLine(recode));
            LOGGER(selectList.clear());
            count += PBOC_Library::SelectTagToBuffer(tagList, recode, appData, &selectList);
            LOGGER(for(itr = selectList.begin();itr != selectList.end(); ++itr)
            {
                _log << "找到标签:<" << TlvConvert::ToHeaderAscii(*itr) << ">\n";
            });
            recode.Clear();
            /* 标签未查找完 */
            if(count < tagList.size())
            {
                LOGGER(_log.WriteLine("循环读取记录查找标签..."));
                size_t subCount = 0;
                ASSERT_FuncErrRet(GetSubAppData(gpoToken, tagList, subtagList, appData, &subCount), DeviceError::OperatorErr);
                count += subCount;
            }
            
        }// if
        /* 记录实际查找到的标签数 */
        if(NULL != pCount)
        {
            *pCount = count;
        }
        LOGGER(_log << "需要读取的标签数:<" << tagList.size() << ">\n"
            << "实际读取并查找到的标签数:<" << count << ">\n");

        return _logRetValue(true);
    }
    /**
     * @brief 获取标签列表中需要用GET DATA获取的标签 
     * @param [in] tagList 需要利用GET_DATA获取的标签列表 
     * @param [out] appData 获取到的数据 
     * @param [out] pCount [default:NULL] 实际获取到的标签数目 
     */ 
    bool GetDataTagValue(const list<TlvHeader>& tagList, ByteBuilder& appData, size_t* pCount = NULL)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        ByteBuilder getDataCmd(16);
        ByteBuilder buffer(DEV_BUFFER_SIZE);
        ByteBuilder recode(DEV_BUFFER_SIZE);
        list<TlvHeader>::const_iterator itr;
        size_t count = 0;
        ushort sw = 0;

        for(itr = tagList.begin();itr != tagList.end(); ++itr)
        {
            /* 限制必须为标准PBOC中的标签 */
            if(PBOC_Library::IsGetDataTag(*itr))
            {
                /* 尝试用GET_DATA命令去取标签应用数据中找不到的标签 */
                buffer.Clear();
                getDataCmd.Clear();
                TlvConvert::ToHeaderBytes(*itr, buffer);
                GetDataCmd::Make(getDataCmd, buffer);

                LOGGER(
                _log<<"取数据命令找标签:<"<<TlvConvert::ToHeaderAscii(*itr)<<"> ");

                // 取数据成功
                sw = _apdu_s(getDataCmd, recode);
                if(sw != ICCardLibrary::UnValidSW && ICCardLibrary::IsSuccessSW(sw))
                {
                    appData += recode;
                    LOGGER(ByteBuilder recodeBuff(8);
                    PBOC_Library::GetTagValue(recode, (*itr), &recodeBuff);
                    _log.WriteStream(recodeBuff) << endl);
                    ++count;
                }
                LOGGER(
                else
                {
                    _log<<"-> 失败"<<endl;
                });
            }
        }

        LOGGER(
        _log<<"需要读取的记录数:<"<<tagList.size()<<">\n"
            <<"通过GET DATA实际读取到的记录数:<"<<count<<">\n");

        if(NULL != pCount)
        {
            *pCount = count;
        }

        return _logRetValue(true);
    }
    /**
     * @brief 发送GAC命令获取交易密文 
     * @param [in] dolTag 需要提取的DOL标签 
     * @param [in] flag 需要获取的应用密文类型 
     * @param [in] srcElement 交易数据标签 
     * @param [out] gacData 获取到的应用密文 
     * @param [in] pFlag [default:NULL] 实际生成的密文类型(为NULL表示不需要) 
     */ 
    bool GAC(TlvHeader dolTag, GenerateACCmd::RefCtlFlag flag, 
        TlvElement& srcElement, ByteBuilder& gacData, GenerateACCmd::RefCtlFlag* pFlag = NULL)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log<<"Dol标签:<"<<TlvConvert::ToHeaderAscii(dolTag)<<">\n");
        LOGGER(_log<<"RefCtlFlag:<"<<_hex(static_cast<byte>(flag))<<">\n");

        TlvElement subElement = srcElement.Select(dolTag);
        ASSERT_FuncErrInfoRet(!subElement.IsEmpty(), TagNotExistErr, "找不到DOL标签");

        ByteBuilder dol(64);
        ByteBuilder dolBuffer(64);
        subElement.GetValue(dol);

        LOGGER(_log.WriteLine("Pdol:").WriteLine(dol));

        ASSERT_FuncErrInfoRet(PBOC_Library::PackPDOL(dol, srcElement, dolBuffer) > 0,
            DeviceError::ArgLengthErr, "生成DOL数据失败");

        LOGGER(ByteBuilder dolFormat(32);
        ByteBuilder dataFormat(32);
        PBOC_Library::FormatTLV(dol, dolBuffer, dolFormat, dataFormat);
        _log.WriteLine("DOL:");
        _log.WriteLine(dolFormat.GetString()).WriteLine(dataFormat.GetString()));
        // 命令 
        ByteBuilder cmdBuffer(64);
        GenerateACCmd::Make(cmdBuffer, dolBuffer, flag, GenerateACCmd::NoRequst);

        ByteBuilder recvBuffer(DEV_BUFFER_SIZE);

        ASSERT_FuncErrInfoRet(_apdu(cmdBuffer, recvBuffer),
            DeviceError::TransceiveErr, "GenerateAC失败");

        if(pFlag != NULL)
            *pFlag = static_cast<GenerateACCmd::RefCtlFlag>(recvBuffer[0]);
        PBOC_Library::GetTagValue(recvBuffer, static_cast<ushort>(recvBuffer[0]), &gacData);

        return _logRetValue(true);
    }
    /**
     * @brief 根据返回的GAC应用密文组数据域 
     * @param [in] gacData 获取到的应用密文(整个报文) 
     * @param [in] taglist 需要提取的标签列表 
     * @param [in] srcElement 标签数据 
     * @param [out] gacArea 组好的GAC数据域 
     * @param [in] isFillEmpty [default:true] 是否保留并填充未找到的标签 
     */ 
    bool PackGacArea(const ByteArray& gacData, const list<TlvHeader>& taglist,
         TlvElement& srcElement, ByteBuilder& gacArea, bool isFillEmpty = true)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log<<"GacData 长度:<"<<gacData.GetLength()<<">,需要长度(11=1密文标识+2计数器+8密文数据)"<<endl);
        ASSERT_FuncErrInfoRet(gacData.GetLength() >= 11, DeviceError::ArgLengthErr, "GAC数据长度错误");

        size_t count = 0;
        /* ARQC的顺序固定部分:
           9f26 9f27 9f10 9f37 9f36 95 9f13 9a 9c
           9f02 5f2a 5a   5f34 82   9f1a 9f03 9f33 */
        ByteBuilder recvBuffer(64);
        LOGGER(ByteBuilder tagBuff(8));
        list<TlvHeader>::const_iterator itr;
        TlvElement subElement;
        for(itr = taglist.begin();itr != taglist.end(); ++itr)
        {
            LOGGER(_log<<"获取标签:<"<<TlvConvert::ToHeaderAscii(*itr)<<">");

            ++count;
            /* switch处理必须从dolData中才能提取出来的标签 */
            switch(*itr)
            {
            // 9f26 应用密文 
            case 0x9F26:
                TlvConvert::MakeTLV(0x9F26, gacData.SubArray(3, 8), gacArea);
                break;
            // 9f27 密文信息数据 
            case 0x9F27:
                TlvConvert::MakeTLV(0x9F27, gacData.SubArray(0, 1), gacArea);
                break;
            // 9f10 发卡行应用数据
            case 0x9F10:
                // 11 = 1 + 2 + 8
                TlvConvert::MakeTLV(0x9F10, gacData.SubArray(11), gacArea);
                break;
            // 9f36 应用交易计数器
            case 0x9F36:
                TlvConvert::MakeTLV(0x9F36, gacData.SubArray(1, 2), gacArea);
                break;
            default:
                --count;
                subElement = srcElement.Select(*itr);
                if(!subElement.IsEmpty())
                {
                    subElement.GetTLV(gacArea);
                    ++count;
                }
                else
                {
                    if(isFillEmpty)
                    {
                        TlvConvert::ToHeaderBytes(*itr, gacArea);
                        TlvConvert::ToLengthBytes(static_cast<size_t>(0), gacArea);
                        ++count;

                        LOGGER(_log.WriteLine("没有找到标签,填充空数据"));
                    }
                    LOGGER(
                    else
                    {
                        _log<<" 失败";
                    })
                }
                break;
            }
            LOGGER(_log<<endl);
        }

        LOGGER(
        _log<<"需要获取的标签数:<"<<taglist.size()<<">\n"
            <<"实际获取到的标签数:<"<<count<<">\n");

        return _logRetValue(count >= taglist.size());
    }
    /**
     * @brief 执行外部认证 
     * @param [in] arpcElement ARPC数据标签,没有91标签则不执行
     */ 
    bool ExternalAuthenticate(TlvElement& arpcElement)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        bool bRet = true;
        // 91标签(发卡行认证数据) 
        TlvElement subElement = arpcElement.Select(PBOC_v2_0_TagMap::GetHeader(PBOC_v2_0_TagMap::IssuerAuthenticationData));
        if(!subElement.IsEmpty())
        {
            ByteBuilder cmd(64);
            ByteBuilder recv(64);
            subElement.GetValue(recv);

            // 未加授权相应码  
            if(recv.GetLength() < 10)
            {
                ByteBuilder tmp(4);
                // 0x8A
                subElement = arpcElement.Select(PBOC_v2_0_TagMap::GetHeader(PBOC_v2_0_TagMap::AuthorizationResponseCode));
                if(subElement.IsEmpty())
                {
                    _logErr(TagNotExistErr, "找不到授权相应码,填写默认值(00)");

                    recv += "00";
                }
                else
                {
                    subElement.GetValue(tmp);
                    recv += tmp;
                }
            }

            ExternalAuthenticateCmd::Make(cmd, recv);

            bRet = _apdu(cmd, recv);
        }
        LOGGER(
        else
        {
            _log.WriteLine("找不到91标签");
        });
        return _logRetValue(bRet);
    }
    /**
     * @brief 执行外部认证 
     * @param [in] issuerAuthenticationData 发卡行认证数据(91标签)
     * @param [in] authorizationResponseCode 授权响应码(8A标签) 
     */ 
    bool ExternalAuthenticate(const ByteArray& issuerAuthenticationData, const ByteArray& authorizationResponseCode)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        ByteBuilder cmd(64);
        ByteBuilder recv(64);

        ExternalAuthenticateCmd::Make(cmd, recv);
        bool bRet = _apdu(cmd, recv);

        return _logRetValue(bRet);
    }
    /**
     * @brief 执行脚本标签
     * @param [in] arpcEmelent ARPC数据标签(需要执行的标签类型,71或72) 
     * @param [in] scriptRoot 需要解析的脚本标签(71或者72) 
     * @param [in] pCount [default:NULL] 执行失败的脚本号(为NULL表示不需要,如果第0条就失败,则说明没有脚本) 
     * @retval bool
     * @return 
     */
    bool ExecuteScript(TlvElement& arpcEmelent, const TlvHeader& scriptRoot, size_t* pCount = NULL)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(
        _log<<"脚本标签:<"<<TlvConvert::ToHeaderAscii(scriptRoot)<<">\n");

        ByteBuilder scriptBuff(64);
        TlvElement subElement = arpcEmelent.Select(scriptRoot);
        ASSERT_FuncErrInfoRet(!subElement.IsEmpty(), TagNotExistErr, "不存在脚本标签");
        subElement.GetValue(scriptBuff);
        subElement = TlvElement::Parse(scriptBuff);
        return ExecuteScript(subElement, pCount);
    }
    /**
     * @brief 执行脚本标签 
     * @param [in] scriptEmelent 脚本数据标签(内部包含至少一个86脚本) 
     * @param [out] pCount [default:NULL] 执行失败的脚本号(为NULL表示不需要,如果第0条就失败,则说明没有脚本) 
     */ 
    bool ExecuteScript(TlvElement& scriptEmelent, size_t* pCount = NULL)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        size_t count = 0;
        ByteBuilder cmd(64);
        ByteBuilder recv(64);
        bool bRet = false;

        // 86标签 
        do
        {
            TlvElement subElement = scriptEmelent.SelectAfter(PBOC_v2_0_TagMap::GetHeader(PBOC_v2_0_TagMap::IssuerScriptCommand));
            if(subElement.IsEmpty())
                break;
        
            ++count;

            LOGGER(_log<<"执行第:<"<<count<<">条脚本\n");

            cmd.Clear();
            subElement.GetValue(cmd);

            if(_apdu(cmd, recv))
            {
                bRet = false;

                LOGGER(_log<<"失败的脚本号:<"<<count<<">\n");

                break;
            }
        } while(true);

        /* 记录失败的脚本序号 */
        if(NULL != pCount)
        {
            *pCount = count;
        }

        return _logRetValue(bRet);
    }
    /**
     * @brief 获取交易明细格式
     * @param [in] detailFormat 获取到的日志格式
     * @param [in] detailHeader 日志格式记录标签头 
     * @retval bool
     * @return 
     */
    bool GetDetailFormat(ByteBuilder& detailFormat, const TlvHeader& detailHeader)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();
        LOGGER(_log<<"日志格式标签:<"<<TlvConvert::ToHeaderAscii(detailHeader)<<">\n");

        /* 取明细格式 */
        ByteBuilder cmd(8);
        ByteBuilder recv(32);

        ByteConvert::FromObject(detailHeader, recv);
        GetDataCmd::Make(cmd, recv);
        recv.Clear();
        ASSERT_FuncErrInfoRet(_apdu(cmd, recv), DeviceError::TransceiveErr, "GET DATA命令取记录格式失败");

        TlvElement root = TlvElement::Parse(recv);
        ASSERT_FuncErrInfoRet(!root.IsEmpty(), DeviceError::RecvFormatErr, "返回的日志记录格式错误");

        TlvElement tagElement = root.Select(detailHeader);
        ASSERT_FuncErrInfoRet(!tagElement.IsEmpty(), TagNotExistErr, "找不到日志记录格式");

        tagElement.GetValue(detailFormat);

        return _logRetValue(true);
    }
    /**
     * @brief 获取交易明细 
     * @param [in] recInfo 日志信息标签数据([0]=SFI [1]=记录总条数)
     * @param [out] details 获取到的明细列表 
     * @param [in] recodeIndex [default:0] 需要获取的明细记录号[1,10] 其他值则表示全部获取 
     * @param [out] pCount [default:NULL] 获取到明细数 
     */ 
    bool GetDetailList(const ByteArray& recInfo, list<ByteBuilder>& details, byte recodeIndex = 0, size_t* pCount = NULL)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(
        _log<<"日志信息:<"<<recInfo<<">\n";
        _log<<"需要获取的记录号:<"<<static_cast<int>(recodeIndex)<<">\n");

        ASSERT_FuncErrRet(recInfo.GetLength() > 1, DeviceError::ArgLengthErr);

        LOGGER(
        _log<<"日志SFI:<"<<_hex(recInfo[0])<<">\n"
            <<"记录条数:<"<<static_cast<int>(recInfo[1])<<">\n");

        byte startRecode = recodeIndex;
        byte endRecode = recodeIndex;
        size_t recodeCount = 0;
        // 只读取单独的记录 
        if(recodeIndex > recInfo[1] || recodeIndex < 1)
        {
            startRecode = 1;
            endRecode = recInfo[1];
        }
        ByteBuilder cmd(16);
        ByteBuilder recv(64);
        ushort sw = ICCardLibrary::UnValidSW;
        // 记录号为[1,recInfo[1]]
        for(byte recodeNum = startRecode;recodeNum <= endRecode; ++recodeNum)
        {
            LOGGER(_log<<"读记录:<"<<static_cast<ushort>(recodeNum)<<">\n");

            cmd.Clear();
            recv.Clear();
            // 日志记录的SFI 
            ReadRecodeCmd::Make(cmd, recInfo[0], recodeNum);
            sw = _apdu_s(cmd, recv);
            /* 日志已经读完 */
            if(sw == ICCardLibrary::RecodeNotFound)
                break;

            if(!ICCardLibrary::IsSuccessSW(sw))
            {
                LOGGER(_logErr(NoRecodeErr, "读记录失败"));
                break;
            }

            ++recodeCount;
            details.push_back(recv);
        }// for
        if(NULL != pCount)
        {
            *pCount = recodeCount;
        }

        return _logRetValue(true);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
/// IC卡上层具体应用逻辑适配器 
class PBOC_v2_0_AppAdapter : public PBOC_v2_0_StepAdapter
{
protected:
    //----------------------------------------------------- 
    /// 执行脚本 
    bool _ExecuteScript(TlvElement& tagElement, ByteBuilder& df31)
    {
        // 执行脚本失败 
        size_t scriptCount = 0;
        size_t index = df31.GetLength() - 5;
        if(!ExecuteScript(tagElement, &scriptCount))
        {
            df31[index] = 0x010 & static_cast<byte>(scriptCount & 0x0F);
            LOGGER(_log << "执行第:<" << scriptCount << ">条脚本失败\n");
            LOGGER(_log << "写卡脚本通知:" << df31 << endl);
            _logErr(DeviceError::DevVerifyErr, "执行脚本失败");
            return false;
        }
        else
        {
            df31[index] = static_cast<byte>(0x020);
            LOGGER(_log << "写卡脚本通知:" << df31 << endl);
        }
        return true;
    }
    //----------------------------------------------------- 
public:
    //----------------------------------------------------- 
    /**
     * @brief 组装外部必须传入的交易数据  
     * @param [out] amountData 组好的交易数据 
     * @param [in] amount 交易金额(以分为单位) 9F02
     * @param [in] otherAmount 其他金额(以分为单位,手续费用) 9F03
     * @param [in] amountType 交易类型 9C
     * @param [in] amountCode 交易货币代码 5F2A 
     * @param [in] sDate [default:NULL] 交易日期,为NULL表示获取当前日期 9A
     * @param [in] sTime [default:NULL] 交易时间,为NULL标识获取当前时间 9F21
     * @param [in] sName [default:NULL] 商户名称 9F4E
     */ 
    static size_t PackAmountData(ByteBuilder& amountData, 
        uint amount, uint otherAmount, byte amountType, ushort amountCode, 
        const char* sDate = NULL, const char* sTime = NULL, const char* sName = NULL)
    {
        size_t len = 0;
        ByteBuilder tmp(16);
        /* 交易金额 9F02 */
        tmp.Format("%012d", amount);
        len += TlvConvert::ToHeaderBytes(0x9F02, amountData);
        len += TlvConvert::ToLengthBytes(6, amountData);
        len += ByteConvert::FromAscii(tmp, amountData);
        tmp.Clear();
        /* 其他金额 9F03 */
        tmp.Format("%012d", otherAmount);
        len += TlvConvert::ToHeaderBytes(0x9F03, amountData);
        len += TlvConvert::ToLengthBytes(6, amountData);
        len += ByteConvert::FromAscii(tmp, amountData);
        tmp.Clear();
        /* 交易类型 9C */
        tmp.Format("%02d", amountType);
        len += TlvConvert::ToHeaderBytes(0x9C, amountData);
        len += TlvConvert::ToLengthBytes(1, amountData);
        len += ByteConvert::FromAscii(tmp, amountData);
        tmp.Clear();
        /* 交易货币代码 5F2A */
        tmp.Format("%04d", amountCode);
        len += TlvConvert::ToHeaderBytes(0x5F2A, amountData);
        len += TlvConvert::ToLengthBytes(2, amountData);
        len += ByteConvert::FromAscii(tmp, amountData);
        tmp.Clear();
        /* 时间信息 */
        time_t lt;
        time(&lt);
        tm t;
        t = (*localtime(&lt));
        /* 交易日期 9A */
        size_t slen = _strlen(sDate);
        if(slen < 1)
            PBOC_Library::GetLocalTime(&tmp, NULL, &t);
        else
        {
            tmp = sDate;
            ByteConvert::Fill(tmp, 6, false, 0x00);
        }
        len += TlvConvert::ToHeaderBytes(0x9A, amountData);
        len += TlvConvert::ToLengthBytes(3, amountData);
        len += ByteConvert::FromAscii(tmp, amountData);
        tmp.Clear();
        /* 交易时间 9F21 */
        slen = _strlen(sTime);
        if(slen < 1)
            PBOC_Library::GetLocalTime(NULL, &tmp, &t);
        else
        {
            tmp = sTime;
            ByteConvert::Fill(tmp, 6, false, 0x00);
        }
        len += TlvConvert::ToHeaderBytes(0x9F21, amountData);
        len += TlvConvert::ToLengthBytes(3, amountData);
        len += ByteConvert::FromAscii(tmp, amountData);
        /* 商户名称 9F4E */
        tmp = sName;
        ByteConvert::Fill(tmp, 20, true, 0x00);
        len += TlvConvert::MakeTLV(0x9F4E, tmp, amountData);

        return len;
    }
    //----------------------------------------------------- 
    /**
     * @brief 获取卡号(从57或者5A标签中获取) 
     * @param [out] cardNo 获取到的卡号 
     * @param [in] allowLocked [default:true] 是否允许获取应用被锁定的卡片信息 
     * @param [in] rootAid [default:""] 获取卡号的金融应用区域(为空表示使用PBOC_V2_0_BASE_AID) 
     */ 
    bool GetCardNumber(ByteBuilder& cardNo, bool allowLocked = true, const ByteArray& rootAid = "")
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log.WriteLine("RootAid:").WriteLine(rootAid));

        /* 选文件 */
        ByteBuilder aidData(64);
        bool bRet = SelectAid(rootAid, &aidData, NULL, allowLocked);
        ASSERT_FuncErrInfoRet(bRet, DeviceError::OperatorErr, "选AID失败");

        ByteBuilder recode(DEV_BUFFER_SIZE);
        list<PBOC_Library::AFL>::const_iterator itr;
        PBOC_Library::GPO_Token gpoToken;
        ASSERT_FuncErrInfoRet(GPO(aidData, gpoToken), DeviceError::OperatorErr, "应用初始化GPO失败");
        for(itr = gpoToken.AFL_List.begin();itr != gpoToken.AFL_List.end(); ++itr)
        {
            for(byte i = itr->Start;i <= itr->End; ++i)
            {
                LOGGER(_log<<"SFI:<"<<_hex(itr->Sfi)<<">,RecodeNum:<"<<_hex(i)<<">\n");
                /* 逐条记录读取 */
                if(_read_recode(itr->Sfi, i, recode) == false)
                {
                    _logErr(NoRecodeErr);
                    return _logRetValue(false);
                }

                TlvElement root = TlvElement::Parse(recode);
                // 卡号或等效二磁道数据
                TlvElement tagElement = root.Select(0x5A);
                if(tagElement.IsEmpty())
                    tagElement = root.Select(0x57);
                if(!tagElement.IsEmpty())
                {
                    aidData.Clear();
                    tagElement.GetValue(aidData);

                    recode.Clear();
                    ByteConvert::ToAscii(aidData, recode);
                    size_t len = 0;
                    for(;len < recode.GetLength(); ++len)
                    {
                        if(!_is_digital(static_cast<char>(recode[len])))
                            break;
                    }
                    cardNo = recode.SubArray(0, len);
                    return _logRetValue(true);
                }

                recode.Clear();
            }// for i
        }// for itr

        return _logRetValue(false);
    }
    /**
     * @brief 取余额 
     * @param [out] balance 获取到的余额以分为单位 
     * @param [in] aid [default:""] 需要选择的金融应用AID(为空表示使用PBOC_V2_0_BASE_AID) 
     */ 
    bool GetBalance(uint& balance, const ByteArray& aid = "")
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log.WriteLine("Aid:").WriteLine(aid.IsEmpty()?DevCommand::FromAscii(PBOC_V2_0_BASE_AID):aid));

        ByteBuilder sendCmd(8);
        ByteBuilder recvCmd(8);
        SelectCmd::Make(sendCmd, aid.IsEmpty()?DevCommand::FromAscii(PBOC_V2_0_BASE_AID):aid, SelectCmd::First);
        ASSERT_FuncErrInfoRet(_apdu(sendCmd, recvCmd), DeviceError::SendErr, "选AID失败");

        sendCmd.Clear();
        DevCommand::FromHex("80CA9F7900", sendCmd);
        ushort sw = _apdu_s(sendCmd, recvCmd);
        ASSERT_FuncErrInfoRet(sw != ICCardLibrary::UnValidSW, DeviceError::OperatorStatusErr, "取余额指令失败");
        
        if(!ICCardLibrary::IsSuccessSW(sw))
        {
            sendCmd[ICCardLibrary::LC_INDEX] = static_cast<byte>(0x09);
            recvCmd.Clear();

            ASSERT_FuncErrInfoRet(_apdu(sendCmd, recvCmd), DeviceError::SendErr, "取余额失败");
        }
        
        sendCmd.Clear();
        PBOC_Library::GetTagValue(recvCmd, 0x9F79, &sendCmd);

        LOGGER(
        _log<<"余额:<"<<sendCmd<<",");

        recvCmd.Clear();
        ByteConvert::ToAscii(sendCmd, recvCmd);
        balance = ArgConvert::FromString<uint>(recvCmd.GetString());

        LOGGER(_log<<balance<<">\n");

        return _logRetValue(true);
    }
    /**
     * @brief 获取IC卡信息 
     * @param [in] aid 需要选择的金融应用AID,为""则默认为金融AID "A000000333" 
     * @param [in] tag 需要读取的必选标签列表(为空表示读取所有卡片数据)
     * @param [in] subtag 需要读取的可选标签列表
     * @param [out] appData 读取到的应用数据(包含需要的标签和额外的标签) 
     * @param [in] allowLocked [default:true] 是否允许选择被锁定的AID 
     */ 
    bool GetInformation(const ByteArray& aid, const ByteArray& tag, const ByteArray& subtag, ByteBuilder& appData, 
        bool allowLocked = true)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();
        LOGGER(_log << "Aid:\n" << (aid.IsEmpty() ? DevCommand::FromAscii(PBOC_V2_0_BASE_AID) : aid) << endl;
        ByteBuilder formatTag(32);
        PBOC_Library::FormatTAG(tag, formatTag, ' ');
        _log << "TagList:\n" << formatTag.GetString() << "\n"
            << "IsAllowLockedAid:<" << allowLocked << ">\n");

        ByteBuilder finalAid(16);
        ByteBuilder aidData(64);
        bool bRet = SelectAid(aid, &aidData, &finalAid, allowLocked);
        ASSERT_FuncErrInfoRet(bRet, DeviceError::OperatorErr, "SelectAid失败");

        /* 卡片数据 */
        list<TlvHeader> taglist;
        PBOC_Library::MakeTagList(tag, taglist);

        list<TlvHeader> subtaglist;
        PBOC_Library::MakeTagList(subtag, subtaglist);

        PBOC_Library::GPO_Token gpoToken;
        /* 读全部数据 */
        if(taglist.empty())
        {
            LOGGER(_log.WriteLine("SelectAid Tag..."));
            /* 当前AID的数据 */
            TlvConvert::MakeTLV(0x4F, finalAid, appData);
            /* 当前应用数据 */
            appData += aidData;
            ICCardLibrary::RemoveSW(appData);
            /* GPO */
            ASSERT_FuncErrInfoRet(GPO(aidData, gpoToken), DeviceError::OperatorErr, "GPO失败");
            PBOC_Library::GPO_TokenToTag(gpoToken, appData);
            /* 基本信息 */
            ASSERT_FuncErrInfoRet(GetAllAppData(gpoToken, appData), DeviceError::OperatorErr, "GET_APPDATA失败");
            /* 取数据需要的标签 */
            PBOC_Library::MakeTagList(PBOC_V2_0_GETDATA_TAGLIST, taglist);
            ASSERT_FuncErrInfoRet(GetDataTagValue(taglist, appData), DeviceError::OperatorErr, "GET_DATA取数据失败");
        }
        else
        {
            size_t count = 0;
            /* 当前AID的数据 */
            size_t appendlen = TlvConvert::MakeTLV(0x4F, finalAid, aidData);
            list<TlvHeader> sublist;
            count = PBOC_Library::SelectTagToBuffer(taglist, aidData, appData, &sublist);
            PBOC_Library::SelectTagToBuffer(subtaglist, aidData, appData);
            aidData.RemoveTail(appendlen);
            /* 剔除已经读取过的标签,防止在读信息中再次读取记录但是找不到标签 */
            if(count > 0)
            {
                LOGGER(list<TlvHeader>::iterator itrSubList;
                for(itrSubList = sublist.begin();itrSubList != sublist.end();++itrSubList)
                {
                    _log<<"找到标签:<"<<TlvConvert::ToHeaderAscii(*itrSubList)<<">\n";
                });

                list_helper<TlvHeader>::split(taglist, sublist);
            }
            /* 读部分信息 */
            bRet = GPO_GetSubAppData(taglist, subtaglist, aidData, appData);
            ASSERT_FuncErrInfoRet(bRet, DeviceError::OperatorErr, "读部分信息失败");
        }

        return _logRetValue(true);
    }
    /**
     * @brief 生成55域数据 
     * @param [in] amountData 交易数据(9F02 9F03等) 
     * @param [out] arqc 获取到的ARQC 55域报文数据
     * @param [in] appData IC卡信息数据(需要有8C标签) 
     * @param [in][out] pFlag [default:NULL] 需要生成的GAC类型返回实际生成的 为空表示ARQC 
     * @param [in] tag [default:NULL] ARQC报文中的标签,为空表示获取规范中默认的必选标签 
     */ 
    bool GenArqc(const ByteArray& amountData, ByteBuilder& arqc, 
        const ByteArray& appData, GenerateACCmd::RefCtlFlag* pFlag = NULL, const ByteBuilder* pTag = NULL)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();
        LOGGER(_log.WriteLine("交易数据:");
        _log.WriteLine(amountData)<<endl;
        if(pTag == NULL)
        {
            _log << "55域标签:<" << PBOC_V2_0_DEFAULT_ARQC_TAGLIST << ">\n";
        }
        else
        {
            _log.WriteLine("55域标签:").WriteStream(*pTag) << endl;
        });

        ByteBuilder gacData(32);
        ByteBuilder tmpBuff(64);

        TlvElement tagElement = TlvElement::Parse(amountData);
        ASSERT_FuncErrInfoRet(!tagElement.IsEmpty(), DeviceError::ArgFormatErr, "解析交易数据失败");
        ASSERT_FuncErrInfoRet(TlvElement::Parse(tagElement, TerminalValue),
            DeviceError::ArgFormatErr, "解析终端数据失败");
        ASSERT_FuncErrInfoRet(TlvElement::Parse(tagElement, appData),
            DeviceError::ArgFormatErr, "解析IC卡应用数据失败");

        /* 9F13每次都重新单独获取 */
        list<TlvHeader> taglist;
        ByteBuilder arqcTag(8);
        if(pTag == NULL)
        {
            // 规范中默认的必选标签 
            DevCommand::FromAscii(PBOC_V2_0_DEFAULT_ARQC_TAGLIST, arqcTag);
            pTag = &arqcTag;
        }
        PBOC_Library::MakeTagList(*pTag, taglist);
        if(taglist.empty())
        {
            // 规范中默认的必选标签 
            PBOC_Library::MakeTagList(PBOC_V2_0_DEFAULT_ARQC_TAGLIST, taglist);
        }
        if(container_helper<list<TlvHeader> >::contains(taglist, TlvHeader(0x9F13)))
        {
            ByteBuilder cmd(64);
            GetDataCmd::Make(cmd, DevCommand::FromHex("9F13"));
            ASSERT_FuncErrInfoRet(_apdu(cmd, tmpBuff), DeviceError::OperatorErr, "获取9F13失败");
            ASSERT_FuncErrInfoRet(TlvElement::Parse(tagElement, tmpBuff),
                DeviceError::ArgFormatErr, "GET DATA解析9F13失败");
        }

        /* 不管是ARQC还是AAC都组报文 */
        GenerateACCmd::RefCtlFlag gacFlag = GenerateACCmd::ARQC;
        if(pFlag != NULL) gacFlag = *pFlag;
        ASSERT_FuncErrInfoRet(GAC(0x8C, gacFlag, tagElement, gacData, pFlag),
            DeviceError::OperatorErr, "GAC 产生ARQC失败");

        LOGGER(size_t lastArqcLen = arqc.GetLength());
        ASSERT_FuncErrInfoRet(PackGacArea(gacData, taglist, tagElement, arqc), 
            DeviceError::ArgFormatErr, "组55域报文数据失败");

        LOGGER(
        _log.WriteLine("GAC数据:");
        _log.WriteHex(arqc.SubArray(lastArqcLen))<<"\n");

        return _logRetValue(true);
    }
    /**
     * @brief RunArpc 执行写卡脚本
     * 
     * @param [in] amountData 交易数据
     * @param [in] arpc 银联返回的写卡55域报文数据 
     * @param [in] appData IC卡信息数据(需要有8D标签) 
     * @param [out] df31 写卡脚本通知 
     * @param [in] gacTag [default:NULL] 需要返回的55域数据标签列表(为空表示获取规范中默认的标签) 
     * @param [out] pGAC [default:NULL] 返回的55域报文数据(为NULL表示不需要该数据,并且不会发送GAC指令) 
     * @warning pGAC 为NULL将不会发送GAC指令
     * @param [in][out] pFlag [default:NULL] 需要生成的GAC命令类型(默认为NULL则表示生成TC) 
     * @param [in] logFirst [default:true] 脚本的执行中是优先记录明细还是优先执行脚本(必须第一步成功才会执行后续的步骤) 
     * 
     * @return bool 
     */
    bool RunArpc(const ByteArray& amountData, const ByteArray& arpc, 
        const ByteArray& appData, ByteBuilder& df31, const char* gacTag = NULL, ByteBuilder* pGAC = NULL, 
        GenerateACCmd::RefCtlFlag* pFlag = NULL, bool logFirst = true)
    {
        /* Log Header */
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(_log.WriteLine("交易数据:");
        _log.WriteLine(amountData);
        _log.WriteLine("Arpc:");
        _log.WriteLine(arpc);
        _log<<"GAC标签:<"<<(_is_empty_or_null(gacTag)?PBOC_V2_0_DEFAULT_TC_TAGLIST:gacTag)<<">\n");

        ByteBuilder tag9f18(4);
        DevCommand::FromHex("DF310500", df31);
        ByteConvert::Fill(tag9f18, 4);
        df31 += tag9f18;

        LOGGER(_log<<"DF31:DF310500"<<ArgConvert::ToString(tag9f18)<<endl);

        TlvElement root = TlvElement::Parse(arpc);
        ASSERT_FuncErrInfoRet(root.IsEmpty(), DeviceError::ArgFormatErr, "解析55域数据失败");

        TlvElement tagElement = root.Select(0x9F18);
        if(!tagElement.IsEmpty())
        {
            df31.RemoveTail(tag9f18.GetLength());
            tag9f18.Clear();
            tagElement.GetValue(tag9f18);
            ByteConvert::Fill(tag9f18, 4);
            df31 += tag9f18;
            LOGGER(_log<<"RESET DF31:DF310500"<<ArgConvert::ToString(tag9f18)<<endl);
        }
        
        // 是否允许执行写卡脚本 
        bool enableScript = true;
        GenerateACCmd::RefCtlFlag gacFlag = GenerateACCmd::TC;
        if(pFlag != NULL)
        {
            gacFlag = (*pFlag);
            if(gacFlag != GenerateACCmd::TC)
                enableScript = false;
        }
        // 有91标签则执行外部认证
        ASSERT_FuncErrInfoRet(ExternalAuthenticate(root), DeviceError::DevVerifyErr, "外部认证失败");

        if(enableScript && !logFirst) ASSERT_FuncRet(_ExecuteScript(root, df31));
        if(pGAC != NULL)
        {
            // 只有再需要TC时才有必要解析数据
            tagElement = TlvElement::Parse(amountData);
            ASSERT_FuncErrInfoRet(!tagElement.IsEmpty(),
                DeviceError::ArgFormatErr, "解析交易数据失败");
            ASSERT_FuncErrInfoRet(TlvElement::Parse(tagElement, TerminalValue),
                DeviceError::ArgFormatErr, "解析终端数据失败");
            ASSERT_FuncErrInfoRet(TlvElement::Parse(tagElement, appData),
                DeviceError::ArgFormatErr, "解析IC卡应用数据失败");

            // GAC2生成TC 
            ByteBuilder gacData(64);
            bool bRet = GAC(0x8D, gacFlag, tagElement, gacData, &gacFlag);
            ASSERT_FuncErrInfoRet(bRet && logFirst, DeviceError::OperatorErr, "GAC2失败");
            // 未成功生成TC,写卡失败 
            if(enableScript && gacFlag != GenerateACCmd::TC)
            {
                ASSERT_FuncErrInfoRet(logFirst, DeviceError::OperatorStatusErr, "GAC2类型不为TC");
            }
            else
            {
                list<TlvHeader> taglist;
                if(_is_empty_or_null(gacTag))
                    PBOC_Library::MakeTagList(PBOC_V2_0_DEFAULT_TC_TAGLIST, taglist);
                else
                    PBOC_Library::MakeTagList(gacTag, taglist);

                // 如果组TC失败,只记录信息,不报错 
                if(!PackGacArea(gacData, taglist, tagElement, *pGAC))
                    _logErr(DeviceError::ArgFormatErr, "组55域TC报文数据失败");
            }
        }
        if(enableScript && logFirst) ASSERT_FuncRet(_ExecuteScript(tagElement, df31));

        return _logRetValue(true);
    }
    /**
     * @brief 获取卡片交易明细 
     * @param [in] formatTag 日志格式标签
     * @param [in] infoTag 日志记录标签 
     * @param [in] detailInfo 日志信息数据(为""则表示自动根据标签去获取)
     * @param [out] detailFormat 获取到的日志格式 
     * @param [out] detailList 获取到的日志数据 
     * @param [in] aid [default:PBOC_V2_0_BASE_AID] 需要选择的金融应用AID
     * @param [in] recodeIndex [default:0] 需要获取的日志记录号(0:获取全部日志) 
     * @param [in] detailInfo [default:0] 日志记录的SFI,为0表示自动获取 
     */
    bool GetDetail(const TlvHeader& formatTag, const TlvHeader& infoTag, const ByteArray& detailInfo,
        ByteBuilder& detailFormat, list<ByteBuilder>& detailList, 
        const ByteArray& aid = "", byte recodeIndex = 0)
    {
        LOG_FUNC_NAME();
        ASSERT_Device();

        LOGGER(
        _log<<"FORMAT_TAG:<"<<TlvConvert::ToHeaderAscii(formatTag)<<">\n"
            <<"INFO_TAG:<"<<TlvConvert::ToHeaderAscii(infoTag)<<">\n"
            <<"DEFAULT_INFO:\n"<<detailInfo<<endl);

        LOGGER(
        _log<<"Aid:\n"<<(aid.IsEmpty()?DevCommand::FromAscii(PBOC_V2_0_BASE_AID):aid)<<endl);

        ByteBuilder aidData(64);
        ASSERT_FuncErrRet(SelectAid(aid, &aidData), DeviceError::OperatorErr);
        ASSERT_FuncErrRet(GetDetailFormat(detailFormat, formatTag), DeviceError::OperatorErr);

        ByteBuilder recBuff(4);
        // 获取记录SFI位置 
        ByteBuilder tmp(8);
        list<TlvHeader> infoTagList;
        list<TlvHeader> subTagList;
        infoTagList.push_back(infoTag);

        PBOC_v2_0_StepAdapter::GPO_GetSubAppData(infoTagList, subTagList, aidData, tmp);
        PBOC_Library::GetTagValue(tmp, infoTag, &recBuff);
        if(recBuff.IsEmpty())
        {
            _logErr(PBOC_v2_0_BaseAdapter::TagNotExistErr, "获取日志入口失败");
            ASSERT_FuncErrInfoRet(!detailInfo.IsEmpty(), DeviceError::ArgIsNullErr, "日志入口参数错误");

            LOGGER(_log<<"设置日志入口为默认值("<<detailInfo<<")\n");
            recBuff = detailInfo;
        }
        LOGGER(_log<<"日志信息:<"<<recBuff<<">\n");

        ASSERT_FuncErrRet(GetDetailList(recBuff, detailList, recodeIndex), DeviceError::OperatorErr);

        return _logRetValue(true);
    }
    /**
     * @brief 获取卡片交易明细 
     * @param [out] detailFormat 日志格式 
     * @param [out] detailList 获取到的日志数据 
     * @param [in] aid [default:PBOC_V2_0_BASE_AID] 需要选择的金融应用AID
     * @param [in] recodeIndex [default:0] 需要获取的日志记录号(0:获取全部日志) 
     */
    bool GetDealDetail(ByteBuilder& detailFormat, list<ByteBuilder>& detailList, 
        const ByteArray& aid = "", byte recodeIndex = 0)
    {
        const ushort DETAIL_FORMAT_RAG = 0x09F4F;
        const char DETAIL_INFO_TAG[]   = "9F4D";
        const char DETAIL_INFO[] = "0B0A";

        return GetDetail(DETAIL_FORMAT_RAG, DETAIL_INFO_TAG, DevCommand::FromHex(DETAIL_INFO), detailFormat, detailList, aid, recodeIndex);
    }
    /**
     * @brief 获取卡片圈存日志明细 
     * @param [out] detailFormat 日志格式 
     * @param [out] detailList 获取到的日志数据 
     * @param [in] aid [default:PBOC_V2_0_BASE_AID] 需要选择的金融应用AID
     * @param [in] recodeIndex [default:0] 需要获取的日志记录号(0:获取全部日志) 
     */ 
    bool GetEarmarkDetail(ByteBuilder& detailFormat, list<ByteBuilder>& detailList, 
        const ByteArray& aid = "", byte recodeIndex = 0)
    {
        const ushort DETAIL_FORMAT_RAG = 0x0DF4F;
        const char DETAIL_INFO_TAG[]   = "DF4D";
        const char DETAIL_INFO[] = "0C0A";

        return GetDetail(DETAIL_FORMAT_RAG, DETAIL_INFO_TAG, DevCommand::FromHex(DETAIL_INFO), detailFormat, detailList, aid, recodeIndex);
    }
    //----------------------------------------------------- 
};
//--------------------------------------------------------- 
} // namespace v2_0 
} // namespace pboc
} // namesapce iccard 
} // namespace device 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_PBOC_V2_0_APPADAPTER_H_
//=========================================================
