//========================================================= 
/**@file PBOC_v1_0_Library.h 
 * @brief PBOC IC卡相关数据结构定义及相关常规操作
 * 
 * 提供关于标签的一些基本数据
 * 
 * @date 2014-10-17   17:31:18 
 * @author Zhyioo 
 * @version 1.0
 */ 
//--------------------------------------------------------- 
#ifndef _LIBZHOUYB_PBOC_V1_0_LIBRARY_H_
#define _LIBZHOUYB_PBOC_V1_0_LIBRARY_H_
//--------------------------------------------------------- 
#include "../../../../include/Base.h"
//---------------------------------------------------------
namespace zhou_yb {
namespace device {
namespace iccard {
namespace pboc {
//---------------------------------------------------------
/**
 * @brief IC卡常用操作库 
 *
 * 作为一个命名空间存在,这样可以使得后续不同版本的PBOC库能够扩展并使用 
 */
namespace PBOC_Library
{
    //-----------------------------------------------------
    /// AID应用结构
    struct AID
    {
        /// AID值
        ByteBuilder aid;
        /// 优先级
        byte priority;
        /// AID名称 
        ByteBuilder name;

        AID() : aid(16), priority(0), name(4) {}
    };
    //-----------------------------------------------------
    /**
     * @brief 将字符串形式的AID转换为链表形式  
     * @param [in] aidlist AID列表的字符串表示形式,每个AID之间以SPLIT_CHAR号分隔 
     * @param [out] _list 获取到的AID列表 
     * @param [in] flag 分隔符 
     */ 
    static size_t MakeAidList(const char* aidlist, list<PBOC_Library::AID>& _list, char flag = SPLIT_CHAR)
    {
        size_t aidCount = 0;
        size_t aidLen = _strlen(aidlist);
        size_t splitCount = 0;
        ByteBuilder tmp(8);

        splitCount = StringConvert::GetSplitFlagCount(ByteArray(aidlist, aidLen), flag);

        for(size_t i = 0;i <= splitCount; ++i)
        {
            tmp.Clear();
            StringConvert::Split(ByteArray(aidlist, aidLen), tmp, i, flag);
            if(!tmp.IsEmpty())
            {
                _list.push_back(PBOC_Library::AID());
                ByteConvert::FromAscii(tmp, _list.back().aid);
                ++aidCount;
            }
        }

        return aidCount;
    }
    //-----------------------------------------------------
    //@{
    /**@name
     * @brief 其他函数
     */
    /// 获取当前的系统日期和时间 
    static void GetLocalTime(ByteBuilder* pDate_4, ByteBuilder* pTime_3, struct tm* pT = NULL)
    {
        tm t;
        if(NULL == pT)
        {
            time_t lt;
            time(&lt);
            t = (*localtime(&lt));
            
            pT = &t;
        }
        if(NULL != pDate_4)
        {
            pDate_4->Format("%04d%02d%02d", (*pT).tm_year+1900, (*pT).tm_mon+1, (*pT).tm_mday);
        }
        if(NULL != pTime_3)
        {
            pTime_3->Format("%02d%02d%02d", (*pT).tm_hour, (*pT).tm_min, (*pT).tm_sec);
        }
    }
    //@}
    //-----------------------------------------------------
}
//---------------------------------------------------------
} // namespace pboc
} // namespace iccard 
} // namespace device 
} // namespace zhou_yb
//--------------------------------------------------------- 
#endif // _LIBZHOUYB_PBOC_V1_0_LIBRARY_H_
//=========================================================
