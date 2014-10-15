//---------------------------------------------------------------------------
#ifndef DBDatabaseH
#define DBDatabaseH
//---------------------------------------------------------------------------
#include <vector>
#include "DBBindBasic.hpp"

namespace sfg {

class DBEnvironment;
class DBConnection;
class DBStmt;
class DBRecordset;
class DBAttribute;
class DBParam;
class DBParams;
class DBField;
class DBFields;

//---------------------------------------------------------------------------
class DBEnvironment : public DBObject, public virtual IDBEnvironment
{
protected:
   SQLRETURN   nRetcode;
   SQLHENV     hEnv;
public://Properties
   SQLHENV getHENV();
public://Methods
   DBEnvironment();
   ~DBEnvironment();
   void AddRef();
   void Release();
   SQLRETURN ReleaseHandles();
   SQLRETURN Init(bool bPool = true);
};
//---------------------------------------------------------------------------
class DBConnection : public DBObject, public virtual IDBConnection
{
private://Variables,
   SQLRETURN        nRetcode;
   DBEnvironment*   oEnvironment;
   SQLHDBC          hDbc;
   int              nStmtCount;
   String           sStrConn;
   bool             bAutoCommit; // whether to autocommit or not
   ConnState        eConnState; // state of the connection
   DbmsType         eDbmsType;
   String           sQuoteChar;
   String           sDriverName;
   String           sDriverVer;
   String           sDBMSName;
   String           sDBMSVer;
   String           sDatabase;
   String           sUser;
   String           sCatalog;
   String           sSchema;
   SQLUSMALLINT     nMaxCatalogName;
   SQLUSMALLINT     nMaxSchemaName;
   SQLUSMALLINT     nMaxTableName;
   SQLUSMALLINT     nMaxColumnName;
   SQLUINTEGER      nScrollOptions;
   friend class DBStmt;
   friend class DBRecordset;
private://Private Methods
   void InternalInit(IDBEnvironment*,const String&);
   SQLRETURN CreateHandles(bool bPool = true);
   SQLRETURN InternalConnect(SQLTCHAR*, bool bPrompt = false);
   SQLRETURN ComputeDBInfo();
   SQLRETURN ToggleAutoCommit();
public://Properties
   const String& getStrConn();
   SQLRETURN setStrConn(const String &Value);
   SQLHDBC getHDBC();
   IDBEnvironment* getEnvironment();
   const String& getDBMSName();
   DbmsType getDBMSType();
   const String& getDBMSVersion();
   const String& getDatabaseName();
   const String& getUserName();
   const String& getQuoteChar();
   const String& getDriverName();
   const String& getDriverVersion();
   SQLUSMALLINT getMaxCatalogName();
   SQLUSMALLINT getMaxSchemaName();
   SQLUSMALLINT getMaxTableName();
   SQLUSMALLINT getMaxColumnName();
   SQLUINTEGER getScrollOptions();
   const String& getCatalogName();
   const String& getSchemaName();
   int getStmtCount();
public://Methods
   DBConnection(IDBEnvironment *oEnvironment);
   DBConnection(IDBEnvironment *oEnvironment, const String& sStrConn);
   ~DBConnection();
   void AddRef();
   void Release();
   SQLRETURN ReleaseHandles();
   SQLRETURN Connect();
   SQLRETURN Connect(const String& sStrConn, bool bPrompt = false);
   SQLRETURN Disconnect();
   SQLRETURN BeginTrans();
   SQLRETURN Commit();
   SQLRETURN Roolback();
   bool IsConnected();
   SQLRETURN IsConnected(bool *bValue);
};
//---------------------------------------------------------------------------
class DBAttribute
{
private://Variables
   friend class DBStmt;
   friend class DBRecordset;
protected:
   SQLINTEGER nAttribute;
   SQLPOINTER pValue;
   SQLINTEGER nLength;
public:
   DBAttribute();
   DBAttribute(SQLINTEGER nAttribute, SQLPOINTER pValue, SQLINTEGER nLength);
   ~DBAttribute();
};
typedef std::vector<DBAttribute*> DBAttrList;
//---------------------------------------------------------------------------
class DBStmt : public DBObject, public virtual IDBStmt
{
private:
   friend class DBAttribute;
   friend class DBParam;
   friend class DBParams;
   friend class DBField;
   friend class DBFields;
protected://Events
   StmtNotify AfterPrepare;
   StmtNotify AfterExecute;
protected://Variables
   SQLRETURN      nRetcode;
   DBConnection*  oConnection;
   DBParams*      oParams;
   SQLHSTMT       hStmt;
   StmtState      eStmtState; // is hstmt. allocated?
   String         sSQLCmd;
   bool           bPrepare; // if true, use SQLPrepare for this statement
   DBAttrList*    oAttrs; // non-default attributes of the statement
private://Private Methods
   void InternalInit(IDBConnection* oConnection, const String& sSQLCmd,  bool bPrepare);
   SQLRETURN CreateHandles();
   SQLRETURN PutParamData();
   SQLRETURN LoadAttributes();
protected://Protected Methods
   SQLRETURN Initialize(bool bPrepare);
public://Properties
   const String& getSQLCmd();
   void setSQLCmd(const String &Value);
   SQLHSTMT getHSTMT();
   IDBConnection* getConnection();
   IDBParams* getParams();
public://Methods
   DBStmt(IDBConnection* oConnection);
   DBStmt(IDBConnection* oConnection, const String& sSQLCmd, bool bPrepare = false);
   ~DBStmt();
   virtual SQLRETURN ReleaseHandles();
   bool IsReady();
   bool IsAllocated();
   bool IsExecuted();
   SQLLEN RowCount();
   SQLRETURN Prepare();
   virtual SQLRETURN Execute();
   virtual SQLRETURN ExecDirect();
   SQLRETURN ClearStmtAttrs();
   SQLRETURN GetStmtAttr(SQLINTEGER nAttribute, SQLPOINTER pValue, SQLINTEGER nLength,
		SQLINTEGER* StringLengthPointer);
   SQLRETURN SetStmtAttr(SQLINTEGER nAttribute, SQLPOINTER pValue, SQLINTEGER nLength);
   SQLRETURN ClearStmtAttr(SQLINTEGER nAttribute);   
   IDBParam* ParamByIndex(int nIndex);
   IDBParam* ParamByName(const String& sName);
public: //Eventos
   void SetAfterPrepare(StmtNotify pValue);
   void SetAfterExecute(StmtNotify pValue);
};
//---------------------------------------------------------------------------
class DBRecordset : public DBStmt, public virtual IDBRecordset
{
private:
   friend class DBParam;
   friend class DBParams;
   friend class DBField;
   friend class DBFields;
protected://Events
   RecordsetNotify BeforeOpen;
   RecordsetNotify AfterOpen;
   RecordsetNotify BeforeClose;
   RecordsetNotify AfterClose;
   RecordsetNotify AfterScroll;
protected://Variables
   DBFields*       oFields;
   SQLUINTEGER     nCursorType;
   SQLCHAR         cBookmark[10];
   SQLLEN          nBookmarkLen;
   SQLINTEGER      nBindOffset;
   SQLUSMALLINT    nRowStatus;
   SQLUINTEGER     nRowsFetched;
   RecordsetState  eRsState;
   bool            bEOF;
   bool            bBOF;
private:
   SQLRETURN PutFieldData();
   SQLRETURN GetFieldData();
public://Properties
   IDBFields* getFields();
   RecordsetState getRsState();
   bool getBOF();
   bool getEOF();
   void setCursorType(SQLUINTEGER nValue);
   SQLUINTEGER getCursorType();
protected:
   SQLRETURN Execute();
   SQLRETURN ExecDirect();
public://Methods
   DBRecordset(IDBConnection* oConnection);
   DBRecordset(IDBConnection* oConnection, const String& sSQLCmd, bool bPrepare = false);
   ~DBRecordset();
   SQLRETURN ReleaseHandles();
   SQLRETURN Open(const String& sSQLCmd, SQLUINTEGER nCursorType = SQL_CURSOR_TYPE_DEFAULT);
   SQLRETURN Open();
   SQLRETURN Close();
   SQLRETURN MoveFirst();
   SQLRETURN MoveLast();
   SQLRETURN MoveNext();
   SQLRETURN MovePrew();
   SQLRETURN RefreshRow();
   SQLRETURN Edit();
   SQLRETURN Post();
   SQLRETURN Add();
   SQLRETURN Delete();
   IDBField* FieldByIndex(int nIndex);
   IDBField* FieldByName(const String& sName);
   using DBStmt::Prepare;
public: //Eventos
   void SetBeforeOpen(RecordsetNotify pValue); 
   void SetAfterOpen(RecordsetNotify pValue); 
   void SetBeforeClose(RecordsetNotify pValue); 
   void SetAfterClose(RecordsetNotify pValue); 
   void SetAfterScroll(RecordsetNotify pValue);
};
//---------------------------------------------------------------------------
class DBParam : public DBBind, public virtual IDBParam
{
private://Variables
   friend class DBStmt;
   friend class DBRecordset;
   friend class DBParams;
protected://Events
   ParamData OnParamPutData;
   ParamData OnParamGetData;
protected:
   DBParams*    oParams;
   SQLSMALLINT  nParamType;
private://Private methods
   SQLRETURN DescribeParam();
   SQLRETURN Bind();
   SQLRETURN PutData();
   SQLRETURN GetData();
public://Properties
   IDBParams* getParams();
   SQLSMALLINT getParamNum();
   const String& getParamName();
   void setParamName(const String &Value);
   SQLSMALLINT getParamType();
   void setParamType(const SQLSMALLINT Value);
private:
   DBParam(IDBParams* oParams, SQLSMALLINT nParamNum, SQLSMALLINT nParamType = SQL_PARAM_INPUT);
public://Methods
   DBParam(IDBParams* oParams);
   DBParam(IDBParams* oParams, SQLSMALLINT nParamNum, const String& sParamName, DataType eDataType,
         SQLULEN nDataSize, SQLSMALLINT nDecimalDigits, bool bNotNull = false, SQLSMALLINT nParamType = SQL_PARAM_INPUT);
   DBParam(IDBParams* oParams, SQLSMALLINT nParamNum, const String& sParamName,
         SQLPOINTER pPointer, DataType eDataType, SQLULEN nDataSize, SQLSMALLINT nDecimalDigits,
         bool bNotNull = false, SQLSMALLINT nParamType = SQL_PARAM_INPUT);
   ~DBParam();
public://Events
   void SetOnParamPutData(ParamData pValue);
   void SetOnParamGetData(ParamData pValue);
};
//---------------------------------------------------------------------------
typedef std::vector<DBParam*> DBParamList;
class DBParams : public DBObject, public virtual IDBParams
{
private://Variables
   friend class DBConnection;
   friend class DBStmt;
   friend class DBRecordset;
   friend class DBParam;
protected:
   SQLRETURN    nRetcode;
   DBStmt*      oStmt;
   DBParamList* oListParams;
   bool         bAutoGenerate;
private://Private Methods
   SQLRETURN CreateListParams(SQLSMALLINT nParamsCount);
public://Properties
   IDBStmt* getStmt();
public://Methods
   DBParams(IDBStmt* oStmt, bool bAutoGenerate = true, SQLSMALLINT nParamsCount = 0);
   ~DBParams();
   int Count();
   IDBParam* ParamByIndex(int nIndex);
   IDBParam* ParamByName(const String& sName);
   IDBParam* Add(IDBParam* oParam);
   IDBParam* Insert(int nIndex, IDBParam* oParam);
   IDBParam* Remove(int nIndex);
   SQLRETURN BindParams();
   SQLRETURN UnbindParams();
};
//---------------------------------------------------------------------------
class DBField : public DBBind, public virtual IDBField
{
private://Variables
   friend class DBStmt;
   friend class DBRecordset;
   friend class DBFields;
protected://Events
   FieldData OnFieldPutData;
   FieldData OnFieldGetData;
protected:
   DBFields*    oFields;
   bool         bAutoIncrement;
   bool         bIsPkKey;
   bool         bIsVirtual;
   bool         bReadOnly;
private://Private methods
   SQLRETURN DescribeColumn();
   SQLRETURN Bind();
   SQLRETURN PutData();
   SQLRETURN GetData();
public://Properties
   IDBFields* getFields();
   SQLSMALLINT getFieldNum();
   const String& getFieldName();
   void setFieldName(const String &Value);
   bool getAutoIncrement();
   SQLRETURN setAutoIncrement(const bool Value);
   bool getIsPkKey();
   SQLRETURN setIsPkKey(const bool Value);
   bool getIsVirtual();
   SQLRETURN setIsVirtual(const bool Value);
   bool getReadOnly();
private:
   DBField(IDBFields* oFields, SQLSMALLINT nFieldNum);
public://Methods
   DBField(IDBFields* oFields);
   DBField(IDBFields* oFields, SQLSMALLINT nFieldNum, const String& sFieldName, 
         DataType eDataType, SQLULEN nDataSize, SQLSMALLINT nDecimalDigits = 0, 
         bool bNotNull = false, bool bIsPkKey = false, bool bIsVirtual = false);
   DBField(IDBFields* oFields, SQLSMALLINT nFieldNum, const String& sFieldName,
         SQLPOINTER pPointer, DataType eDataType, SQLULEN nDataSize, SQLSMALLINT nDecimalDigits = 0,
         bool bNotNull = false, bool bIsPkKey = false, bool bIsVirtual = false);
   ~DBField();
public://Events
   void SetOnFieldPutData(FieldData pValue);
   void SetOnFieldGetData(FieldData pValue);
};
//---------------------------------------------------------------------------
typedef std::vector<DBField*> DBFieldList;
class DBFields : public DBObject, public virtual IDBFields
{
private://Variables
   friend class DBStmt;
   friend class DBRecordset;
   friend class DBField;
protected:
   SQLRETURN    nRetcode;
   DBRecordset* oRecordset;
   DBFieldList* oListFields;
   bool         bAutoGenerate;
private://Private Methods
   SQLRETURN CreateListFields(SQLSMALLINT nFieldsCount);
public://Properties
   IDBRecordset* getRecordset();
public://Methods
   DBFields(IDBRecordset* oRecordset, bool bAutoGenerate = true, SQLSMALLINT nFieldsCount = 0);
   ~DBFields();
   int Count();
   IDBField* FieldByIndex(int nIndex);
   IDBField* FieldByName(const String& sName);
   IDBField* Add(IDBField* oField);
   IDBField* Insert(int nIndex, IDBField* oField);
   IDBField* Remove(int nIndex);
   SQLRETURN BindFields();
   SQLRETURN UnbindFields();
};
//---------------------------------------------------------------------------
}; //end namespace sfg
#endif
