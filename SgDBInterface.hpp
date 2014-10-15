//---------------------------------------------------------------------------
#ifndef SgDBInterfaceH
#define SgDBInterfaceH
//---------------------------------------------------------------------------
#include "SgConfig.h"
#undef __APPLIB__ 
#define __APPLIB__ "SgDBInterface"
#include "SgConfigDll.h"

#ifdef SG_WINDOWS
   #include <windows.h>
#endif
#ifndef __SQLTYPES
   #include <sqltypes.h>
#endif
#if !defined(ODBCINT64)
   typedef __int64          SQLBIGINT;
   typedef unsigned __int64 SQLUBIGINT;
#define ODBCINT64
#endif

#if !defined(_SQL_DATETIME_DEF)
#define _SQL_DATETIME_DEF
typedef SQL_TIMESTAMP_STRUCT SQL_DATETIME_STRUCT;
#endif /* _SQL_DATETIME_DEF */

#include <sqlext.h>
#include <functional>
#include "db/Blob.h"

#define RC_SUCCESS(rc) ((rc) == SQL_SUCCESS || (rc) == SQL_SUCCESS_WITH_INFO)
#define MAX_CATALOG_NAME 30
#define MAX_SCHEMA_NAME  30
#define MAX_TABLE_NAME   30
#define MAX_COLUMN_NAME  30
#define SQL_ATTR_CURRENT_SCHEMA 0x4E6

