//========================================================= 
/**@file ini.h 
 * @brief Ini配置文件 
 * 
 * - 配置文件格式为:
 *  - [分组]
 *  - 配置项="值";注释
 *  - 在一个文件中可有多个不同的分组和配置项 
 * .
 * 
 * @date 2012-10-17   15:48:09 
 * @author Zhyioo 
 * @version 1.0
 */ 
#pragma once 
//---------------------------------------------------------
#include "../base/Convert.h"
using zhou_yb::base::ByteArray;
using zhou_yb::base::ByteBuilder;
using zhou_yb::base::StringConvert;

#include "list.h"
#include <string>
using std::string;

#include <fstream>
using std::ifstream;
using std::ofstream;
using std::endl;
//---------------------------------------------------------
namespace zhou_yb {
namespace container {
//--------------------------------------------------------- 
/// 配置项
struct IniItem
{
    /// 键值
    string Key;
    /// 值
    string Value;
    /// 注释
    string Note;

    IniItem(const char* key = "", const char* value = "", const char* tag = "")
        : Key(key), Value(value), Note(tag) {}
    /// 返回一个空的引用 
    static string EmptyValue;
    /// 剔除字符串前后的空格
    static void TrimItem(const string& str, string& dst, size_t offset, size_t len)
    {
        if(len < 1)
            return;
        
        size_t start = offset;
        // 剔除value前面的空格
        while(str[start] == ' ')
            ++start;
        
        if(start >= str.length())
        {
            dst = "";
            return ;
        }
        
        size_t end = offset + len;
        if(end >= str.length())
            end = str.length();
        
        // 剔除value末尾的空格
        while(str[end - 1] == ' ' && end > start)
            --end;
        // 如果字符串包含在 "" 中则提取出来
        if(str[start] == '\"' && str[end - 1] == '\"')
        {
            ++start;
            --end;
        }
        
        dst = str.substr(start, end - start);
    }
    /// 解析 note为注释的起始字符
    static bool Parse(const string& format, IniItem* item, char note = ';')
    {
        if(format.empty())
            return false;
        size_t equalIndex = StringConvert::IndexOf(ByteArray(format.c_str(), format.length()), '=');
        if(equalIndex == SIZE_EOF)
            return false;
        
        if(NULL == item)
            return true;
        
        TrimItem(format, item->Key, 0, equalIndex);
        
        // 查找字符串标记
        ByteArray formatPos(format.c_str(), format.length());
        formatPos = formatPos.SubArray(equalIndex + 1);
        // 第一个有效字符位置
        size_t spaceIndex = StringConvert::IndexOfNot(formatPos, ' ');
        if(spaceIndex == SIZE_EOF)
            return false;
        formatPos = formatPos.SubArray(spaceIndex);
        size_t startIndex = StringConvert::IndexOf(formatPos, '\"');
        size_t endIndex = SIZE_EOF;
        size_t noteIndex = StringConvert::IndexOf(formatPos, note);
        // " 不在注释内
        if(startIndex != SIZE_EOF && startIndex < noteIndex)
        {
            endIndex = StringConvert::LastIndexOf(formatPos, '\"');
            if(endIndex != SIZE_EOF)
            {
                noteIndex = StringConvert::LastIndexOf(formatPos, note);
                if(noteIndex < endIndex)
                {
                    noteIndex = SIZE_EOF;
                }
            }
        }
        else
        {
            startIndex = 0;
        }
        equalIndex += spaceIndex;
        if(noteIndex == SIZE_EOF)
        {
            noteIndex = formatPos.GetLength();
        }
        else
        {
            // +2 => 跳过 '=' 和 ';'
            TrimItem(format, item->Note, noteIndex + equalIndex + 2, formatPos.GetLength() - noteIndex);
        }
        TrimItem(format, item->Value, startIndex + equalIndex + 1, noteIndex - startIndex);
        return true;
    }
};
//--------------------------------------------------------- 
/// 配置项分组
struct IniGroup
{
    //-----------------------------------------------------
    /// 子项迭代器类型 
    typedef list<IniItem>::iterator iterator;
    /// 子项迭代器类型 
    typedef list<IniItem>::const_iterator const_iterator;
    //----------------------------------------------------- 
    /// 组名
    string Name;
    /// 注释
    string Note;
    /// 子项
    list<IniItem> Items;
    //----------------------------------------------------- 
    IniGroup(const char* name = "", const char* tag = "") : Name(name), Note(tag) {}
    //----------------------------------------------------- 
    /// 空的分组,用于返回一个临时的引用 
    static IniItem EmptyItem;
    //-----------------------------------------------------
    /// 解析分组
    static bool Parse(const string& format, IniGroup* group, char note = ';')
    {
        if(format.empty())
            return false;

        string::size_type left = format.find('[');
        // 没有找到[号
        if(left == string::npos)
            return false;
        // 查找第一个不是空格的字符 
        string::size_type i = 0;
        for(i = 0;i < format.length(); ++i)
        {
            if(format[i] != ' ')
                break;
        }
        if(left != i)
            return false;

        string::size_type right = format.rfind(']');
        if(right == string::npos)
            return false;
        if(right >= left)
        {
            if(NULL != group)
            {
                ++left;
                group->Name = format.substr(left, right - left);
                string::size_type tag = format.find(note);
                if(tag > right)
                {
                    ++tag;
                    group->Note = format.substr(tag, format.length() - tag);
                }
            }
            return true;
        }
        return false;
    }
    //----------------------------------------------------- 
    /// 获取第一个子项 
    inline iterator begin()
    {
        return Items.begin();
    }
    /// 获取第一个子项 
    inline const_iterator begin() const
    {
        return Items.begin();
    }
    /// 获取最后一个子项 
    inline iterator end()
    {
        return Items.end();
    }
    /// 获取最后一个子项 
    inline const_iterator end() const
    {
        return Items.end();
    }
    //----------------------------------------------------- 
    /// 获取指定键值的配置项 
    iterator Get(const string& key, bool ignoreCase = true)
    {
        iterator itr;
        for(itr = Items.begin(); itr != Items.end(); ++itr)
        {
            if(key.length() == itr->Key.length())
            {
                if(StringConvert::Compare(key.c_str(), itr->Key.c_str(), ignoreCase))
                    return itr;
            }
        }
        return Items.end();
    }
    /// 获取指定键值的配置项 
    const_iterator Get(const string& key, bool ignoreCase = true) const 
    {
        const_iterator itr;
        for(itr = Items.begin(); itr != Items.end(); ++itr)
        {
            if(key.length() == itr->Key.length())
            {
                if(StringConvert::Compare(key.c_str(), itr->Key.c_str(), ignoreCase))
                    return itr;
            }
        }
        return Items.end();
    }
    /// 获取指定的值,没有的话使用外部传入的默认值 
    const string& GetValue(const string& key, const string& defaultVal, bool ignoreCase = true) const 
    {
        const_iterator itr = Get(key, ignoreCase);
        if(itr == end())
            return defaultVal;
        return itr->Value;
    }
    /**
     * @brief 返回插入的迭代器,如果键值已经存在则不再添加 
     */  
    iterator Insert(const string& key, bool ignoreCase = true)
    {
        iterator itr = Get(key, ignoreCase);
        if(itr == end())
        {
            Items.push_back(IniItem());
            itr = Items.end();
            --itr;

            return itr;
        }
        return itr;
    }
    /// 删除指定的键值,如果返回 end 则说明不存在键值 
    iterator Remove(const string& key, bool ignoreCase = true)
    {
        iterator itr = Get(key, ignoreCase);
        if(itr != end())
            Items.erase(itr);
        
        return itr;
    }
    /// 取数据成员接口 
    IniItem& operator[](const string& key)
    {
        list<IniItem>::iterator itr = Get(key);
        if(itr == Items.end())
            return EmptyItem;
        return (*itr);
    }
    /// 取数据成员接口 
    const IniItem& operator[](const string& key) const
    {
        list<IniItem>::const_iterator itr = Get(key);
        if(itr == Items.end())
            return EmptyItem;
        return (*itr);
    }
    //-----------------------------------------------------
};
//--------------------------------------------------------- 
/**
 * @brief ini配置文件的读写操作
 * @code
 IniFile ini;
 ini["分组"]["配置项"].value;
 * @endcode
 */ 
class IniFile
{
    //----------------------------------------------------- 
public:
    /// 分组迭代器 
    typedef list<IniGroup>::iterator iterator;
    /// 分组迭代器 
    typedef list<IniGroup>::const_iterator const_iterator;
    //----------------------------------------------------- 
protected:
    //-----------------------------------------------------
    /// 配置项
    list<IniGroup> grps;
    /// 空项 
    static IniGroup EmptyGroup;
    //-----------------------------------------------------
public:
    //-----------------------------------------------------
    /// 从文件中读取配置
    bool Open(const char* path, char note = ';')
    {
        ifstream fin;
        fin.open(path);
        // 打开文件失败
        if(fin.fail())
            return false;
        IniGroup lastGrp;
        IniItem lastItem;
        // 添加一个默认的分组
        grps.push_back(IniGroup());

        string strBuffer;
        char tmpChar;

        while(!fin.eof())
        {
            // 逐行读取并解析
            strBuffer = "";
            while(!fin.eof())
            {
                tmpChar = static_cast<char>(fin.get());
                if(fin.eof())
                    break;
                // 如果是换行或者回车则一整行读取完毕 
                if(tmpChar == static_cast<char>(10) || tmpChar == static_cast<char>(13) || tmpChar == static_cast<char>(EOF))
                    break;
                strBuffer += tmpChar;
            }

            if(strBuffer.length() < 1 || strBuffer[0] == note)
                continue;
            lastGrp.Name = "";
            lastGrp.Note = "";
            lastGrp.Items.clear();
            if(IniGroup::Parse(strBuffer, &lastGrp, note))
            {
                //if(!lastGrp.empty())
                {
                    grps.push_back(lastGrp);
                }
                continue;
            }
            lastItem.Key = "";
            lastItem.Value = "";
            lastItem.Note = "";
            if(IniItem::Parse(strBuffer, &lastItem, note))
            {
                grps.back().Items.push_back(lastItem);
                continue;
            }
        }
        // 新添加的第一个组项为空则删除
        if(grps.front().Items.size() < 1)
        {
            grps.pop_front();
        }
        fin.close();
        return true;
    }
    /// 保存更改后的配置文件
    bool Save(const char* filePath, char note = ';')
    {
        ofstream fout;
        fout.open(filePath);
        if(fout.fail())
            return false;
        list<IniGroup>::iterator itr;
        list<IniItem>::iterator itemitr;
        for(itr = grps.begin(); itr != grps.end(); ++itr)
        {
            fout<<'['<<(*itr).Name<<']';
            if(!(*itr).Note.empty())
            {
                fout<<note<<(*itr).Note;
            }
            fout<<endl;

            for(itemitr = (*itr).Items.begin(); 
                itemitr != (*itr).Items.end(); 
                ++itemitr)
            {
                fout<<(*itemitr).Key<<'=';
                fout<<'\"';
                fout<<(*itemitr).Value;
                fout<<'\"';
                if(!(*itemitr).Note.empty())
                    fout<<note<<(*itemitr).Note;
                fout<<endl;
            }
            // 组与组之间空一行 
            fout<<endl;
        }
        //fout.flush();
        fout.close();
        return true;
    }
    /// 关闭配置文件 
    inline void Close()
    {
        grps.clear();
    }
    //-----------------------------------------------------
    /// 获取第一个分组 
    inline iterator begin()
    {
        return grps.begin();
    }
    /// 获取第一个分组
    inline const_iterator begin() const
    {
        return grps.begin();
    }
    /// 获取最后一个分组 
    inline iterator end()
    {
        return grps.end();
    }
    /// 获取最后一个分组 
    inline const_iterator end() const 
    {
        return grps.end();
    }
    /// 分组数目  
    inline size_t size() const
    {
        return grps.size();
    }
    //----------------------------------------------------- 
    /// 插入新的分组 
    iterator Insert(const string& grpName)
    {
        iterator itr = Get(grpName);
        if(itr == end())
        {
            grps.push_back(IniGroup());
            itr = grps.end();
            --itr;
        }
        return itr;
    }
    /// 删除项，返回删除的迭代器,返回end则说明没有做删除操作 
    iterator Remove(const string& grpName)
    {
        list<IniGroup>::iterator itr = Get(grpName);
        // 存在则删除
        if(itr != grps.end())
        {
            grps.erase(itr);
            return itr;
        }
        return end();
    }
    /// 获取指定分组名称的分组迭代器,返回end则说明没有查找到  
    iterator Get(const string& name, bool ignoreCase = true)
    {
        iterator itr;
        for(itr = grps.begin(); itr != grps.end(); ++itr)
        {
            if(StringConvert::Compare(
                ByteArray(itr->Name.c_str(), itr->Name.length()),
                ByteArray(name.c_str(), name.length()), ignoreCase))
            {
                return itr;
            }
        }
        return grps.end();
    }
    /// 获取指定分组名称的分组迭代器,返回end则说明没有查找到  
    const_iterator Get(const string& name, bool ignoreCase = true) const 
    {
        const_iterator itr;
        for(itr = grps.begin(); itr != grps.end(); ++itr)
        {
            if(StringConvert::Compare(
                ByteArray(itr->Name.c_str(), itr->Name.length()),
                ByteArray(name.c_str(), name.length()), ignoreCase))
            {
                return itr;
            }
        }
        return grps.end();
    }
    /// 分组键值索引  
    IniGroup& operator[](const string& grpName)
    {
        list<IniGroup>::iterator itr = Get(grpName);
        if(itr == grps.end())
            return EmptyGroup;
        return (*itr);
    }
    /// 分组键值索引 
    const IniGroup& operator[](const string& grpName) const
    {
        list<IniGroup>::const_iterator itr = Get(grpName);
        if(itr == grps.end())
            return EmptyGroup;
        return (*itr);
    }
    //-----------------------------------------------------
};
//---------------------------------------------------------
} // namespace container
} // namespace zhou_yb
//=========================================================
