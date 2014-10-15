#ifndef SgConfigH
#define SgConfigH

/* if ODBCVER is not defined, assume version 3.51 */
#define ODBCVER 0x0351

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
   #define SG_WINDOWS
#endif

#if defined(_UNICODE) && !defined(UNICODE)
   #define UNICODE
#endif

typedef int  SGRETCODE;
typedef bool SGRETBOOL;

#define SG_SUCESS   0
#define SG_WARNING  1
#define SG_ERROR    (-1)

#include <string>
#include <istream>
#include <ostream>
#include <sstream>

#define SgMoveMemory(D,S,L) memmove((D),(S),(L))
#define SgCopyMemory(D,S,L) memcpy((D),(S),(L))
#define SgFillMemory(D,L,V) memset((D),(V),(L))
#define SgZeroMemory(D,L)   memset((D),0,(L))

#define SMALL_MSG     64
#define SHORT_MSG    128
#define NORMAL_MSG   256
#define LONG_MSG     512
#define BIG_MSG     1024

#if defined(UNICODE)
   #define SgFormatMsg(cBuffer,nSize,cMsg, ...) \
      _snwprintf((cBuffer),(nSize),(cMsg),__VA_ARGS__)
#else
   #define SgFormatMsg(cBuffer,nSize,cMsg, ...) \
      _snprintf((cBuffer),(nSize),(cMsg),__VA_ARGS__)
#endif /* !UNICODE */

namespace sfg {
#if defined(UNICODE)
   typedef std::wistream       SgIStream;
   typedef std::wostream       SgOStream;
   typedef std::wiostream      SgIOStream;
   typedef std::wstring        SgString;
   typedef std::wistringstream SgIStringStream;
   typedef std::wostringstream SgOStringStream;
   typedef std::wstringstream  SgStringStream;
#else
   typedef std::istream        SgIStream;
   typedef std::ostream        SgOStream;
   typedef std::iostream       SgIOStream;
   typedef std::string         SgString;
   typedef std::istringstream  SgIStringStream;
   typedef std::ostringstream  SgOStringStream;
   typedef std::stringstream   SgStringStream;
#endif /* !UNICODE */
}; //end namespace sfg

#include <tchar.h>
#ifndef _TCHAR_DEFINED
   typedef _TCHAR TCHAR;
   #define _TCHAR_DEFINED
#endif /* !_TCHAR_DEFINED */

#undef _TEXT
#undef _T
#if defined(UNICODE)
   #define _TEXT(q) L##q
#else
   #define _TEXT(q) q
#endif
#define _T(q) _TEXT(q)

using sfg::SgIStream;
using sfg::SgOStream;
using sfg::SgIOStream;
using sfg::SgString;
using sfg::SgStringStream;
using sfg::SgIStringStream;
using sfg::SgOStringStream;
typedef sfg::SgString String;
typedef std::string AnsiString;
typedef std::wstring WideString;

#endif

