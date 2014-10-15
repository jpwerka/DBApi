//---------------------------------------------------------------------------
#ifndef DBBindBasicH
#define DBBindBasicH
//---------------------------------------------------------------------------
#include "../SgDBInterface.hpp"

#ifdef _MSC_VER
   #pragma warning(default:4716)
   #pragma warning(disable:4251)
   #pragma warning(disable:4242)
   #pragma warning(disable:4244)
   #pragma warning(disable:4250)
   #pragma warning(disable:4275)
#endif

namespace sfg {
   
enum DataKind { 
   TYPE_PRIMITIVE, 
   TYPE_COMPLEX, 
   TYPE_INVALID 
};
//---------------------------------------------------------------------------
TCHAR* CopyStr(const String& sStrDest, size_t &nStrLen, TCHAR *cStrSrc);
TCHAR* CopyStr(const String& sStrDest, size_t &nStrLen, const String &sStrSrc);
//---------------------------------------------------------------------------
class DBException : public IDBException
{
protected:
   mutable String whatbuf; // buffer used to store result of what() operation
                           // so it doesn't get destroyed on us
private:
   String sExType;
   String sMethod;
   String sErrMsg;
   String sSQLState;
   String sSQLMsg;
   SQLINTEGER nSQLErr;
public:
   const String& getMethod();
   const String& getErrMsg();
   const String& getExType();
   const String& getSQLState();
   const String& getSQLMsg();
   SQLINTEGER getSQLErr() {return nSQLErr;};
   void setMethod(const String& Value) {sMethod = Value;};
   void setErrMsg(const String& Value) {sErrMsg = Value;};
   void setSQLState(const String& Value) {sSQLState = Value;};
   void setSQLMsg(const String& Value) {sSQLMsg = Value;};
   void setSQLErr(SQLINTEGER Value) {nSQLErr = Value;};
public:
   DBException() : IDBException(), sExType(_T("")), sMethod(_T("")), sErrMsg(_T("")),
      sSQLState(_T("")), sSQLMsg(_T("")), nSQLErr(0) {};
   virtual ~DBException() throw() {};
   const String& twhat() throw();
   void clear();
public:
   friend void ClearException();
   friend void SetExceptionInfo(const String& sMethod, const String& sErrMsg,
      const String& sSQLState,const String& sSQLMsg, SQLINTEGER nSQLErr);
};
void ExceptionByHandle(SQLSMALLINT nHandleType, SQLHANDLE hHandle,
      const String& sMethod, const String& sErrMsg);
//---------------------------------------------------------------------------
class DBObject : public virtual IDBObject
{
private://Properties
   void* pTag;
   int   nRefCount;
public://Properties
   void setTag(void* Value);
   void* getTag();
protected:
   DBObject();
public://Methods
   ~DBObject() {};
   virtual void AddRef();
   virtual void Release();
};
//---------------------------------------------------------------------------
class TypeTranslation;
class DBBind : public DBObject, public virtual IDBBind
{
protected://Variables
   TypeTranslation* oType;
   SQLRETURN      nRetcode;
   BindType       eBindType;
   SQLUSMALLINT   nBindNum;
   String         sBindName;
   SQLULEN        nDataSize;
   SQLSMALLINT    nDecimalDigits;
   bool           bNotNull;
   SQLPOINTER     pPointer;
   SQLLEN         nRetLength;
   bool           bAutoGenerate;
   bool           bIsBind;
   bool           bInternalValue;
   bool           bUnsigned;
   String         sStrValue; // To return AsString()
   BindNotify     AfterSetValue;
private://private methods
   void InternalInit(BindType eBindType, SQLSMALLINT nBindNum, const String &sBindName,
         SQLULEN nDataSize, SQLSMALLINT nDecimalDigits, bool bNotNull);
protected://protected methods
   void AllocData();
   void GetLenForBind(SQLLEN &nBufferLen);
   SQLRETURN PutData(SQLHSTMT hStmt);
   SQLRETURN GetData(SQLHSTMT hStmt);
   virtual SQLRETURN Bind();
   virtual SQLRETURN Unbind();
   void ReleasePointer();
public://Properties
   BindType getBindType();
   DataType getDataType();
   SQLULEN getDataSize();
   SQLSMALLINT getSQLType();
   SQLSMALLINT getNativeType();
   SQLULEN getNativeSize();
   SQLSMALLINT getDecimalDigits();
   bool getNotNull();
   SQLPOINTER getPointer();
   SQLRETURN setDataType(const DataType Value);
   SQLRETURN setDataSize(const SQLULEN Value);
   SQLRETURN setDecimalDigits(const SQLSMALLINT Value);
   SQLRETURN setNotNull(const bool Value);
protected:
   DBBind(BindType eBindType, SQLSMALLINT nBindNum);
public://Methods
   DBBind(BindType eBindType, SQLSMALLINT nBindNum, const String &sBindName,
         DataType eDataType, SQLULEN nDataSize, SQLSMALLINT nDecimalDigits, bool bNotNull = false);
   DBBind(BindType eBindType, SQLSMALLINT nBindNum, const String &sBindName, SQLPOINTER pPointer,
         DataType eDataType, SQLULEN nDataSize, SQLSMALLINT nDecimalDigits, bool bNotNull = false);
   ~DBBind();
   SQLRETURN SetNull();
   bool IsNull();
   bool IsBind();
   const String& AsString();
   int AsInteger();
   float AsFloat();
   double AsDouble();
   bool AsBoolean();
   SQL_DATETIME_STRUCT AsDateTime();
   SQLRETURN SetValue(const char* Value, size_t nStrLen);
   SQLRETURN SetValue(const WCHAR* Value, size_t nStrLen);
   SQLRETURN SetValue(const bool Value);
   SQLRETURN SetValue(const short Value);
   SQLRETURN SetValue(const int Value);
   SQLRETURN SetValue(const long Value);
   SQLRETURN SetValue(const __int64 Value);
   SQLRETURN SetValue(const float Value, size_t nDecimal);
   SQLRETURN SetValue(const double Value, size_t nDecimal);
   SQLRETURN SetValue(const SQL_DATE_STRUCT Value);
   SQLRETURN SetValue(const SQL_TIME_STRUCT Value);
   SQLRETURN SetValue(const SQL_DATETIME_STRUCT Value);
   SQLRETURN SetValue(const AnsiString* Value);
   SQLRETURN SetValue(const WideString* Value);
   SQLRETURN SetValue(const Blob* Value);
   SQLRETURN SetValue(const AnsiString& Value);
   SQLRETURN SetValue(const WideString& Value);
   SQLRETURN SetValue(const Blob& Value);
   SQLRETURN CopyValue(IDBBind* oBind);
   SQLRETURN operator<<(const char* Value);
   SQLRETURN operator<<(const WCHAR* Value);
   SQLRETURN operator<<(const bool Value);
   SQLRETURN operator<<(const short Value);
   SQLRETURN operator<<(const int Value);
   SQLRETURN operator<<(const long Value);
   SQLRETURN operator<<(const __int64 Value);
   SQLRETURN operator<<(const float Value);
   SQLRETURN operator<<(const double Value);
   SQLRETURN operator<<(const SQL_DATE_STRUCT Value);
   SQLRETURN operator<<(const SQL_TIME_STRUCT Value);
   SQLRETURN operator<<(const SQL_DATETIME_STRUCT Value);
   SQLRETURN operator<<(const AnsiString* Value);
   SQLRETURN operator<<(const WideString* Value);
   SQLRETURN operator<<(const Blob* Value);
   SQLRETURN operator<<(const AnsiString& Value);
   SQLRETURN operator<<(const WideString& Value);
   SQLRETURN operator<<(const Blob& Value);
   void SetAfterSetValue(BindNotify pValue);
};
//---------------------------------------------------------------------------
class TypeTranslation
{
private:
   DataType       eDataType;
   SQLSMALLINT    nSQLType;
   SQLSMALLINT    nNativeType;
   DataKind       eDataKind;  // is the type primitive or complex
   int            nNativeSize;
public:
   TypeTranslation() {};
   TypeTranslation(DataType eDataType, SQLSMALLINT nSQLType, SQLSMALLINT nNativeType, DataKind eDataKind, SQLULEN nNativeSize);
   DataType getDataType() {return eDataType;};
   SQLSMALLINT getSQLType() {return nSQLType;};
   SQLSMALLINT getNativeType() {return nNativeType;};
   bool IsComplex() {return (bool)(eDataKind == TYPE_COMPLEX);};
   SQLULEN getNativeSize() {return nNativeSize;};
};
//---------------------------------------------------------------------------
DataType GetTypeBySQL(SQLSMALLINT nSQLType, bool bUnsigned = false);
TypeTranslation* GetMapType(DataType eDataType);
TypeTranslation* GetStrMapType(const AnsiString& sStrType);
//---------------------------------------------------------------------------
}; //end namespace sfg
#endif


