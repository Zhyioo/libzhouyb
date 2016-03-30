//========================================================= 
/**@file Define.h
 * @brief 常用宏,类型的定义
 *
 * @date 2011-10-17   15:18:37
 * @author Zhyioo
 * @version 1.0
 */
#pragma once
//---------------------------------------------------------
#if defined(_MSC_VER) && _MSC_VER <= 1200 // VC6
// 忽略VC6下对宏(DevAdapterBehavior)参数不完整的警告
#   pragma warning(disable:4003)
// 忽略VC6下模板类型超过255个字符的警告 
#   pragma warning(disable:4786)
#endif

// 忽略VS2010下对 sprintf 等不安全的警告 
#if defined(_MSC_VER)
#   pragma warning(disable:4996)
#   pragma warning(disable:4127)
#endif

// VC6没有该宏 
#if !defined(__FUNCTION__) && defined(_MSC_VER)
#    define __FUNCTION__ ""
#endif

#ifndef _MSC_VER
#   define IN_LINUX
#   define __stdcall
#endif

/* 文件路径分隔符 */
#ifdef _MSC_VER
#   define PATH_SEPARATOR '\\'
#else
#   define PATH_SEPARATOR '/'
#endif

/// 函数声明的定义 
#define DLL_FUNCTION(funcName) fp##funcName
/// 定义函数指针 
#define DEF_FUNCTION(funcName) fp##funcName funcName
/// 加载函数 
#ifdef _MSC_VER
#   define LOAD_FUNCTION(hdll, funcName) (funcName = reinterpret_cast<fp##funcName>(GetProcAddress(hdll, #funcName)))
#else
#   define LOAD_FUNCTION(hdll, funcName) (funcName = reinterpret_cast<fp##funcName>(dlsym(hdll, #funcName)))
#endif
//---------------------------------------------------------
//typedef unsigned int   size_t;
typedef unsigned char  byte;
typedef unsigned int   uint;
typedef unsigned short ushort;
typedef void*          pointer;
typedef unsigned long  ulong;
//---------------------------------------------------------
/* Unicode */
#ifdef UNICODE
typedef wchar_t char_t;
#define strlen_t wcslen
#define sprintf_t wsprintf
#define strcpy_t wcscpy
#define strcmp_t wcscmp

#ifndef _T 
#   define _T(x) L##x
#endif

#else
typedef char char_t;
#define strlen_t strlen
#define sprintf_t sprintf
#define strcpy_t strcpy
#define strcmp_t strcmp

#ifndef _T 
#   define _T(x) x
#endif

#endif
//--------------------------------------------------------- 
/// 默认的分隔字符 
#define SPLIT_CHAR '|'
/// 默认的分隔字符串 
#define SPLIT_STRING "|"

/// 每个字节的偏移量
#define BIT_OFFSET      8
#define HALF_BIT_OFFSET 4

/// 转换BOOL类型
#ifndef TRUE 
#   define TRUE  1
#endif

#ifndef FALSE 
#   define FALSE 0
#endif

#ifndef NULL
#   define NULL  0
#endif

#define SIZE_EOF static_cast<size_t>(EOF)

#define ToBOOL(val) ((val) ? TRUE : FALSE)
#define Tobool(val) ((val) == TRUE)

/// C方式的类型转换(仅仅起标识的作用,查找时方便)
#define ctype_cast(type) (type)

/// 宏的最大值 
#define ENUM_MAX(name) MAX_##name

/// 获取数组的大小
#define SizeOfArray(arr)    (sizeof(arr)/sizeof(arr[0]))
/// 获取转换表的长度(相同类型的表)
#define SizeOfTable(table)  (SizeOfArray(table)/2)
/// 设置数组为0 
#define SetArrayZero(arr) (memset(arr,0,sizeof(arr)))
/// 设置数据为0
#define SetObjZero(obj) (memset(&(obj),0,sizeof(obj)))

/// 将string字符串拷贝到C字符数组中 
#define string_to_array(str,arr) do{memcpy((arr),(str).c_str(),(str).length());(arr)[(str).length()]=0;}while(false)
/// 获取C字符串的长度(支持空指针)
#define _strlen(cstr)   ((NULL == (cstr)) ? 0 : strlen(cstr))
/// 获取UNICDOE C字符串的长度(支持空指针)
#define _wcslen(wstr)   ((NULL == (wstr)) ? 0 : wcslen(wstr))
/// 获取char_t格式字符串的长度(支持空指针)
#define _strlen_t(cstr)   ((NULL == (cstr)) ? 0 : strlen_t(cstr))
/// 返回字符串是否为空或长度为0 
#define _is_empty_or_null(cstr) (_strlen(cstr) < 1)
/// 返回字符串是否为空或长度为0 
#define _is_empty_or_null_t(cstr) (_strlen_t(cstr) < 1)
/// 输出字符串(支持空指针)
#define _strput(cstr)   ((NULL == (cstr)) ? "" : static_cast<const char*>(cstr))
/// 输出字符串(支持空指针)
#define _strput_t(cstr)   ((NULL == (cstr)) ? _T("") : static_cast<const char_t*>(cstr))
/// 如果指针为空，则获取另外一个数据
#define _get_pval(ptr,data) (NULL == (ptr) ? (data) : (*ptr))
/// 获取数字中大的那个
#define _max(x,y)       (((x)>(y))?(x):(y))
/// 获取数字中小的那个
#define _min(x,y)       (((x)>(y))?(y):(x))
/// 判断字符是否为数字字符
#define _is_digital(c)      (((c)>='0')&&((c)<='9'))
/// 判断字符是否为小写字母
#define _is_lower(c)    (((c)>='a')&&((c)<='z'))
/// 判断字符是否为大写字母
#define _is_upper(c)    (((c)>='A')&&((c)<='Z'))
/// 判断一个字符是否为字母
#define _is_char(c)     (_is_lower(c) || _is_upper(c))
/// 判断字符是否为16进制字符
#define _is_hex(c)      ((_is_num(c)||(((c)>='A')&&((c)<='F'))||(((c)>='a')&&((c)<='f'))))

#define _itobyte(c) (static_cast<byte>((c)&0x0FF))
// 采用静态查表方式转换数据(效率更高) 
#ifdef NOUSING_STATIC_TRANSTABLE
/// 获取字符的小写形式 
#   define _get_lower(c) (_is_upper(c)?((c)-'A'+'a'):(c))
/// 获取字符的大写形式 
#   define _get_upper(c) (_is_lower(c)?((c)-'a'+'A'):(c))
/// 将字符转换为大写(非字符不变) 
#   define _make_upper(c) (_is_lower(c)?((c)+=('A'-'a')):(c))
/// 将字符转换为小写(非字符不变)
#   define _make_lower(c) (_is_upper(c)?((c)-=('A'-'a')):(c))
#else
/// 获取字符的小写形式 
#   define _get_lower(c) (static_cast<char>(char_lower_table[_itobyte(c)]))
/// 获取字符的大写形式 
#   define _get_upper(c) (static_cast<char>(char_upper_table[_itobyte(c)]))
/// 将字符转换为大写(非字符不变) 
#   define _make_upper(c) ((c)=(static_cast<char>(char_upper_table[_itobyte(c)]))
/// 将字符转换为小写(非字符不变)
#   define _make_lower(c) ((c)=(static_cast<char>(char_lower_table[_itobyte(c)]))
#endif
//--------------------------------------------------------- 
//=========================================================