#ifdef __cplusplus
extern "C" {
#endif

namespace sfg {

enum BindType {
   BIND_COLUMN,
   BIND_PARAMETER,
   BIND_INVALID
};

enum DataType {
   TYPE_UNKNOWN        = 0,
   TYPE_CHAR           = 'c',
   TYPE_VARCHAR        = 's',
   TYPE_LONGVARCHAR    = 'h',
   TYPE_DECIMAL        = 'd',
   TYPE_NUMERIC        = 'n',
   TYPE_WCHAR          = 'w',
   TYPE_WVARCHAR       = 'e',
#if defined(UNICODE)
   TYPE_TCHAR          = TYPE_WCHAR,
   TYPE_TVARCHAR       = TYPE_WVARCHAR,
#else
   TYPE_TCHAR          = TYPE_CHAR,
   TYPE_TVARCHAR       = TYPE_VARCHAR,
#endif
   TYPE_WLONGVARCHAR   = 'q',
   TYPE_BIT            = 'b',
   TYPE_STINYINT       = 'k',
   TYPE_UTINYINT       = 'K',
   TYPE_SSMALLINT      = 'j',
   TYPE_USMALLINT      = 'J',
   TYPE_SINTEGER       = 'i',
   TYPE_UINTEGER       = 'I',
   TYPE_SBIGINT        = 'l',
   TYPE_UBIGINT        = 'L',
   TYPE_REAL           = 'r',
   TYPE_DOUBLE         = 'u',
   TYPE_FLOAT          = 'f',
   TYPE_BINARY         = 'y',
   TYPE_VARBINARY      = 'v',
   TYPE_LONGVARBINARY  = 'g',
   TYPE_DATE           = 't',
   TYPE_TIME           = 'm',
   TYPE_TIMESTAMP      = 'p',
   TYPE_STRINGA        = 'S',
   TYPE_STRINGW        = 'W',
   TYPE_BLOB           = 'B',
#if defined(UNICODE)
   TYPE_STRING         = TYPE_STRINGW
#else
   TYPE_STRING         = TYPE_STRINGA
#endif
};

enum ConnState {
   CONN_UNALLOCATED,
   CONN_ALLOCATED,
   CONNECTED
};

enum DbmsType {
   DB_UNKNOWN,
   DB_ORACLE,
   DB_SQL_SERVER,
   DB_MYSQL,
   DB_POSTGRESQL,
   DB_SQLITE,
   DB_OPENRDA,
   DB_INFORMIX,
   DB_DB2,
   DB_ACCESS,
   DB_EXCEL
};

enum StmtState {
   STMT_UNALLOCATED = 0,
   STMT_ALLOCATED,
   STMT_PREPARED,
   STMT_EXECUTED
};

enum RecordsetState {
   RS_NONE,
   RS_EDIT,
   RS_ADD
};

enum DataModelState { 
   DMS_NONE      =(BYTE)0x00,
   DMS_ADDED     =(BYTE)0x01,
   DMS_READ      =(BYTE)0x02,
   DMS_COMMITTED =(BYTE)0x04,
   DMS_CHANGED   =(BYTE)0x08,
   DMS_DELETED   =(BYTE)0x10
};

enum DataModelType { 
   DMT_STMT = 0, 
   DMT_RECORDSET 
};

enum DataModelStmt { 
   DMM_SELECT = 0, 
   DMM_INSERT, 
   DMM_UPDATE, 
   DMM_DELETE 
};

enum MetaFieldAttr {
   MFA_NONE    = 0x00000000,
   MFA_PKKEY   = 0x00000001,
   MFA_NOTNULL = 0x00000002,
   MFA_AUTOINC = 0x00000004,
   MFA_FKKEY   = 0x00000008,
   MFA_INDEX   = 0x00000010,
   MFA_FKLIST  = 0x00000020,
   MFA_BROWSE  = 0x00000040,
   MFA_FILTER  = 0x00000080,
   MFA_SEARCH  = 0x00000100,
   MFA_ORDER   = 0x00000200,
   MFA_MONEY   = 0x00001000
};

enum MetaFieldEdit {
   MFE_MASK_EDIT,
   MFE_TEXT_AREA,
   MFE_COMBO_BOX,
   MFE_CHECK_BOX
};

//---------------------------------------------------------------------------
struct IDBEnvironment;
struct IDBConnection;
struct IDBStmt;
struct IDBRecordset;
struct IDBBind;
struct IDBParam;
struct IDBParams;
struct IDBField;
struct IDBFields;
struct IDBDataModel;
struct IDBVirtualField;
struct IDBVirtualFields;
struct IDBMetaTable;
struct IDBMetaField;
struct IDBMetaFields;
struct IDBRecordsetSchema;
//---------------------------------------------------------------------------
struct IDBException
{
public://Properties
   virtual const String& getExType() = 0;
   virtual const String& getMethod() = 0;
   virtual const String& getErrMsg() = 0;
   virtual const String& getSQLState() = 0;
   virtual const String& getSQLMsg() = 0;
   virtual SQLINTEGER getSQLErr() = 0;
protected:
   IDBException() throw() {};
public://Methods
   virtual ~IDBException() throw() {};
   virtual const String& twhat() throw() = 0;
   virtual void clear() = 0;
};
SGAPI IDBException* SGENTRY GetLastDBException(void);
//---------------------------------------------------------------------------
struct IDBObject
{
public://Properties
   virtual void setTag(void* Value) = 0;
   virtual void* getTag() = 0;
private://Private constructors, not instance directly abstract class
   IDBObject(const IDBObject&){};
   IDBObject& operator=(const IDBObject&){};
protected:
   IDBObject(){};
public://Methods
   virtual ~IDBObject(){};
   virtual void AddRef() = 0;
   virtual void Release() = 0;
};
//---------------------------------------------------------------------------
struct IDBEnvironment : public virtual IDBObject
{
public://Properties
   virtual SQLHENV getHENV() = 0;
private://Private constructors, not instance directly abstract class
   IDBEnvironment(const IDBEnvironment&){};
   IDBEnvironment& operator=(const IDBEnvironment&){};
protected:
   IDBEnvironment(){};
public://Methods
   virtual ~IDBEnvironment(){};
   virtual SQLRETURN ReleaseHandles() = 0;
   virtual SQLRETURN Init(bool bPool = true) = 0;
};
SGAPI IDBEnvironment* SGENTRY GetDBEnvironment1(void);
SGAPI IDBEnvironment* SGENTRY GetDefaultEnvironment(void);
//---------------------------------------------------------------------------
struct IDBConnection : public virtual IDBObject
{
public://Properties
   virtual const String& getStrConn() = 0;
   virtual SQLRETURN setStrConn(const String& Value) = 0;
   virtual SQLHDBC getHDBC() = 0;
   virtual IDBEnvironment* getEnvironment() = 0;
   virtual const String& getDBMSName() = 0;
   virtual DbmsType getDBMSType() = 0;
   virtual const String& getDBMSVersion() = 0;
   virtual const String& getDatabaseName() = 0;
   virtual const String& getUserName() = 0;
   virtual const String& getQuoteChar() = 0;
   virtual const String& getDriverName() = 0;
   virtual const String& getDriverVersion() = 0;
   virtual SQLUSMALLINT getMaxCatalogName() = 0;
   virtual SQLUSMALLINT getMaxSchemaName() = 0;
   virtual SQLUSMALLINT getMaxTableName() = 0;
   virtual SQLUSMALLINT getMaxColumnName() = 0;
   virtual SQLUINTEGER getScrollOptions() = 0;
   virtual const String& getCatalogName() = 0;
   virtual const String& getSchemaName() = 0;
   virtual int getStmtCount() = 0;
private://Private constructors, not instance directly abstract class
   IDBConnection(const IDBConnection&){};
   IDBConnection& operator=(const IDBConnection&){};
protected:
   IDBConnection(){};
public://Methods
   virtual ~IDBConnection(){};
   virtual SQLRETURN ReleaseHandles() = 0;
   virtual SQLRETURN Connect() = 0;
   virtual SQLRETURN Connect(const String& cStrConn, bool bPrompt = false) = 0;
   virtual SQLRETURN Disconnect() = 0;
   virtual SQLRETURN BeginTrans() = 0;
   virtual SQLRETURN Commit() = 0;
   virtual SQLRETURN Roolback() = 0;
   virtual bool IsConnected() = 0;
   //virtual SQLRETURN IsConnected(bool *bValue) = 0;
};
SGAPI IDBConnection* SGENTRY GetDBConnection1(
      IDBEnvironment* oEnvironment);
SGAPI IDBConnection* SGENTRY GetDBConnection2(
      IDBEnvironment* oEnvironment, const String& sStrConn);
SGAPI IDBConnection* SGENTRY GetDefaultConnection(void);
//---------------------------------------------------------------------------
//typedef void (*StmtNotify)(IDBStmt*);
typedef std::function<void(IDBStmt*)> StmtNotify;
struct IDBStmt : public virtual IDBObject
{
public://Properties
   virtual const String& getSQLCmd() = 0;
   virtual void setSQLCmd(const String& Value) = 0;
   virtual SQLHSTMT getHSTMT() = 0;
   virtual IDBConnection* getConnection() = 0;
   virtual IDBParams* getParams() = 0;
private://Private constructors, not instance directly abstract class
   IDBStmt(const IDBStmt&){};
   IDBStmt& operator=(const IDBStmt&){};
protected:
   IDBStmt(){};
public://Methods
   virtual ~IDBStmt(){};
   virtual SQLRETURN ReleaseHandles() = 0;
   virtual bool IsReady() = 0;
   virtual bool IsAllocated() = 0;
   virtual bool IsExecuted() = 0;
   virtual SQLLEN RowCount() = 0;
/*
   virtual SQLRETURN IsReady(bool *bValue) = 0;
   virtual SQLRETURN IsAllocated(bool *bValue) = 0;
   virtual SQLRETURN IsExecuted(bool *bValue) = 0;
   virtual SQLRETURN RowCount(SQLLEN *nValue) = 0;
*/
   virtual SQLRETURN Prepare() = 0;
   virtual SQLRETURN Execute() = 0;
   virtual SQLRETURN ExecDirect() = 0;
   virtual SQLRETURN ClearStmtAttrs() = 0;
   virtual SQLRETURN GetStmtAttr(SQLINTEGER nAttribute, SQLPOINTER pValue, SQLINTEGER nLength,
		SQLINTEGER* StringLengthPointer) = 0;
   virtual SQLRETURN SetStmtAttr(SQLINTEGER nAttribute, SQLPOINTER pValue, SQLINTEGER nLength) = 0;
   virtual SQLRETURN ClearStmtAttr(SQLINTEGER nAttribute) = 0;

   virtual IDBParam* ParamByIndex(int nIndex) = 0;
   virtual IDBParam* ParamByName(const String& sName) = 0;
/*
   virtual SQLRETURN ParamByIndex(int nIndex, IDBParam** oParam) = 0;
   virtual SQLRETURN ParamByName(const String& sName, IDBParam** oParam) = 0;
*/
public: //Eventos
   virtual void SetAfterPrepare(StmtNotify pValue) = 0;
   virtual void SetAfterExecute(StmtNotify pValue) = 0;
};
SGAPI IDBStmt* SGENTRY GetDBStmt1(
      IDBConnection* oConnection);
SGAPI IDBStmt* SGENTRY GetDBStmt2(
      IDBConnection* oConnection, const String& sSQLCmd, bool bPrepare = false);
//---------------------------------------------------------------------------
//typedef void (*RecordsetNotify)(IDBRecordset*);
typedef std::function<void(IDBRecordset*)> RecordsetNotify;
struct IDBRecordset : public virtual IDBStmt
{
public://Properties
   virtual IDBFields* getFields() = 0;
   virtual RecordsetState getRsState() = 0;
   virtual bool getBOF() = 0;
   virtual bool getEOF() = 0;
   virtual void setCursorType(SQLUINTEGER nValue) = 0;
   virtual SQLUINTEGER getCursorType() = 0;
/*
   virtual SQLRETURN getFields(IDBFields** oFields) = 0;
   virtual SQLRETURN setFields(IDBFields* oFields) = 0;
   virtual SQLRETURN getRsState(RecordsetState *eState) = 0;
   virtual SQLRETURN getBOF(bool *bValue) = 0;
   virtual SQLRETURN getEOF(bool *bValue) = 0;
   virtual SQLRETURN setCursorType(SQLUINTEGER nValue) = 0;
   virtual SQLRETURN getCursorType(SQLUINTEGER *nValue) = 0;
*/
protected:
   virtual SQLRETURN Execute() = 0;
   virtual SQLRETURN ExecDirect() = 0;
protected:
   IDBRecordset(){};
public://Methods
   virtual ~IDBRecordset(){};
   virtual SQLRETURN ReleaseHandles() = 0;
   virtual SQLRETURN Open(const String& sSQLCmd, SQLUINTEGER nCursorType = SQL_CURSOR_TYPE_DEFAULT) = 0;
   virtual SQLRETURN Open() = 0;
   virtual SQLRETURN Close() = 0;
   virtual SQLRETURN MoveFirst() = 0;
   virtual SQLRETURN MoveLast() = 0;
   virtual SQLRETURN MoveNext() = 0;
   virtual SQLRETURN MovePrew() = 0;
   virtual SQLRETURN RefreshRow() = 0;
   virtual SQLRETURN Edit() = 0;
   virtual SQLRETURN Post() = 0;
   virtual SQLRETURN Add() = 0;
   virtual SQLRETURN Delete() = 0;
   virtual IDBField* FieldByIndex(int nIndex) = 0;
   virtual IDBField* FieldByName(const String& sName) = 0;
/*
   virtual SQLRETURN FieldByIndex(int nIndex, IDBField** oField) = 0;
   virtual SQLRETURN FieldByName(const String& sName, IDBField** oField) = 0;
*/
public: //Eventos
   virtual void SetBeforeOpen(RecordsetNotify pValue) = 0;
   virtual void SetAfterOpen(RecordsetNotify pValue) = 0;
   virtual void SetBeforeClose(RecordsetNotify pValue) = 0;
   virtual void SetAfterClose(RecordsetNotify pValue) = 0;
   virtual void SetAfterScroll(RecordsetNotify pValue) = 0;
};
SGAPI IDBRecordset* SGENTRY GetDBRecordset1(
      IDBConnection* oConnection);
SGAPI IDBRecordset* SGENTRY GetDBRecordset2(
      IDBConnection* oConnection, const String& sSQLCmd, bool bPrepare = false);
//---------------------------------------------------------------------------
typedef std::function<void(IDBBind*)> BindNotify;
struct IDBBind : public virtual IDBObject
{
public://Properties
   virtual BindType getBindType() = 0;
   virtual DataType getDataType() = 0;
   virtual SQLULEN getDataSize() = 0;
   virtual SQLSMALLINT getSQLType() = 0;
   virtual SQLSMALLINT getNativeType() = 0;
   virtual SQLULEN getNativeSize() = 0;
   virtual SQLSMALLINT getDecimalDigits() = 0;
   virtual bool getNotNull() = 0;
   virtual SQLPOINTER getPointer() = 0;
/*
   virtual SQLRETURN getBindType(BindType *eValue) = 0;
   virtual SQLRETURN getDataType(DataType *eValue) = 0;
   virtual SQLRETURN getSQLType(SQLSMALLINT *nValue) = 0;
   virtual SQLRETURN getNativeType(SQLSMALLINT *nValue) = 0;
   virtual SQLRETURN getSize(SQLSMALLINT *nValue) = 0;
   virtual SQLRETURN getDataSize(SQLULEN *nValue) = 0;
   virtual SQLRETURN getDecimalDigits(SQLSMALLINT *nValue) = 0;
   virtual SQLRETURN getNotNull(bool *bValue) = 0;
   virtual SQLRETURN getPointer(SQLPOINTER *pValue) = 0;
*/
   virtual SQLRETURN setDataType(const DataType Value) = 0;
   virtual SQLRETURN setDataSize(const SQLULEN Value) = 0;
   virtual SQLRETURN setDecimalDigits(const SQLSMALLINT Value) = 0;
   virtual SQLRETURN setNotNull(const bool Value) = 0;
private://Private constructors, not instance directly abstract class
   IDBBind(const IDBBind&){};
   IDBBind& operator=(const IDBBind&){};
protected:
   IDBBind(){};
public://Methods
   virtual ~IDBBind(){};
   virtual SQLRETURN SetNull() = 0;
   virtual bool IsNull() = 0;
   virtual bool IsBind() = 0;
   virtual const String& AsString() = 0;
   virtual int AsInteger() = 0;
   virtual float AsFloat() = 0;
   virtual double AsDouble() = 0;
   virtual bool AsBoolean() = 0;
   virtual SQL_DATETIME_STRUCT AsDateTime() = 0;
/*
   virtual SQLRETURN IsNull(bool* bValue) = 0;
   virtual SQLRETURN IsBind(bool* bValue) = 0;
   virtual SQLRETURN AsString(TCHAR* cValue, size_t &nStrLen) = 0;
   virtual SQLRETURN AsInteger(int* nValue) = 0;
   virtual SQLRETURN AsFloat(float* nValue) = 0;
   virtual SQLRETURN AsDouble(double* nValue) = 0;
   virtual SQLRETURN AsBoolean(bool* bValue) = 0;
   virtual SQLRETURN AsDateTime(SQL_DATETIME_STRUCT* dValue) = 0;
*/
   virtual SQLRETURN SetValue(const char* Value, size_t nStrLen) = 0;
   virtual SQLRETURN SetValue(const WCHAR* Value, size_t nStrLen) = 0;
   virtual SQLRETURN SetValue(const bool Value) = 0;
   virtual SQLRETURN SetValue(const short Value) = 0;
   virtual SQLRETURN SetValue(const int Value) = 0;
   virtual SQLRETURN SetValue(const long Value) = 0;
   virtual SQLRETURN SetValue(const __int64 Value) = 0;
   virtual SQLRETURN SetValue(const float Value, size_t nDecimal) = 0;
   virtual SQLRETURN SetValue(const double Value, size_t nDecimal) = 0;
   virtual SQLRETURN SetValue(const SQL_DATE_STRUCT Value) = 0;
   virtual SQLRETURN SetValue(const SQL_TIME_STRUCT Value) = 0;
   virtual SQLRETURN SetValue(const SQL_DATETIME_STRUCT Value) = 0;
   virtual SQLRETURN SetValue(const AnsiString* Value) = 0;
   virtual SQLRETURN SetValue(const WideString* Value) = 0;
   virtual SQLRETURN SetValue(const Blob* Value) = 0;
   virtual SQLRETURN SetValue(const AnsiString& Value) = 0;
   virtual SQLRETURN SetValue(const WideString& Value) = 0;
   virtual SQLRETURN SetValue(const Blob& Value) = 0;
   virtual SQLRETURN CopyValue(IDBBind* oBind) = 0;
   virtual SQLRETURN operator<<(const char* Value) = 0;
   virtual SQLRETURN operator<<(const WCHAR* Value) = 0;
   virtual SQLRETURN operator<<(const bool Value) = 0;
   virtual SQLRETURN operator<<(const short Value) = 0;
   virtual SQLRETURN operator<<(const int Value) = 0;
   virtual SQLRETURN operator<<(const long Value) = 0;
   virtual SQLRETURN operator<<(const __int64 Value) = 0;
   virtual SQLRETURN operator<<(const float Value) = 0;
   virtual SQLRETURN operator<<(const double Value) = 0;
   virtual SQLRETURN operator<<(const SQL_DATE_STRUCT Value) = 0;
   virtual SQLRETURN operator<<(const SQL_TIME_STRUCT Value) = 0;
   virtual SQLRETURN operator<<(const SQL_DATETIME_STRUCT Value) = 0;
   virtual SQLRETURN operator<<(const AnsiString* Value) = 0;
   virtual SQLRETURN operator<<(const WideString* Value) = 0;
   virtual SQLRETURN operator<<(const Blob* Value) = 0;
   virtual SQLRETURN operator<<(const AnsiString& Value) = 0;
   virtual SQLRETURN operator<<(const WideString& Value) = 0;
   virtual SQLRETURN operator<<(const Blob& Value) = 0;
   virtual void SetAfterSetValue(BindNotify pValue) = 0;
};
//---------------------------------------------------------------------------
//typedef void (*ParamData)(IDBParam*,SQLPOINTER,SQLLEN);
typedef std::function<void(IDBParam*,SQLPOINTER,SQLLEN)> ParamData;
struct IDBParam : public virtual IDBBind
{
public://Properties
   virtual IDBParams* getParams() = 0;
   virtual SQLSMALLINT getParamNum() = 0;
   virtual const String& getParamName() = 0;
   virtual void setParamName(const String& Value) = 0;
   virtual SQLSMALLINT getParamType() = 0;
   virtual void setParamType(const SQLSMALLINT Value) = 0;
/*
   virtual SQLRETURN getParams(IDBParams** oParams) = 0;
   virtual SQLRETURN getParamNum(SQLSMALLINT *nValue) = 0;
   virtual SQLRETURN getParamName(TCHAR *cValue, size_t &nStrLen) = 0;
   virtual SQLRETURN setParamName(const String& Value) = 0;
   virtual SQLRETURN getParamType(SQLSMALLINT *nValue) = 0;
   virtual SQLRETURN setParamType(const SQLSMALLINT Value) = 0;
*/
protected:
   IDBParam(){};
public://Methods
   virtual ~IDBParam() {};
   virtual void SetOnParamPutData(ParamData pValue) = 0;
   virtual void SetOnParamGetData(ParamData pValue) = 0;
};
SGAPI IDBParam* SGENTRY GetDBParam1(
      IDBParams* oParams);
SGAPI IDBParam* SGENTRY GetDBParam2(
      IDBParams* oParams, SQLSMALLINT nParamNum, const String& sParamName,
      DataType eDataType, SQLULEN nDataSize, SQLSMALLINT nDecimalDigits = 0,
      bool bNotNull = false, SQLSMALLINT nParamType = SQL_PARAM_INPUT);
SGAPI IDBParam* SGENTRY GetDBParam3(
      IDBParams* oParams, SQLSMALLINT nParamNum, const String& sParamName,
      SQLPOINTER pPointer, DataType eDataType, SQLULEN nDataSize, SQLSMALLINT nDecimalDigits = 0,
      bool bNotNull = false, SQLSMALLINT nParamType = SQL_PARAM_INPUT);
//---------------------------------------------------------------------------
struct IDBParams : public virtual IDBObject
{
public://Properties
   virtual IDBStmt* getStmt() = 0;
private://Private constructors, not instance directly abstract class
   IDBParams(const IDBParams&){};
   IDBParams& operator=(const IDBParams&){};
protected:
   IDBParams(){};
public://Methods
   virtual ~IDBParams(){};
   virtual int Count() = 0;
   virtual IDBParam* ParamByIndex(int nIndex) = 0;
   virtual IDBParam* ParamByName(const String& sName) = 0;
   virtual IDBParam* Add(IDBParam* oParam) = 0;
   virtual IDBParam* Insert(int nIndex, IDBParam* oParam) = 0;
   virtual IDBParam* Remove(int nIndex) = 0;
/*
   virtual SQLRETURN Count(int *nValue) = 0;
   virtual SQLRETURN ParamByIndex(int nIndex, IDBParam** oParam) = 0;
   virtual SQLRETURN ParamByName(const String& sName, IDBParam** oParam) = 0;
   virtual SQLRETURN Add(IDBParam* oParam) = 0;
   virtual SQLRETURN Insert(int nIndex, IDBParam* oParam) = 0;
   virtual SQLRETURN Remove(int nIndex, IDBParam** oParam) = 0;
*/
   virtual SQLRETURN BindParams() = 0;
   virtual SQLRETURN UnbindParams() = 0;
};
SGAPI IDBParams* SGENTRY GetDBParams1(
      IDBStmt* oStmt);
//---------------------------------------------------------------------------
//typedef void (*FieldData)(IDBField*,SQLPOINTER,SQLLEN);
typedef std::function<void(IDBField*,SQLPOINTER,SQLLEN)> FieldData;
struct IDBField : public virtual IDBBind
{
public://Properties
   virtual IDBFields* getFields() = 0;
   virtual SQLSMALLINT getFieldNum() = 0;
   virtual const String& getFieldName() = 0;
   virtual void setFieldName(const String& Value) = 0;
   virtual bool getAutoIncrement() = 0;
   virtual SQLRETURN setAutoIncrement(const bool Value) = 0;
   virtual bool getIsPkKey() = 0;
   virtual SQLRETURN setIsPkKey(const bool Value) = 0;
   virtual bool getIsVirtual() = 0;
   virtual SQLRETURN setIsVirtual(const bool Value) = 0;
   virtual bool getReadOnly() = 0;
/*
   virtual SQLRETURN getFields(IDBFields** oFields) = 0;
   virtual SQLRETURN getFieldNum(SQLSMALLINT *nValue) = 0;
   virtual SQLRETURN getFieldName(TCHAR *cValue, size_t &nStrLen) = 0;
   virtual SQLRETURN setFieldName(const String& Value) = 0;
   virtual SQLRETURN getAutoIncrement(bool *bValue) = 0;
   virtual SQLRETURN setAutoIncrement(bool Value) = 0;
*/
protected:
   IDBField() {};
public://Methods
   virtual ~IDBField() {};
   virtual void SetOnFieldPutData(FieldData pValue) = 0;
   virtual void SetOnFieldGetData(FieldData pValue) = 0;
};
SGAPI IDBField* SGENTRY GetDBField1(
      IDBFields* oFields);
SGAPI IDBField* SGENTRY GetDBField2(
      IDBFields* oFields, SQLSMALLINT nFieldNum, const String& sFieldName,
      DataType eDataType, SQLULEN nDataSize, SQLSMALLINT nDecimalDigits = 0,
      bool bNotNull = false, bool bIsPkKey = false, bool bIsVirtual = false);
SGAPI IDBField* SGENTRY GetDBField3(
      IDBFields* oFields, SQLSMALLINT nFieldNum, const String& sFieldName,
      SQLPOINTER pPointer, DataType eDataType, SQLULEN nDataSize, SQLSMALLINT nDecimalDigits = 0,
      bool bNotNull = false, bool bIsPkKey = false, bool bIsVirtual = false);
//---------------------------------------------------------------------------
struct IDBFields : public virtual IDBObject
{
public://Properties
   virtual IDBRecordset* getRecordset() = 0;
private://Private constructors, not instance directly abstract class
   IDBFields(const IDBFields&){};
   IDBFields& operator=(const IDBFields&){};
protected:
   IDBFields(){};
public://Methods
   virtual ~IDBFields(){};
   virtual int Count() = 0;
   virtual IDBField* FieldByIndex(int nIndex) = 0;
   virtual IDBField* FieldByName(const String& sName) = 0;
   virtual IDBField* Add(IDBField* oField) = 0;
   virtual IDBField* Insert(int nIndex, IDBField* oField) = 0;
   virtual IDBField* Remove(int nIndex) = 0;
/*
   virtual SQLRETURN Count(int *nValue) = 0;
   virtual SQLRETURN FieldByIndex(int nIndex, IDBField** oField) = 0;
   virtual SQLRETURN FieldByName(const String& sName, IDBField** oField) = 0;
   virtual SQLRETURN Add(IDBField* oField) = 0;
   virtual SQLRETURN Insert(int nIndex, IDBField* oField) = 0;
   virtual SQLRETURN Remove(int nIndex, IDBField** oField) = 0;
*/
   virtual SQLRETURN BindFields() = 0;
   virtual SQLRETURN UnbindFields() = 0;
};
SGAPI IDBFields* SGENTRY GetDBFields1(
      IDBRecordset* oRecordset);
//---------------------------------------------------------------------------
struct IDBMetaTable : public virtual IDBObject
{
public://Properties
   virtual IDBConnection* getConnection() = 0;
   virtual IDBMetaFields* getMetaFields() = 0;
   virtual void setMetaFields(IDBMetaFields* oMetaFields) = 0;
   virtual const String& getTableName() = 0;
   virtual SQLRETURN setTableName(const String& Value) = 0;
private://Private constructors, not instance directly abstract class
   IDBMetaTable(const IDBMetaTable&){};
   IDBMetaTable& operator=(const IDBMetaTable&){};
protected:
   IDBMetaTable(){};
public://Methods
   virtual ~IDBMetaTable(){};
   virtual IDBMetaField* FieldByIndex(int nIndex) = 0;
   virtual IDBMetaField* FieldByName(const String& sName) = 0;
/*
   virtual SQLRETURN FieldByIndex(int nIndex, IDBMetaField** oField) = 0;
   virtual SQLRETURN FieldByName(const String& sName, IDBMetaField** oField) = 0;
*/
};
SGAPI IDBMetaTable* SGENTRY GetDBMetaTable1(
      IDBConnection* oConnection);
SGAPI IDBMetaTable* SGENTRY GetDBMetaTable2(
      IDBConnection* oConnection, const String& sTableName, bool bCreateFields = true);
//---------------------------------------------------------------------------
struct IDBMetaField : public virtual IDBObject
{
public://Properties
   virtual IDBMetaFields* getMetaFields() = 0;
   virtual int getFieldNum() = 0;
   virtual void setFieldNum(const int Value) = 0;
   virtual const String& getFieldName() = 0;
   virtual void setFieldName(const String& Value) = 0;
   virtual DataType getDataType() = 0;
   virtual void setDataType(const DataType Value) = 0;
   virtual int getDataSize() = 0;
   virtual void setDataSize(const int Value) = 0;
   virtual int getDecimalDigits() = 0;
   virtual void setDecimalDigits(const int Value) = 0;
   virtual int getProperties() = 0;
   virtual void setProperties(const int Value) = 0;
   virtual bool getIsPkKey() = 0;
   virtual void setIsPkKey(const bool Value) = 0;
   virtual bool getNotNull() = 0;
   virtual void setNotNull(const bool Value) = 0;
   virtual bool getAutoIncrement() = 0;
   virtual void setAutoIncrement(const bool Value) = 0;
   virtual bool getIsFkKey() = 0;
   virtual void setIsFkKey(const bool Value) = 0;
   virtual bool getIsIndex() = 0;
   virtual void setIsIndex(const bool Value) = 0;
   virtual bool getIsFkList() = 0;
   virtual void setIsFkList(const bool Value) = 0;
   virtual bool getIsBrowse() = 0;
   virtual void setIsBrowse(const bool Value) = 0;
   virtual bool getIsFilter() = 0;
   virtual void setIsFilter(const bool Value) = 0;
   virtual bool getIsSearch() = 0;
   virtual void setIsSearch(const bool Value) = 0;
   virtual bool getIsOrder() = 0;
   virtual void setIsOrder(const bool Value) = 0;   
   virtual bool getIsMoney() = 0;
   virtual void setIsMoney(const bool Value) = 0;
   virtual MetaFieldEdit getEditCmp() = 0;
   virtual void setEditCmp(const MetaFieldEdit Value) = 0;
   virtual const String& getEditMask() = 0;
   virtual void setEditMask(const String& Value) = 0;
   virtual const String& getFieldLabel() = 0;
   virtual void setFieldLabel(const String& Value) = 0;
   virtual const String& getCheckValues() = 0;
   virtual void setCheckValues(const String& Value) = 0;
   virtual const String& getComboOptions() = 0;
   virtual void setComboOptions(const String& Value) = 0;
   virtual const String& getFkTableName() = 0;
   virtual void setFkTableName(const String& Value) = 0;
   virtual const String& getFkColumnName() = 0;
   virtual void setFkColumnName(const String& Value) = 0;
   virtual bool getIsVirtual() = 0;
   virtual void setIsVirtual(const bool Value) = 0;
private://Private constructors, not instance directly abstract class
   IDBMetaField(const IDBMetaField&){};
   IDBMetaField& operator=(const IDBMetaField&){};
protected:
   IDBMetaField(){};
public://Methods
   virtual ~IDBMetaField(){};
};
SGAPI IDBMetaField* SGENTRY GetDBMetaField1(
      IDBMetaFields* oMetaFields);
SGAPI IDBMetaField* SGENTRY GetDBMetaField2(
      IDBMetaFields* oMetaFields, int nFieldNum, const String& sFieldName, const String& sFieldLabel,
      MetaFieldEdit eEditCmp, DataType eDataType, int nDataSize, int nDecimalDigits = 0, 
      bool bNotNull = false, bool bIsPkKey = false, bool bAutoIncrement = false, int nProperties = 0,
      const String& sEditMask = _T(""), const String& sCheckValues = _T("1;0"), 
      const String& sFkTableName = _T(""), const String& sFkColumnName = _T(""));
//---------------------------------------------------------------------------
struct IDBMetaFields : public virtual IDBObject
{
public://Properties
   virtual IDBMetaTable* getMetaTable() = 0;
private://Private constructors, not instance directly abstract class
   IDBMetaFields(const IDBMetaFields&){};
   IDBMetaFields& operator=(const IDBMetaFields&){};
protected:
   IDBMetaFields(){};
public://Methods
   virtual ~IDBMetaFields(){};
   virtual int Count() = 0;
   virtual IDBMetaField* FieldByIndex(int nIndex) = 0;
   virtual IDBMetaField* FieldByName(const String& sName) = 0;
   virtual IDBMetaField* Add(IDBMetaField* oMetaField) = 0;
   virtual IDBMetaField* Insert(int nIndex, IDBMetaField* oMetaField) = 0;
   virtual IDBMetaField* Remove(int nIndex) = 0;
/*
   virtual SQLRETURN Count(int *nValue) = 0;
   virtual SQLRETURN FieldByIndex(int nIndex, IDBMetaField** oField) = 0;
   virtual SQLRETURN FieldByName(const String& sName, IDBMetaField** oField) = 0;
   virtual SQLRETURN Add(IDBMetaField* oField) = 0;
   virtual SQLRETURN Insert(int nIndex, IDBMetaField* oField) = 0;
   virtual SQLRETURN Remove(int nIndex, IDBMetaField** oField) = 0;
*/
};
SGAPI IDBMetaFields* SGENTRY GetDBMetaFields1(
      IDBMetaTable* oMetaTable);
//---------------------------------------------------------------------------
typedef std::function<void(IDBDataModel*)> DataModelNotify;
typedef std::function<void(IDBDataModel*,bool&)> DataModelValid;
struct IDBDataModel : public virtual IDBObject
{
public: //Propriedades
   virtual BYTE getDmState() = 0;
   virtual DataModelType getDmType() = 0;
   virtual IDBMetaTable* getMetaTable() = 0;
   virtual IDBMetaFields* getMetaFields() = 0;
   virtual IDBFields* getFields() = 0;
   virtual IDBRecordset* getRecordset() = 0;
   virtual IDBStmt* getStmt(DataModelStmt eStmtType) = 0;
   virtual void setRecordset(IDBRecordset *oRecordset) = 0;
   virtual void setStmt(IDBStmt* oStmt, DataModelStmt eStmtType) = 0;
private://Private constructors, not instance directly abstract class
   IDBDataModel(const IDBDataModel&){};
   IDBDataModel& operator=(const IDBDataModel&){};
protected:
   IDBDataModel(){};
public: //Métodos
   virtual ~IDBDataModel(){};
   virtual void Add() = 0;
   virtual bool Delete(bool bValid = true) = 0;
   virtual bool Read() = 0;
   virtual bool Post(bool bValid = true) = 0;
   virtual bool Cancel() = 0;
   virtual bool Valid() = 0;
   virtual IDBMetaField* MetaFieldByName(const String &sName) = 0;
   virtual IDBMetaField* MetaFieldByIndex(int nIndex) = 0;
   virtual IDBField* FieldByName(const String &sName) = 0;
   virtual IDBField* FieldByIndex(int nIndex) = 0;
public: //Eventos
   virtual void SetAfterRead(DataModelNotify pValue) = 0;
   virtual void SetAfterPost(DataModelNotify pValue) = 0;
   virtual void SetAfterCancel(DataModelNotify pValue) = 0;
   virtual void SetOnCalcFields(DataModelNotify pValue) = 0;
   virtual void SetOnValid(DataModelValid pValue) = 0;
};
SGAPI IDBDataModel* SGENTRY GetDBDataModel1(
      IDBMetaTable *oMetaTable, DataModelType eDmType = DMT_STMT, bool bCreateDBObj = true);
//---------------------------------------------------------------------------
struct IDBRecordsetSchema : public virtual IDBRecordset
{
public: //Proprieadades
   virtual bool getMetadataID() = 0;
private://Private constructors, not instance directly abstract class
   IDBRecordsetSchema(const IDBRecordsetSchema&){};
   IDBRecordsetSchema& operator=(const IDBRecordsetSchema&){};
protected:
   IDBRecordsetSchema(){};
public://Methods
   virtual ~IDBRecordsetSchema(){};
   virtual SQLRETURN Initialize() = 0;
};
//---------------------------------------------------------------------------
SGAPI SQLRETURN SGENTRY GetSchemaTables(IDBRecordsetSchema** oRecordset, IDBConnection* oConnection, const String& sTableName);
SGAPI SQLRETURN SGENTRY GetSchemaColumns(IDBRecordsetSchema** oRecordset, IDBConnection* oConnection, const String& sTableName, const String& sColumnName);
SGAPI SQLRETURN SGENTRY GetSchemaPrimaryKeys(IDBRecordsetSchema** oRecordset, IDBConnection* oConnection, const String& sTableName);
SGAPI SQLRETURN SGENTRY GetSchemaForeignKeys(IDBRecordsetSchema** oRecordset, IDBConnection* oConnection, const String& sTableName);
SGAPI bool SGENTRY ExistTable(IDBConnection* oConnection, const String& sTableName);
SGAPI bool SGENTRY ExistColumn(IDBConnection* oConnection, const String& sTableName, const String& sColumnName);
//---------------------------------------------------------------------------
SGAPI SQLRETURN SGENTRY AddMetaTable(IDBMetaTable* oMetaTable);
SGAPI SQLRETURN SGENTRY AddMetaTableByName(IDBMetaTable** oMetaTable, IDBConnection* oConnection, const String& sTableName);
SGAPI SQLRETURN SGENTRY GetMetaTableByName(IDBMetaTable** oMetaTable, IDBConnection* oConnection, const String& sTableName);
SGAPI SQLRETURN SGENTRY AddMetaTableFromSchema(IDBMetaTable** oMetaTable, IDBConnection* oConnection, const String& sTableName);
//---------------------------------------------------------------------------
SGAPI SQLRETURN SGENTRY GetSQLTables(IDBRecordsetSchema** oRecordset, IDBConnection* oConnection, const String& sTableName, const String& sSQL = _T(""));
SGAPI SQLRETURN SGENTRY GetSQLColumns(IDBRecordsetSchema** oRecordset, IDBConnection* oConnection, const String& sTableName, const String& sColumnName, const String& sSQL = _T(""));
SGAPI SQLRETURN SGENTRY GetSQLPrimaryKeys(IDBRecordsetSchema** oRecordset, IDBConnection* oConnection, const String& sTableName, const String& sSQL = _T(""));
SGAPI SQLRETURN SGENTRY GetSQLForeignKeys(IDBRecordsetSchema** oRecordset, IDBConnection* oConnection, const String& sTableName, const String& sSQL = _T(""));
SGAPI bool SGENTRY ExistTableSQL(IDBConnection* oConnection, const String& sTableName, const String& sSQL = _T(""));
SGAPI bool SGENTRY ExistColumnSQL(IDBConnection* oConnection, const String& sTableName, const String& sColumnName, const String& sSQL = _T(""));
SGAPI SQLRETURN SGENTRY AddMetaTableFromSQL(IDBMetaTable** oMetaTable, IDBConnection* oConnection, const String& sTableName,
      const String& sSQLTable = _T(""), const String& sSQLColumns = _T(""),
      const String& sSQLPk = _T(""), const String& sSQLFk = _T(""));
//---------------------------------------------------------------------------
}; //end namespace sfg

#ifdef __cplusplus
}
#endif

#endif //SgDBInterfaceH
