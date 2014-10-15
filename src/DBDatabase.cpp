#include <map>
#include <cassert>
#include <cfloat>
#include <algorithm>
#include "DBDatabase.hpp"
//---------------------------------------------------------------------------
namespace sfg {
//---------------------------------------------------------------------------
static DBEnvironment oDefaultEnv;
IDBEnvironment* GetDefaultEnvironment(void) {
   return &oDefaultEnv;
}
IDBEnvironment* GetDBEnvironment1(void) {
   return new DBEnvironment();
}
// ***************** Implementation code for DBEnvironment ****************
//---------------------------------------------------------------------------
DBEnvironment::DBEnvironment() : 
      DBObject(), IDBEnvironment()
{
   this->hEnv = SQL_NULL_HENV;
}
//---------------------------------------------------------------------------
DBEnvironment::~DBEnvironment()
{
   ReleaseHandles();
}
//---------------------------------------------------------------------------
void DBEnvironment::AddRef() {
   if (this == &oDefaultEnv) //Environment default not have reference count
      return;
   DBObject::AddRef();
}
//---------------------------------------------------------------------------
void DBEnvironment::Release() {
   if (this == &oDefaultEnv) //Environment default not have reference count
      return;
   DBObject::Release();
}
//---------------------------------------------------------------------------
SQLRETURN DBEnvironment::ReleaseHandles()
{
   // free environment handle
   if (hEnv != SQL_NULL_HENV) {
      nRetcode = SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
      hEnv = SQL_NULL_HENV;
   }
   return nRetcode;
}
//---------------------------------------------------------------------------
SQLRETURN DBEnvironment::Init(bool bPool)
{
   nRetcode = SQL_SUCCESS;
   if (hEnv != SQL_NULL_HENV)
      return nRetcode;

   // Before we allocate our first environment handle, try to turn on connection pooling
   // Note we are not checking error codes here, we just try to turn it on,
   // and if the driver does not support pooling there is nothing we can do.

   if (bPool) {
      // pool across the environment handle
      nRetcode = SQLSetEnvAttr(SQL_NULL_HENV, SQL_ATTR_CONNECTION_POOLING, (SQLPOINTER)SQL_CP_ONE_PER_DRIVER, SQL_IS_INTEGER);
      if (!RC_SUCCESS(nRetcode)) {
         hEnv = SQL_NULL_HENV;
         ExceptionByHandle(SQL_HANDLE_ENV, SQL_NULL_HENV,
               _T("DBEnvironment::Init"),
               _T("Unable to pool across the environment handle!"));
         return nRetcode;
      }
   }

   // Allocate environment handle
   nRetcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
   if (!RC_SUCCESS(nRetcode)) {
      hEnv = SQL_NULL_HENV;
      ExceptionByHandle(SQL_HANDLE_ENV, SQL_NULL_HENV,
            _T("DBEnvironment::Init"),
            _T("Unable to allocate SQL environment handle!"));
      return nRetcode;
   }

   // set the DB behavior version. Not sure why this is required, but
   // connection pooling does not seem to work without it
   nRetcode = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER);
   if (!RC_SUCCESS(nRetcode)) {

      SgOStringStream oErrMsg;
      oErrMsg << _T("Unable to set DB version behavior!");

      nRetcode = SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
      if (!RC_SUCCESS(nRetcode))
         oErrMsg << _T("  Unable to free environment handle!");

      hEnv = SQL_NULL_HENV;

      oErrMsg << std::ends;

      ExceptionByHandle(SQL_HANDLE_ENV, SQL_NULL_HENV,
            _T("DBEnvironment::Init"),
            oErrMsg.str());
      return nRetcode;
   }

   //set the matching condition for using an existing connection in the pool
   //use relaxed matching, i.e. most liberal rules for matching existing connection
   nRetcode = SQLSetEnvAttr(hEnv, SQL_ATTR_CP_MATCH, (SQLPOINTER) SQL_CP_RELAXED_MATCH, SQL_IS_INTEGER);
   if (!RC_SUCCESS(nRetcode)) {

      SgOStringStream oErrMsg;
      oErrMsg << _T("Unable to set matching condition for environment!");

      nRetcode = SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
      if (!RC_SUCCESS(nRetcode))
         oErrMsg << _T("  Unable to free environment handle!");

      hEnv = SQL_NULL_HENV;

      oErrMsg << std::ends;

      ExceptionByHandle(SQL_HANDLE_ENV, SQL_NULL_HENV,
            _T("DBEnvironment::Init"),
            oErrMsg.str());
      return nRetcode;
   }
   return nRetcode;
}
//---------------------------------------------------------------------------
HENV DBEnvironment::getHENV()
{
   if (hEnv == SQL_NULL_HENV)
      ExceptionByHandle(SQL_HANDLE_ENV, SQL_NULL_HENV,
            _T("DBEnvironment::getHENV"),
            _T("Environment is not allocated!"));
   return hEnv;
}
//---------------------------------------------------------------------------
static DBConnection oDefaultConn(&oDefaultEnv);
IDBConnection* GetDefaultConnection(void) {
   return &oDefaultConn;
}
IDBConnection* GetDBConnection1(
      IDBEnvironment *oEnvironment) {
   return new DBConnection(oEnvironment);
}
IDBConnection* GetDBConnection2(
      IDBEnvironment *oEnvironment, const String& sStrConn) {
   return new DBConnection(oEnvironment,sStrConn);
}
//---------------------------------------------------------------------------
DBConnection::DBConnection(IDBEnvironment* oEnvironment) :
      DBObject(), IDBConnection()
{
   InternalInit(oEnvironment,_T(""));
}
//---------------------------------------------------------------------------
DBConnection::DBConnection(IDBEnvironment* oEnvironment, const String& sStrConn) :
      DBObject(), IDBConnection()
{
   InternalInit(oEnvironment,sStrConn);
}
//---------------------------------------------------------------------------
void DBConnection::InternalInit(IDBEnvironment* oEnvironment, const String& sStrConn)
{
   this->nRetcode = SQL_SUCCESS;
   this->oEnvironment = dynamic_cast<DBEnvironment*>(oEnvironment);
   this->hDbc = SQL_NULL_HDBC;
   this->nStmtCount = 0;
   this->sStrConn = sStrConn;
   this->bAutoCommit = true;
   this->eConnState = CONN_UNALLOCATED;
   this->eDbmsType = DB_UNKNOWN;
   this->sQuoteChar = _T("");
   this->sDriverName = _T("");
   this->sDriverVer = _T("");
   this->sDBMSName = _T("");
   this->sDBMSVer = _T("");
   this->sDatabase = _T("");
   this->sUser = _T("");
   this->sCatalog = _T("");
   this->sSchema = _T("");
   this->nMaxCatalogName = MAX_CATALOG_NAME;
   this->nMaxSchemaName = MAX_SCHEMA_NAME;
   this->nMaxTableName = MAX_TABLE_NAME;
   this->nMaxColumnName = MAX_COLUMN_NAME;
   this->nScrollOptions = SQL_SO_FORWARD_ONLY;
}
//---------------------------------------------------------------------------
DBConnection::~DBConnection()
{
   ReleaseHandles();
}
//---------------------------------------------------------------------------
void DBConnection::AddRef() {
   if (this == &oDefaultConn) //Connection default not have reference count
      return;
   DBObject::AddRef();
}
//---------------------------------------------------------------------------
void DBConnection::Release() {
   if (this == &oDefaultConn) //Connection default not have reference count
      return;
   DBObject::Release();
}
//---------------------------------------------------------------------------
SQLHDBC DBConnection::getHDBC()
{
   if (hDbc == SQL_NULL_HDBC)
      ExceptionByHandle(SQL_HANDLE_DBC, SQL_NULL_HDBC,
            _T("DBConnection::getHDBC"),
            _T("Connection handle is not allocated!"));
   return hDbc;
}
//---------------------------------------------------------------------------
IDBEnvironment* DBConnection::getEnvironment() {
  return oEnvironment;
}
//---------------------------------------------------------------------------
const String& DBConnection::getStrConn() {
  return sStrConn;
}
//---------------------------------------------------------------------------
const String& DBConnection::getDBMSName() {
  return sDBMSName;
}
//---------------------------------------------------------------------------
DbmsType DBConnection::getDBMSType() {
   return eDbmsType;
}
//---------------------------------------------------------------------------
const String& DBConnection::getDBMSVersion() {
   return sDBMSVer;
}
//---------------------------------------------------------------------------
const String& DBConnection::getQuoteChar(void) {
   return sQuoteChar;
}
//---------------------------------------------------------------------------
const String& DBConnection::getDriverName() {
   return sDriverName;
}
//---------------------------------------------------------------------------
const String& DBConnection::getDriverVersion() {
   return sDriverVer;
}
//---------------------------------------------------------------------------
const String& DBConnection::getDatabaseName() {
   return sDatabase;
}
//---------------------------------------------------------------------------
const String& DBConnection::getUserName() {
   return sUser;
}
//---------------------------------------------------------------------------
SQLUSMALLINT DBConnection::getMaxCatalogName() {
   return nMaxCatalogName;
}
//---------------------------------------------------------------------------
SQLUSMALLINT DBConnection::getMaxSchemaName() {
   return nMaxSchemaName;
}
//---------------------------------------------------------------------------
SQLUSMALLINT DBConnection::getMaxTableName() {
   return nMaxTableName;
}
//---------------------------------------------------------------------------
SQLUSMALLINT DBConnection::getMaxColumnName() {
   return nMaxColumnName;
}
//---------------------------------------------------------------------------
SQLUINTEGER DBConnection::getScrollOptions() {
   return nScrollOptions;
}
//---------------------------------------------------------------------------
const String& DBConnection::getCatalogName() {
   return sCatalog;
}
//---------------------------------------------------------------------------
const String& DBConnection::getSchemaName() {
   return sSchema;
}
//---------------------------------------------------------------------------
int DBConnection::getStmtCount() {
   return nStmtCount;
}
//---------------------------------------------------------------------------
SQLRETURN DBConnection::setStrConn(const String &Value)
{
   ConnState eOldState = this->eConnState;
   nRetcode = SQL_SUCCESS;
   if (this->sStrConn != Value && this->eConnState == CONNECTED) {
      nRetcode = Disconnect();
      if (!RC_SUCCESS(nRetcode))
         return nRetcode;
   }
   this->sStrConn = Value;

   if (eOldState == CONNECTED) {
      nRetcode = InternalConnect((SQLTCHAR*)sStrConn.c_str(), false);
   }
   return nRetcode;
}
//---------------------------------------------------------------------------
SQLRETURN DBConnection::CreateHandles(bool bPool)
{
   if (eConnState == CONN_ALLOCATED)
      return SQL_SUCCESS;

   nRetcode = oEnvironment->Init(bPool);
   if (!RC_SUCCESS(nRetcode))
      return nRetcode;

   nRetcode = SQLAllocHandle(SQL_HANDLE_DBC, oEnvironment->getHENV(), &hDbc);
   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_DBC, SQL_NULL_HDBC,
            _T("DBConnection::CreateHandles"),
            _T("Unable to allocate SQL connection handle!"));
      return nRetcode;
   }
   eConnState = CONN_ALLOCATED;
   return nRetcode;
}
//---------------------------------------------------------------------------
SQLRETURN DBConnection::InternalConnect (SQLTCHAR *cConnStrIn, bool bPrompt)
{
   assert(hDbc != SQL_NULL_HDBC);
   nRetcode = SQL_SUCCESS;

   SQLTCHAR cConnStrOut[2048] = {0};  // Microsoft DB requires at least 1024
                            // bytes here, otherwise will overwrite
                            // memory
   SQLSMALLINT nConnStrIn;
   SQLSMALLINT nConnStrOut = 0;
   SQLUSMALLINT nDriverCompletion;
   bool bUseConnectStr;

   if (eConnState == CONNECTED)
      return nRetcode;

   // needed statics
   HWND hAppHwnd = NULL;
#if  defined(SG_WINDOWS)
   hAppHwnd = GetDesktopWindow();
#endif

   // To prompt for connection, use SQL_DRIVER_COMPLETE
   if (bPrompt)
      nDriverCompletion = SQL_DRIVER_COMPLETE;
   else
      nDriverCompletion = SQL_DRIVER_NOPROMPT;

    // Prompt for connection
   bUseConnectStr = false;

#if defined(_UNICODE)
   nConnStrIn = wcslen(cConnStrIn);
#else
   nConnStrIn = strlen((char*)cConnStrIn) * sizeof(SQLTCHAR);
#endif

   if (nConnStrIn > 0)
      bUseConnectStr = true;

   if (bUseConnectStr)
      nRetcode = SQLDriverConnect(hDbc, hAppHwnd, (SQLTCHAR*)cConnStrIn,
                        nConnStrIn, (SQLTCHAR*)cConnStrOut, sizeof(cConnStrOut),
                        &nConnStrOut, nDriverCompletion);
   else
      nRetcode = SQLDriverConnect(hDbc, hAppHwnd, (SQLTCHAR*)cConnStrIn,
                        0, (SQLTCHAR*)cConnStrOut, sizeof(cConnStrOut),
                        &nConnStrOut, nDriverCompletion);


    if (!RC_SUCCESS(nRetcode)) {
      String sErrMsg;
      sErrMsg.reserve(256);
      sErrMsg += _T("Unable to connect to database using string connection: ");
      sErrMsg += (TCHAR*) cConnStrIn;
      sErrMsg += _T("!");
      if (bPrompt)
      {
         sErrMsg += _T("\n");
         sErrMsg += _T("Note that in order to browse for a connection, connection pooling must be turned off.\n");
         sErrMsg += _T("To do this you must #define DTL_NO_POOL or manually initialize the environment with no pooling via \n");
         sErrMsg += _T("DBConnection.GetDefaultEnvironment().init(false);  \nbefore you connect for the first time.\n");
         sErrMsg += _T("(If you prompt for a connection the first time you connect then pooling is automatically turned off.)");
      }
      ExceptionByHandle(SQL_HANDLE_DBC, hDbc,
            _T("DBConnection::InternalConnect"),
            sErrMsg);
      return nRetcode;
   }

   nRetcode = ComputeDBInfo();
   if (!RC_SUCCESS(nRetcode))
      return nRetcode;

   // update state variables
   // exception safety maintained as state variables not updated until
   // connection succeeds end
   // only the SgString assignment can throw afterwards (so it's done first)
   this->sStrConn = (TCHAR*) cConnStrOut;
   this->eConnState = CONNECTED;
   this->bAutoCommit = true;
   return nRetcode;
}
//---------------------------------------------------------------------------
SQLRETURN DBConnection::Connect()
{
   return Connect(sStrConn);
}
//---------------------------------------------------------------------------
SQLRETURN DBConnection::Connect(const String& sStrConn, bool bPrompt)
{
   nRetcode = SQL_SUCCESS;
   if (this->sStrConn == sStrConn && this->eConnState == CONNECTED)
      return nRetcode;

   this->sStrConn = sStrConn;
   nRetcode = CreateHandles(!bPrompt);
   if (!RC_SUCCESS(nRetcode))
      return nRetcode;

   nRetcode = InternalConnect((SQLTCHAR*)this->sStrConn.c_str(), bPrompt);
   return nRetcode;
}
//---------------------------------------------------------------------------
bool DBConnection::IsConnected()
{
   return (eConnState == CONNECTED);
}
//---------------------------------------------------------------------------
SQLRETURN DBConnection::IsConnected(bool *bValue)
{
   *bValue = (bool)(eConnState == CONNECTED);
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN DBConnection::ComputeDBInfo()
{
   assert(hDbc != SQL_NULL_HDBC);
   nRetcode = SQL_SUCCESS;

   SQLTCHAR cDBInfo[255];
   SWORD nDummy;
   SDWORD nLDummy;

   memset(&cDBInfo,0,sizeof(cDBInfo));
   nRetcode = SQLGetInfo(hDbc, SQL_DBMS_NAME, &cDBInfo, sizeof(cDBInfo), &nDummy);
   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_DBC, hDbc,
            _T("DBConnection::ComputeDBInfo"),
            _T("Unable to get name of DBMS!"));
      return nRetcode;
   }
   sDBMSName = (TCHAR*)cDBInfo;

   int nFound = -1;

   if (sDBMSName == _T("Microsoft SQL Server"))
      eDbmsType = DB_SQL_SERVER;
   else if (sDBMSName == _T("MySQL"))
      eDbmsType = DB_MYSQL;
   else if (sDBMSName == _T("Oracle"))
      eDbmsType = DB_ORACLE;
   else if (sDBMSName == _T("OpenRDA"))
      eDbmsType = DB_OPENRDA;
   else if (sDBMSName == _T("Access"))
      eDbmsType = DB_ACCESS;
   else if (sDBMSName == _T("EXCEL"))
      eDbmsType = DB_EXCEL;
   else
      eDbmsType = DB_UNKNOWN;

   memset(&cDBInfo,0,sizeof(cDBInfo));
   nRetcode = SQLGetInfo(hDbc, SQL_DBMS_VER, &cDBInfo, sizeof(cDBInfo), &nDummy);
   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_DBC, hDbc,
            _T("DBConnection::ComputeDBInfo"),
            _T("Unable to get version of DBMS!"));
      return nRetcode;
   }
   sDBMSVer = (TCHAR*)cDBInfo;

   memset(&cDBInfo,0,sizeof(cDBInfo));
   nRetcode = SQLGetInfo(hDbc, SQL_IDENTIFIER_QUOTE_CHAR, &cDBInfo, sizeof(cDBInfo), &nDummy);
   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_DBC, hDbc,
            _T("DBConnection::ComputeDBInfo"),
            _T("Unable to get SQL quote caracter!"));
      return nRetcode;
   }
   sQuoteChar = (TCHAR*)cDBInfo;

   memset(&cDBInfo,0,sizeof(cDBInfo));
   nRetcode = SQLGetInfo(hDbc, SQL_DRIVER_NAME, &cDBInfo, sizeof(cDBInfo), &nDummy);
   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_DBC, hDbc,
            _T("DBConnection::ComputeDBInfo"),
            _T("Unable to get name of driver!"));
      return nRetcode;
   }
   sDriverName = (TCHAR*)cDBInfo;

   memset(&cDBInfo,0,sizeof(cDBInfo));
   nRetcode = SQLGetInfo(hDbc, SQL_DRIVER_VER, &cDBInfo, sizeof(cDBInfo), &nDummy);
   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_DBC,  hDbc,
            _T("DBConnection::ComputeDBInfo"),
            _T("Unable to get version of driver!"));
      return nRetcode;
   }
   sDriverVer = (TCHAR*)cDBInfo;

   memset(&cDBInfo,0,sizeof(cDBInfo));
   nRetcode = SQLGetInfo(hDbc, SQL_DATABASE_NAME, &cDBInfo, sizeof(cDBInfo), &nDummy);
   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_DBC,  hDbc,
            _T("DBConnection::ComputeDBInfo"),
            _T("Unable to get database name!"));
      return nRetcode;
   }
   sDatabase = (TCHAR*)cDBInfo;

   memset(&cDBInfo,0,sizeof(cDBInfo));
   nRetcode = SQLGetConnectAttr(hDbc, SQL_ATTR_CURRENT_CATALOG, &cDBInfo, sizeof(cDBInfo), &nLDummy);
   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_DBC,  hDbc,
            _T("DBConnection::ComputeDBInfo"),
            _T("Unable to get catalog name!"));
      return nRetcode;
   }
   sCatalog = (TCHAR*)cDBInfo;

   /*memset(&cDBInfo,0,sizeof(cDBInfo));
   nRetcode = SQLGetConnectAttr(hDbc, SQL_ATTR_CURRENT_SCHEMA, &cDBInfo, sizeof(cDBInfo), &nLDummy);
   if (!RC_SUCCESS(nRetcode))
      ExceptionByHandle(SQL_HANDLE_DBC, _T("DBConnection::ComputeDBInfo"),
            _T("Unable to get schema name!"),
            oEnvironment, this, NULL);
   sSchema = cDBInfo;*/

   memset(&cDBInfo,0,sizeof(cDBInfo));
   nRetcode = SQLGetInfo(hDbc, SQL_USER_NAME, &cDBInfo, sizeof(cDBInfo), &nDummy);
   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_DBC,  hDbc,
            _T("DBConnection::ComputeDBInfo"),
            _T("Unable to get database user name!"));
      return nRetcode;
   }
   sUser = (TCHAR*)cDBInfo;

   nRetcode = SQLGetInfo(hDbc, SQL_MAX_CATALOG_NAME_LEN, &nMaxCatalogName, sizeof(nMaxCatalogName), &nDummy);
   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_DBC,  hDbc,
            _T("DBConnection::ComputeDBInfo"),
            _T("Unable to get max schema name!"));
      return nRetcode;
   }
   if (nMaxCatalogName == 0)
      nMaxCatalogName = MAX_CATALOG_NAME;

   nRetcode = SQLGetInfo(hDbc, SQL_MAX_SCHEMA_NAME_LEN, &nMaxSchemaName, sizeof(nMaxSchemaName), &nDummy);
   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_DBC,  hDbc,
            _T("DBConnection::ComputeDBInfo"),
            _T("Unable to get max schema name!"));
      return nRetcode;
   }
   if (nMaxSchemaName == 0)
      nMaxSchemaName = MAX_SCHEMA_NAME;

   nRetcode = SQLGetInfo(hDbc, SQL_MAX_TABLE_NAME_LEN, &nMaxTableName, sizeof(nMaxTableName), &nDummy);
   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_DBC,  hDbc,
            _T("DBConnection::ComputeDBInfo"),
            _T("Unable to get max table name!"));
      return nRetcode;
   }
   if (nMaxTableName == 0)
      nMaxTableName = MAX_TABLE_NAME;

   nRetcode = SQLGetInfo(hDbc, SQL_MAX_COLUMN_NAME_LEN, &nMaxColumnName, sizeof(nMaxColumnName), &nDummy);
   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_DBC,  hDbc,
            _T("DBConnection::ComputeDBInfo"),
            _T("Unable to get max column name!"));
      return nRetcode;
   }
   if (nMaxColumnName == 0)
      nMaxColumnName = MAX_COLUMN_NAME;

   nRetcode = SQLGetInfo(hDbc, SQL_SCROLL_OPTIONS, &nScrollOptions, sizeof(nScrollOptions), &nDummy);
   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_DBC,  hDbc,
            _T("DBConnection::ComputeDBInfo"),
            _T("Unable to get cursor scroll options!"));
      return nRetcode;
   }
   if (nScrollOptions == 0)
      nScrollOptions = SQL_SO_FORWARD_ONLY;
   return nRetcode;
}
//---------------------------------------------------------------------------
SQLRETURN DBConnection::ReleaseHandles()
{
   nRetcode = SQL_SUCCESS;
   if(eConnState == CONN_UNALLOCATED) {
      return nRetcode;
   }

   //Sempre que der ReleaseHandles na conexão as alterações serão descartadas
   if (!bAutoCommit) {
      nRetcode = Roolback();
      if (!RC_SUCCESS(nRetcode))
         return nRetcode;
   }

   switch (eConnState) {
      case CONNECTED:
         nRetcode = Disconnect();
      case CONN_ALLOCATED:
         nRetcode = SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
         if (!RC_SUCCESS(nRetcode)) {

            eConnState = CONN_UNALLOCATED;
            hDbc = SQL_NULL_HDBC;

            ExceptionByHandle(SQL_HANDLE_DBC, hDbc,
                  _T("DBConnection::ReleaseHandles"),
                  _T("Unable to free connection handle!"));
         }
         break;
      case CONN_UNALLOCATED: ; // do nothing
      default: ; // do nothing
   }
   if (!RC_SUCCESS(nRetcode))
      return nRetcode;

   // first ReleaseHandles ownership to prevent double destruction
   eConnState = CONN_UNALLOCATED;
   hDbc = SQL_NULL_HDBC;
   return nRetcode;
}
//---------------------------------------------------------------------------
SQLRETURN DBConnection::Disconnect()
{
   eConnState = CONN_ALLOCATED;
   nRetcode = SQLDisconnect(hDbc);
   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_DBC, hDbc,
            _T("DBConnection::Disconnect"),
            _T("Unable to disconnect using DB handle!"));
   }
   return nRetcode;
}
//---------------------------------------------------------------------------
SQLRETURN DBConnection::ToggleAutoCommit()
{
   //Se está no modo auto commit deve alterar o mesmo
   SQLINTEGER iAutoCommit = (bAutoCommit) ? SQL_AUTOCOMMIT_OFF : SQL_AUTOCOMMIT_ON;
   nRetcode = SQLSetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)iAutoCommit, (SQLINTEGER)0);
   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_DBC, hDbc,
            _T("DBConnection::ToggleAutoCommit"),
            _T("Unable to set attribute auto commit for connection!"));
      return nRetcode;
   }
   bAutoCommit = !bAutoCommit;
   return nRetcode;
}
//---------------------------------------------------------------------------
SQLRETURN DBConnection::BeginTrans()
{
   //Não permite abrir duas transações para uma mesma conexão
   if (!bAutoCommit) {
      SetExceptionInfo(_T("DBConnection::BeginTrans"),
         _T("Transaction is already open for this connection!"),
         _T("25000"),_T("Illegal operation while in a local transaction."),0);
      return SQL_ERROR;
   }
   return ToggleAutoCommit();
}
//---------------------------------------------------------------------------
SQLRETURN DBConnection::Commit()
{
   nRetcode = SQL_SUCCESS;
   //Não está no modo transacional, apenas retorna
   if (bAutoCommit)
      return nRetcode;

   nRetcode = SQLEndTran(SQL_HANDLE_DBC, hDbc, SQL_COMMIT);
   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_DBC, hDbc,
            _T("DBConnection::Commit"),
            _T("Unable to commit transaction for connection!"));
      return nRetcode;
   }
   return ToggleAutoCommit();
}
//---------------------------------------------------------------------------
SQLRETURN DBConnection::Roolback()
{
   nRetcode = SQL_SUCCESS;
   //Não está no modo transacional, apenas retorna
   if (bAutoCommit)
      return nRetcode;

   nRetcode = SQLEndTran(SQL_HANDLE_DBC, hDbc, SQL_ROLLBACK);
   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_DBC, hDbc,
            _T("DBConnection::Roolback"),
            _T("Unable to roolback transaction for connection!"));
      return nRetcode;
   }
   return ToggleAutoCommit();
}
//---------------------------------------------------------------------------
DBAttribute::DBAttribute()
   : nAttribute(0), pValue(NULL), nLength(0)
{}
//---------------------------------------------------------------------------
DBAttribute::DBAttribute(SQLINTEGER nAttribute, SQLPOINTER pValue, SQLINTEGER nLength)
{
   this->nAttribute = nAttribute;
   this->pValue = pValue;
   this->nLength = nLength;
};
//---------------------------------------------------------------------------
DBAttribute::~DBAttribute()
{}
//---------------------------------------------------------------------------
IDBStmt* GetDBStmt1(
      IDBConnection* oConnection) {
   return new DBStmt(oConnection);
}
IDBStmt* GetDBStmt2(
      IDBConnection* oConnection, const String& sSQLCmd, bool bPrepare) {
   return new DBStmt(oConnection,sSQLCmd,bPrepare);
}
//---------------------------------------------------------------------------
// ************** Implementation code for DBStmt *****************
DBStmt::DBStmt(IDBConnection* oConnection) :
      DBObject(), IDBStmt()
{
   InternalInit(oConnection, _T(""), false);
}
//---------------------------------------------------------------------------
DBStmt::DBStmt(IDBConnection* oConnection, const String& sSQLCmd, bool bPrepare) :
      DBObject(), IDBStmt()
{
   InternalInit(oConnection, sSQLCmd, bPrepare);
}
//---------------------------------------------------------------------------
void DBStmt::InternalInit(IDBConnection* oConnection, const String& sSQLCmd, bool bPrepare)
{
   this->nRetcode = SQL_SUCCESS;
   this->oConnection = dynamic_cast<DBConnection*>(oConnection);
   this->hStmt = SQL_NULL_HSTMT;
   this->eStmtState = STMT_UNALLOCATED;
   this->sSQLCmd = sSQLCmd;
   this->bPrepare = bPrepare;
   this->oParams = NULL;
   this->oConnection->nStmtCount++;
   this->oAttrs = new DBAttrList();
   this->AfterPrepare = NULL;
   this->AfterExecute = NULL;
   if (bPrepare)
      Initialize(bPrepare);
}
//---------------------------------------------------------------------------
DBStmt::~DBStmt()
{
   ReleaseHandles();
   ClearStmtAttrs();
   delete oAttrs;
   this->oConnection->nStmtCount--;
}
//---------------------------------------------------------------------------
SQLHSTMT DBStmt::getHSTMT() {
   if (hStmt == SQL_NULL_HDBC)
      ExceptionByHandle(SQL_HANDLE_STMT, SQL_NULL_HDBC,
            _T("DBStmt::getHSTMT"),
            _T("Statement handle is not allocated!"));
   return hStmt;
}
//---------------------------------------------------------------------------
const String& DBStmt::getSQLCmd() {
   return sSQLCmd;
}
//---------------------------------------------------------------------------
void DBStmt::setSQLCmd(const String &Value) {
   this->sSQLCmd = Value;
}
//---------------------------------------------------------------------------
IDBConnection* DBStmt::getConnection() {
   return oConnection;
}
//---------------------------------------------------------------------------
IDBParams* DBStmt::getParams() {
   return oParams;
}
//---------------------------------------------------------------------------
void DBStmt::SetAfterPrepare(StmtNotify pValue)
{
   this->AfterPrepare = pValue;
}
//---------------------------------------------------------------------------
void DBStmt::SetAfterExecute(StmtNotify pValue)
{
   this->AfterExecute = pValue;
}
//---------------------------------------------------------------------------
SQLRETURN DBStmt::CreateHandles()
{
   nRetcode = SQLAllocHandle(SQL_HANDLE_STMT, oConnection->getHDBC(), &hStmt);
   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_STMT, SQL_NULL_HDBC,
            _T("DBStmt::CreateHandles"),
            _T("Unable to allocate statement handle!"));
      return nRetcode;
   }
   eStmtState = STMT_ALLOCATED;
   return nRetcode;
}
//---------------------------------------------------------------------------
SQLRETURN DBStmt::ReleaseHandles()
{
   nRetcode = SQL_SUCCESS;
   if (oParams != NULL) {
      nRetcode = oParams->UnbindParams();
      if (!RC_SUCCESS(nRetcode))
         return nRetcode;
      if (oParams->bAutoGenerate)
         delete oParams;
   }

   if(eStmtState == STMT_UNALLOCATED)
      return nRetcode;

   switch (eStmtState) {
      case STMT_EXECUTED:
      case STMT_PREPARED:
      case STMT_ALLOCATED:
         nRetcode = SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
         if (!RC_SUCCESS(nRetcode)) {

            eStmtState = STMT_UNALLOCATED;
            hStmt = SQL_NULL_HSTMT;

            ExceptionByHandle(SQL_HANDLE_STMT, hStmt,
                  _T("DBStmt::ReleaseHandles"),
                  _T("Unable to free statement handle!"));
         }
         break;
      case STMT_UNALLOCATED: ; // do nothing
      default: ; // do nothing
   }
   if (!RC_SUCCESS(nRetcode))
      return nRetcode;

   eStmtState = STMT_UNALLOCATED;
   hStmt = SQL_NULL_HSTMT;
   return nRetcode;
}
//---------------------------------------------------------------------------
bool DBStmt::IsReady()
{
   return (eStmtState == STMT_PREPARED || eStmtState == STMT_EXECUTED);
}
//---------------------------------------------------------------------------
bool DBStmt::IsAllocated()
{
   return (eStmtState != STMT_UNALLOCATED);
}
//---------------------------------------------------------------------------
bool DBStmt::IsExecuted()
{
   return eStmtState == STMT_EXECUTED;
}
//---------------------------------------------------------------------------
SQLRETURN DBStmt::Initialize(bool bPrepare)
{
   nRetcode = SQL_SUCCESS;
   if (IsReady())
      return nRetcode;

   nRetcode = CreateHandles();
   if (!RC_SUCCESS(nRetcode))
      return nRetcode;

   nRetcode = LoadAttributes();
   if (!RC_SUCCESS(nRetcode))
      return nRetcode;

   if (bPrepare)
      nRetcode = Prepare();
   return nRetcode;
}
//---------------------------------------------------------------------------
SQLRETURN DBStmt::Prepare()
{
   nRetcode = SQL_SUCCESS;
   if (!IsAllocated()) {
      nRetcode = Initialize(false);
      if (!RC_SUCCESS(nRetcode))
         return nRetcode;
   }

   assert(hStmt != SQL_NULL_HSTMT);

   if (IsReady())
      return nRetcode;

   nRetcode = SQLPrepare(hStmt, (SQLTCHAR*)sSQLCmd.c_str(), SQL_NTS);
   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_STMT, hStmt,
            _T("DBStmt::Prepare"),
            _T("Unable to prepare statement SQL!"));
      return nRetcode;
   }

   //Deve verificar se deve setar os parametros da Query;
   SQLSMALLINT nParamsCount = 0;
   nRetcode = SQLNumParams(hStmt, &nParamsCount);
   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_STMT, hStmt,
            _T("DBStmt::Prepare"),
            _T("Unable get number of parameters for statement SQL!"));
      return nRetcode;
   }
   //Se não tem a listagem de parametros deve então criar uma
   if (nParamsCount > 0) {
      if (oParams == NULL) {
         ClearException();
         oParams = new DBParams(this,true,nParamsCount);
         oParams->bAutoGenerate = true;
         if (GetLastDBException()->getErrMsg().length() > 0)
            return SQL_ERROR;
      } else {
         //Se a listagem de parametros já está preenchida e é diferente da
         //quantidade de parametros esperada pela Query causa uma excessão
         if (oParams->oListParams->size() != nParamsCount) {
            SetExceptionInfo(_T("DBStmt::Prepare"),
                  _T("Number of parameters list diferente of number of parameters for statement SQL!"),
                  _T("07001"),_T("Wrong number of parameters"),0);
            return SQL_ERROR;
         }
      }
   }
   eStmtState = STMT_PREPARED;
   if (AfterPrepare != NULL)
      AfterPrepare(this);

   return nRetcode;
}
//---------------------------------------------------------------------------
SQLRETURN DBStmt::Execute()
{
   nRetcode = SQL_SUCCESS;
   if (!IsAllocated()) {
      nRetcode = Initialize((bPrepare) ? false : true);
      if (!RC_SUCCESS(nRetcode))
         return nRetcode;
   }

   assert(hStmt != SQL_NULL_HSTMT);

   //if (IsExecuted())
   //   return;

   if (oParams != NULL) {
      if ((oParams->Count() > 0) && (!oParams->ParamByIndex(0)->IsBind())) {
         nRetcode = oParams->BindParams();
         if (!RC_SUCCESS(nRetcode))
            return nRetcode;
      }
   }

   nRetcode = SQLExecute(hStmt);
   //Se precisa de dados para os parâmetros deve neste momento
   //invocar os eventos para o usuário informar os dados necessários
   if (nRetcode == SQL_NEED_DATA) {
      nRetcode = PutParamData();
      if (!RC_SUCCESS(nRetcode))
         return nRetcode;
   }

   if (!RC_SUCCESS(nRetcode) && nRetcode != SQL_NO_DATA) {
      ExceptionByHandle(SQL_HANDLE_STMT, hStmt,
            _T("DBStmt::Execute"),
            _T("Unable to execute statement SQL!"));
      return nRetcode;
   }
   eStmtState = STMT_EXECUTED;
   if (AfterExecute != NULL)
      AfterExecute(this);

   return nRetcode;
}
//---------------------------------------------------------------------------
SQLRETURN DBStmt::ExecDirect()
{
   nRetcode = SQL_SUCCESS;
   if (!IsAllocated()) {
      nRetcode = Initialize(false);
      if (!RC_SUCCESS(nRetcode))
         return nRetcode;
   }

   assert(hStmt != SQL_NULL_HSTMT);

   //if (IsExecuted())
   //   return;
   if (oParams != NULL) {
      if ((oParams->Count() > 0) && (!oParams->ParamByIndex(0)->IsBind())) {
         nRetcode = oParams->BindParams();
         if (!RC_SUCCESS(nRetcode))
            return nRetcode;
      }
   }

   nRetcode = SQLExecDirect(hStmt, (SQLTCHAR*)sSQLCmd.c_str(), SQL_NTS);
   //Se precisa de dados para os parâmetros deve neste momento
   //invocar os eventos para o usuário informar os dados necessários
   if (nRetcode == SQL_NEED_DATA) {
      nRetcode = PutParamData();
      if (!RC_SUCCESS(nRetcode))
         return nRetcode;
   }

   if (!RC_SUCCESS(nRetcode) && nRetcode != SQL_NO_DATA) {
      ExceptionByHandle(SQL_HANDLE_STMT, hStmt,
            _T("DBStmt::ExecDirect"),
            _T("Unable to execute direct statement SQL!"));
      return nRetcode;
   }

   eStmtState = STMT_EXECUTED;
   if (AfterExecute != NULL)
      AfterExecute(this);

   return nRetcode;
}
//---------------------------------------------------------------------------
SQLRETURN DBStmt::PutParamData()
{
   SQLPOINTER ValuePtr = NULL;
   DBParam *oParam = NULL;
   // get next parameter for which a data pValue is needed
   do {
      nRetcode = SQLParamData(hStmt, &ValuePtr);
      if (!RC_SUCCESS(nRetcode) && nRetcode != SQL_NEED_DATA) {
         ExceptionByHandle(SQL_HANDLE_STMT, hStmt,
               _T("DBStmt::PutParamData"),
               _T("Unable to get param need data for statement SQL!"));
         return nRetcode;
      }
      if (nRetcode != SQL_NEED_DATA)
         break;
      if (nRetcode == SQL_NEED_DATA && oParams == NULL) {
         SetExceptionInfo(_T("DBStmt::PutParamData"),
               _T("Statement need data for parameters, but application not define its."),
               _T("07001"),_T("Wrong number of parameters"),0);
         return SQL_ERROR;
      }
      //Deve encontrar qual o parâmetro que deve ser chamado
      oParam = dynamic_cast<DBParam*>(oParams->ParamByIndex((int)(ValuePtr)-1));
      if (oParam != NULL) {
         if (!RC_SUCCESS(oParam->PutData()))
            return SQL_ERROR;
      }

   } while (nRetcode == SQL_NEED_DATA);
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLLEN DBStmt::RowCount()
{
   SQLLEN nResult = 0;

   assert(hStmt != SQL_NULL_HSTMT);

   if (!IsExecuted())
      return 0;

   nRetcode = SQLRowCount(hStmt, &nResult);
   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_STMT, hStmt,
            _T("DBStmt::RowCount"),
            _T("Unable to get row count for statement SQL!"));
   }
   return nResult;
}
//---------------------------------------------------------------------------
SQLRETURN DBStmt::GetStmtAttr(SQLINTEGER Attribute, SQLPOINTER ValuePtr, SQLINTEGER nLength,
   SQLINTEGER* StringLengthPointer)
{
   assert(hStmt != SQL_NULL_HSTMT);

   nRetcode = SQLGetStmtAttr(hStmt, Attribute, ValuePtr, nLength, StringLengthPointer);
   if (!RC_SUCCESS(nRetcode)) {
      SgOStringStream oErrMsg;
      oErrMsg << _T("Unable to get statement attribute for ")
              << _T("Attribute: ") << Attribute << _T(" Value: ") << ValuePtr;
      ExceptionByHandle(SQL_HANDLE_STMT, hStmt,
            _T("DBStmt::GetStmtAttr"),
            oErrMsg.str());
      return nRetcode;
   }
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
// set statement attributes ... will override the default behavior in DTL
// must cache the attributes we want to set because the hStmt must already exist
// for us to set statement attributes, which only occurs when the statement gets initialized
SQLRETURN DBStmt::SetStmtAttr(SQLINTEGER nAttribute, SQLPOINTER pValue, SQLINTEGER nLength)
{
   DBAttribute *oAttribute = NULL;
   DBAttrList::iterator iAttribute = oAttrs->begin();
   for (; iAttribute != oAttrs->end(); ++iAttribute) {
      if ((*iAttribute)->nAttribute == nAttribute) {
         oAttribute = (*iAttribute);
         oAttribute->pValue = pValue;
         oAttribute->nLength = nLength;
         break;
      }
   }
   if (oAttribute == NULL) {
      oAttribute = new DBAttribute(nAttribute, pValue, nLength);
      oAttrs->push_back(oAttribute);
   }
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN DBStmt::ClearStmtAttr(SQLINTEGER nAttribute)
{
   DBAttribute *oAttribute = NULL;
   DBAttrList::iterator iAttribute = oAttrs->begin();
   for (; iAttribute != oAttrs->end(); ++iAttribute) {
      if ((*iAttribute)->nAttribute == nAttribute) {
         oAttribute = (*iAttribute);
         oAttrs->erase(iAttribute);
         break;
      }
   }
   if (oAttribute != NULL) {
      delete oAttribute;
   }
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN DBStmt::ClearStmtAttrs()
{
   DBAttrList::iterator iAttribute = oAttrs->begin();
   for (; iAttribute != oAttrs->end(); ++iAttribute)
      delete (*iAttribute);
   oAttrs->clear();
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN DBStmt::LoadAttributes()
{
   assert(hStmt != SQL_NULL_HSTMT);

   DBAttrList::iterator iAttribute = oAttrs->begin();
   for (; iAttribute != oAttrs->end(); ++iAttribute) {
      nRetcode = SQLSetStmtAttr(hStmt, (*iAttribute)->nAttribute, (SQLPOINTER) ((*iAttribute)->pValue), (*iAttribute)->nLength);
      if (!RC_SUCCESS(nRetcode)) {
         SgOStringStream oErrMsg;
         oErrMsg << _T("Unable to set statement attribute for ")
                 << _T("Attribute: ") << (*iAttribute)->nAttribute << _T(" Value: ") << (*iAttribute)->pValue;
         ExceptionByHandle(SQL_HANDLE_STMT, hStmt,
               _T("DBStmt::LoadAttributes"),
               oErrMsg.str());
         return nRetcode;
      }
   }
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
IDBParam* DBStmt::ParamByIndex(int nIndex)
{
   return oParams->ParamByIndex(nIndex);
}
//---------------------------------------------------------------------------
IDBParam* DBStmt::ParamByName(const String& sName)
{
   return oParams->ParamByName(sName);
}
//---------------------------------------------------------------------------
IDBRecordset* GetDBRecordset1(
      IDBConnection* oConnection) {
   return new DBRecordset(oConnection);
}
IDBRecordset* GetDBRecordset2(
      IDBConnection* oConnection, const String& sSQLCmd, bool bPrepare) {
   return new DBRecordset(oConnection,sSQLCmd,bPrepare);
}
//---------------------------------------------------------------------------
DBRecordset::DBRecordset(IDBConnection* oConnection) : 
      DBStmt(oConnection), IDBRecordset()
{
   this->nCursorType = SQL_CURSOR_TYPE_DEFAULT;
   this->eRsState = RS_NONE;
   this->nBindOffset = 0;
   this->oFields = NULL;
   this->bBOF = true;
   this->bEOF = true;
   this->BeforeOpen = NULL;
   this->AfterOpen = NULL;
   this->BeforeClose = NULL;
   this->AfterClose = NULL;
   this->AfterScroll = NULL;
}
//---------------------------------------------------------------------------
DBRecordset::DBRecordset(IDBConnection* oConnection, const String& sSQLCmd, bool bPrepare) :
      DBStmt(oConnection, sSQLCmd, bPrepare), IDBRecordset()
{
   this->nCursorType = SQL_CURSOR_TYPE_DEFAULT;
   DBStmt::SetStmtAttr(SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_TYPE_DEFAULT, SQL_IS_INTEGER);
   this->eRsState = RS_NONE;
   this->nBindOffset = 0;
   this->oFields = NULL;
   this->bBOF = true;
   this->bEOF = true;
   this->BeforeOpen = NULL;
   this->AfterOpen = NULL;
   this->BeforeClose = NULL;
   this->AfterClose = NULL;
   this->AfterScroll = NULL;
}
//---------------------------------------------------------------------------
DBRecordset::~DBRecordset()
{
   ReleaseHandles();
}
//---------------------------------------------------------------------------
IDBFields* DBRecordset::getFields() {
   return oFields;
}
//---------------------------------------------------------------------------
RecordsetState DBRecordset::getRsState() {
   return eRsState;
}
//---------------------------------------------------------------------------
bool DBRecordset::getBOF() {
   return bBOF;
}
//---------------------------------------------------------------------------
bool DBRecordset::getEOF() {
   return bEOF;
}
//---------------------------------------------------------------------------
void DBRecordset::setCursorType(SQLUINTEGER nValue) {
   this->nCursorType = nValue;
}
//---------------------------------------------------------------------------
SQLUINTEGER DBRecordset::getCursorType() {
   return nCursorType;
}
//---------------------------------------------------------------------------
void DBRecordset::SetBeforeOpen(RecordsetNotify pValue) {
   this->BeforeOpen = pValue;
}
//---------------------------------------------------------------------------
void DBRecordset::SetAfterOpen(RecordsetNotify pValue) {
   this->AfterOpen = pValue;
}
//---------------------------------------------------------------------------
void DBRecordset::SetBeforeClose(RecordsetNotify pValue) {
   this->BeforeClose = pValue;
}
//---------------------------------------------------------------------------
void DBRecordset::SetAfterClose(RecordsetNotify pValue) {
   this->AfterClose = pValue;
}
//---------------------------------------------------------------------------
void DBRecordset::SetAfterScroll(RecordsetNotify pValue) {
   this->AfterScroll = pValue;
}
//---------------------------------------------------------------------------
SQLRETURN DBRecordset::Execute()
{
   SetExceptionInfo(_T("DBRecordset::Execute"),
      _T("Can't call Execute directly!"),
      _T("HY010"),_T("Function sequence error"),0);
   return SQL_ERROR;
}
//---------------------------------------------------------------------------
SQLRETURN DBRecordset::ExecDirect()
{
   SetExceptionInfo(_T("DBRecordset::Execute"),
      _T("Can't call ExecDirect directly!"),
      _T("HY010"),_T("Function sequence error"),0);
   return SQL_ERROR;
}
//---------------------------------------------------------------------------
SQLRETURN DBRecordset::Open(const String& sSQLCmd, const SQLUINTEGER nCursorType)
{
   this->sSQLCmd = sSQLCmd;
   this->nCursorType = nCursorType;
   return Open();
}
//---------------------------------------------------------------------------
SQLRETURN DBRecordset::Open()
{
   nRetcode = SQL_SUCCESS;
   DBStmt::SetStmtAttr(SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)nCursorType, SQL_IS_INTEGER);
   if(nCursorType != SQL_CURSOR_FORWARD_ONLY)
   {
      DBStmt::SetStmtAttr(SQL_ATTR_CURSOR_SCROLLABLE, (SQLPOINTER)SQL_SCROLLABLE, SQL_IS_INTEGER);
      if (nCursorType == SQL_CURSOR_STATIC) {
         DBStmt::SetStmtAttr(SQL_ATTR_CONCURRENCY, (SQLPOINTER)SQL_CONCUR_READ_ONLY, SQL_IS_INTEGER);
         DBStmt::SetStmtAttr(SQL_ATTR_CURSOR_SENSITIVITY, (SQLPOINTER)SQL_INSENSITIVE, SQL_IS_INTEGER);
      } else {
         DBStmt::SetStmtAttr(SQL_ATTR_CONCURRENCY, (SQLPOINTER)SQL_CONCUR_ROWVER, SQL_IS_INTEGER);
         DBStmt::SetStmtAttr(SQL_ATTR_CURSOR_SENSITIVITY, (SQLPOINTER)SQL_SENSITIVE, SQL_IS_INTEGER);
      }         
      //nRetcode = SQLSetStmtAttr(hStmt, SQL_ATTR_USE_BOOKMARKS, (SQLPOINTER)SQL_UB_VARIABLE, 0);
      //nRetcode = SQLBindCol(hStmt, 0, SQL_C_VARBOOKMARK, cBookmark, sizeof(cBookmark), &nBookmarkLen);
   }

   if (!DBStmt::IsAllocated()) {
      nRetcode = Initialize((bPrepare) ? false : true);
      if (!RC_SUCCESS(nRetcode))
         return nRetcode;
   }

   assert(hStmt != SQL_NULL_HSTMT);

   if(eStmtState == STMT_EXECUTED) {
      nRetcode = Close();
      if (!RC_SUCCESS(nRetcode))
         return nRetcode;
   }

   //Deve verificar se deve setar as colunas da Query;
   SQLSMALLINT nFieldsCount = 0;
   nRetcode = SQLNumResultCols(hStmt, &nFieldsCount);
   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_STMT, hStmt,
            _T("DBRecordset::Open"),
            _T("Unable get number of columns for statement SQL!"));
      return nRetcode;
   }

   //Se não tem a listagem de parametros deve então criar uma
   if (oFields == NULL && nFieldsCount > 0) {
      ClearException();
      oFields = new DBFields(this,true,nFieldsCount);
      oFields->bAutoGenerate = true;
      if (GetLastDBException()->getErrMsg().length() > 0)
         return SQL_ERROR;
   } else {
      //Se a listagem de parametros já está preenchida e é diferente da
      //quantidade de parametros esperada pela Query causa uma excessão
      if (oFields->oListFields->size() != nFieldsCount) {
         SetExceptionInfo(_T("DBStmt::Prepare"),
               _T("Number of columns list diferente of number of columns for statement SQL!"),
               _T("07001"),_T("Wrong number of parameters"),0);
         return SQL_ERROR;
      }
   }

   if (oFields != NULL) {
      if ((oFields->Count() > 0) && (!oFields->FieldByIndex(0)->IsBind())) {
         nRetcode = oFields->BindFields();
         if (!RC_SUCCESS(nRetcode))
            return nRetcode;
      }
   }

   if (BeforeOpen != NULL)
      BeforeOpen(this);

   nRetcode = DBStmt::Execute();
   if (!RC_SUCCESS(nRetcode))
      return nRetcode;

   nRetcode = SQLSetStmtAttr(hStmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)1, 0);
   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_STMT, hStmt,
            _T("DBRecordset::Open"),
            _T("Unable set row array size for statement SQL!"));
      return nRetcode;
   }

   nRetcode = SQLSetStmtAttr(hStmt, SQL_ATTR_ROW_STATUS_PTR, &nRowStatus, 0);
   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_STMT, hStmt,
            _T("DBRecordset::Open"),
            _T("Unable set row status pointer for statement SQL!"));
      return nRetcode;
   }

   nRetcode = SQLSetStmtAttr(hStmt, SQL_ATTR_ROWS_FETCHED_PTR, &nRowsFetched, 0);
   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_STMT, hStmt,
            _T("DBRecordset::Open"),
            _T("Unable set rows fetched pointer for statement SQL!"));
      return nRetcode;
   }

   nRetcode = SQLSetStmtAttr(hStmt, SQL_ATTR_ROW_BIND_TYPE, SQL_BIND_BY_COLUMN, 0);
   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_STMT, hStmt,
            _T("DBRecordset::Open"),
            _T("Unable set row bind type for statement SQL!"));
      return nRetcode;
   }

   nRetcode = SQLSetStmtAttr(hStmt, SQL_ATTR_ROW_BIND_OFFSET_PTR, &nBindOffset, 0);
   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_STMT, hStmt,
            _T("DBRecordset::Open"),
            _T("Unable set row bind offset for statement SQL!"));
      return nRetcode;
   }

   if (AfterOpen != NULL)
      AfterOpen(this);

   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN DBRecordset::Close()
{
   if (BeforeClose != NULL)
      BeforeClose(this);

   nRetcode = SQLCloseCursor(hStmt);
   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_STMT, hStmt,
            _T("DBRecordset::Close"),
            _T("Unable to close cursor!"));
      return nRetcode;
   }
   eStmtState = STMT_PREPARED;

   if (AfterClose != NULL)
      AfterClose(this);

   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN DBRecordset::ReleaseHandles()
{
   if(oFields != NULL) {
      nRetcode = oFields->UnbindFields();
      if (!RC_SUCCESS(nRetcode))
         return nRetcode;
      if (oFields->bAutoGenerate)
         delete oFields;
   }

   if(eStmtState == STMT_UNALLOCATED)
      return SQL_SUCCESS;

   if(eStmtState == STMT_EXECUTED) {
      nRetcode = Close();
      if (!RC_SUCCESS(nRetcode))
         return nRetcode;
   }

   return DBStmt::ReleaseHandles();
}
//---------------------------------------------------------------------------
IDBField* DBRecordset::FieldByIndex(int nIndex)
{
   return oFields->FieldByIndex(nIndex);
}
//---------------------------------------------------------------------------
IDBField* DBRecordset::FieldByName(const String& sName)
{
   return oFields->FieldByName(sName);
}
//---------------------------------------------------------------------------
SQLRETURN DBRecordset::MoveFirst()
{
   SQLRETURN nRetcode = SQL_SUCCESS;
   SQLSMALLINT nOrientation = (nCursorType == SQL_CURSOR_FORWARD_ONLY) ? SQL_FETCH_NEXT : SQL_FETCH_FIRST;

   nRetcode = SQLFetchScroll(hStmt, nOrientation, 0);
   if (!RC_SUCCESS(nRetcode) && nRetcode != SQL_NO_DATA) {
      ExceptionByHandle(SQL_HANDLE_STMT, hStmt,
            _T("DBRecordset::MoveFirst"),
            _T("Unable to move first for cursor!"));
      return nRetcode;
   }
   bEOF = (bool)(nRetcode == SQL_NO_DATA);
   bBOF = true;
   if (RC_SUCCESS(nRetcode)) {
      nRetcode = GetFieldData();
      if (!RC_SUCCESS(nRetcode))
         return nRetcode;
   }

   if (AfterScroll != NULL)
      AfterScroll(this);

   return nRetcode;
}
//---------------------------------------------------------------------------
SQLRETURN DBRecordset::MoveLast()
{
   SQLRETURN nRetcode = SQL_SUCCESS;

   nRetcode = SQLFetchScroll(hStmt, SQL_FETCH_LAST, 0);
   if (!RC_SUCCESS(nRetcode) && nRetcode != SQL_NO_DATA) {
      ExceptionByHandle(SQL_HANDLE_STMT, hStmt,
            _T("DBRecordset::MoveLast"),
            _T("Unable to move last for cursor!"));
      return nRetcode;
   }
   bBOF = (bool)(nRetcode == SQL_NO_DATA);
   bEOF = true;
   if (RC_SUCCESS(nRetcode)) {
      nRetcode = GetFieldData();
      if (!RC_SUCCESS(nRetcode))
         return nRetcode;
   }

   if (AfterScroll != NULL)
      AfterScroll(this);

   return nRetcode;
}
//---------------------------------------------------------------------------
SQLRETURN DBRecordset::MoveNext()
{
   SQLRETURN nRetcode = SQL_SUCCESS;

   nRetcode = SQLFetchScroll(hStmt, SQL_FETCH_NEXT, 0);
   if (!RC_SUCCESS(nRetcode) && nRetcode != SQL_NO_DATA) {
      ExceptionByHandle(SQL_HANDLE_STMT, hStmt,
            _T("DBRecordset::MoveNext"),
            _T("Unable to move next for cursor!"));
      return nRetcode;
   }
   bBOF = (bBOF) ? (bool)(nRetcode == SQL_NO_DATA) : false;
   bEOF = (bool)(nRetcode == SQL_NO_DATA);
   if (RC_SUCCESS(nRetcode)) {
      nRetcode = GetFieldData();
      if (!RC_SUCCESS(nRetcode))
         return nRetcode;
   }

   if (AfterScroll != NULL)
      AfterScroll(this);

   return nRetcode;
}
//---------------------------------------------------------------------------
SQLRETURN DBRecordset::MovePrew()
{
   SQLRETURN nRetcode = SQL_SUCCESS;

   nRetcode = SQLFetchScroll(hStmt, SQL_FETCH_PRIOR, 0);
   if (!RC_SUCCESS(nRetcode) && nRetcode != SQL_NO_DATA) {
      ExceptionByHandle(SQL_HANDLE_STMT, hStmt,
            _T("DBRecordset::MovePrew"),
            _T("Unable to move previous for cursor!"));
      return nRetcode;
   }
   bBOF = (bool)(nRetcode == SQL_NO_DATA);
   bEOF = (bEOF) ? (bool)(nRetcode == SQL_NO_DATA) : false;
   if (RC_SUCCESS(nRetcode)) {
      nRetcode = GetFieldData();
      if (!RC_SUCCESS(nRetcode))
         return nRetcode;
   }

   if (AfterScroll != NULL)
      AfterScroll(this);

   return nRetcode;
}
//---------------------------------------------------------------------------
SQLRETURN DBRecordset::RefreshRow()
{
   SQLRETURN nRetcode = SQL_SUCCESS;

   nRetcode = SQLSetPos(hStmt, 0, SQL_REFRESH, SQL_LOCK_NO_CHANGE);
   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_STMT, hStmt,
            _T("DBRecordset::RefreshRow"),
            _T("Unable to refresh row for cursor!"));
      return nRetcode;
   }
   if (RC_SUCCESS(nRetcode)) {
      nRetcode = GetFieldData();
      if (!RC_SUCCESS(nRetcode))
         return nRetcode;
   }

   return nRetcode;
}
//---------------------------------------------------------------------------
/*void* DBRecordset::GetBookmark()
{
   BYTE *aux = new BYTE(nBookmarkLen);
   CopyMemory(aux, cBookmark, nBookmarkLen);
   return aux;
}
//---------------------------------------------------------------------------
SQLRETURN DBRecordset::GoToBookmark(void* Value)
{
   CopyMemory(cBookmark, Value, nBookmarkLen);
   nRetcode = SQLBulkOperations(hStmt, SQL_FETCH_BY_BOOKMARK);
   if(nRetcode != SQL_NO_DATA)
      RETURNX(SQL_HANDLE_STMT, hStmt);
   return nRetcode;
}*/
//---------------------------------------------------------------------------
SQLRETURN DBRecordset::GetFieldData()
{
   DBField *oField = NULL;
   DataType eDataType = TYPE_UNKNOWN;
   DBFieldList::iterator iField = oFields->oListFields->begin();
   for(; iField != oFields->oListFields->end(); ++iField)
   {
      oField = (*iField);
      eDataType = oField->oType->getDataType();
      if (eDataType == TYPE_LONGVARCHAR ||
          eDataType == TYPE_WLONGVARCHAR ||
          eDataType == TYPE_LONGVARBINARY ||
          eDataType == TYPE_STRINGA ||
          eDataType == TYPE_STRINGW ||
          eDataType == TYPE_BLOB) {
          if (oField->nRetLength > 0) {
            if (!RC_SUCCESS(oField->GetData()))
               return SQL_ERROR;
          }
      }
   }
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN DBRecordset::PutFieldData()
{
   SQLPOINTER ValuePtr;
   DBField *oField = NULL;
   // get next column for which a data pValue is needed
   do {
      ValuePtr = 0;
      nRetcode = SQLParamData(hStmt, &ValuePtr);
      if (!RC_SUCCESS(nRetcode) && nRetcode != SQL_NEED_DATA) {
         ExceptionByHandle(SQL_HANDLE_STMT, hStmt,
               _T("DBRecordset::PutFieldData"),
               _T("Unable to get column need data for statement SQL!"));
         return nRetcode;
      }
      if (nRetcode != SQL_NEED_DATA)
         break;
      if (nRetcode == SQL_NEED_DATA && oFields == NULL) {
         SetExceptionInfo(_T("DBRecordset::PutFieldData"),
               _T("Statement need data for columns, but application not define its."),
               _T("07001"),_T("Wrong number of parameters"),0);
         return SQL_ERROR;
      }
      //Deve encontrar qual o parâmetro que deve ser chamado
      oField = dynamic_cast<DBField*>(oFields->FieldByIndex((int)(ValuePtr)-1));
      if (oField != NULL) {
         if (!RC_SUCCESS(oField->PutData()))
            return SQL_ERROR;
      }

   } while (nRetcode == SQL_NEED_DATA);
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN DBRecordset::Add()
{
   DBField* oField = NULL;
   DBFieldList::iterator iField = oFields->oListFields->begin();
   for(; iField != oFields->oListFields->end(); ++iField)
   {
      oField = (*iField);
      oField->bReadOnly = false;
      if(!oField->bNotNull)
         oField->nRetLength = SQL_NULL_DATA;
      if(oField->bAutoIncrement)
      {
         oField->bReadOnly = true;
         oField->nRetLength = SQL_COLUMN_IGNORE;
      }
   }
   eRsState = RS_ADD;
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN DBRecordset::Edit()
{
   DBField* oField = NULL;
   DBFieldList::iterator iField = iField=oFields->oListFields->begin();
   for(; iField != oFields->oListFields->end(); ++iField)
   {
      oField = (*iField);
      if (!oField->bIsPkKey)
         oField->bReadOnly = false;
      oField->nRetLength = SQL_COLUMN_IGNORE;
      if(oField->bAutoIncrement)
         oField->bReadOnly = true;
   }
   eRsState = RS_EDIT;
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN DBRecordset::Post()
{
   if(eRsState == RS_NONE)
      return SQL_SUCCESS;

   if(eRsState == RS_EDIT)
      nRetcode = SQLSetPos(hStmt, 0, SQL_UPDATE, SQL_LOCK_NO_CHANGE);

   if(eRsState == RS_ADD)
      nRetcode = SQLBulkOperations(hStmt, SQL_ADD); //  SQLSetPos(hStmt, 0, SQL_ADD, SQL_LOCK_NO_CHANGE);

   if (!RC_SUCCESS(nRetcode) && nRetcode != SQL_NEED_DATA) {
      ExceptionByHandle(SQL_HANDLE_STMT, hStmt,
            _T("DBRecordset::Post"),
            _T("Unable to post row for cursor!"));
      return nRetcode;
   }

   //Se precisa de dados para as colunas deve neste momento
   //invocar os eventos para o usuário informar os dados necessários
   if (nRetcode == SQL_NEED_DATA) {
      nRetcode = PutFieldData();
      if (!RC_SUCCESS(nRetcode))
         return nRetcode;
   }

   DBFieldList::iterator iField = oFields->oListFields->begin();
   for ( ; iField != oFields->oListFields->end(); ++iField)
      (*iField)->bReadOnly = true;

   eRsState = RS_NONE;
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN DBRecordset::Delete()
{
   nRetcode = SQLSetPos(hStmt, 0, SQL_DELETE, SQL_LOCK_NO_CHANGE);
   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_STMT, hStmt,
            _T("DBRecordset::Delete"),
            _T("Unable to delete row for cursor!"));
   }
   return nRetcode;
}
//---------------------------------------------------------------------------
IDBParam* GetDBParam1(
      IDBParams* oParams) {
   return new DBParam(oParams);
}
IDBParam* GetDBParam2(
      IDBParams* oParams, SQLSMALLINT nParamNum, const String& sParamName,
      DataType eDataType, SQLULEN nDataSize, SQLSMALLINT nDecimalDigits,
      bool bNotNull, SQLSMALLINT nParamType) {
   return new DBParam(oParams,nParamNum,sParamName,eDataType,nDataSize,nDecimalDigits,bNotNull,nParamType);
}
IDBParam* GetDBParam3(
      IDBParams* oParams, SQLSMALLINT nParamNum, const String& sParamName,
      SQLPOINTER pPointer, DataType eDataType, SQLULEN nDataSize, SQLSMALLINT nDecimalDigits,
      bool bNotNull, SQLSMALLINT nParamType) {
   return new DBParam(oParams,nParamNum,sParamName,pPointer,eDataType,nDataSize,nDecimalDigits,bNotNull,nParamType);
}
//---------------------------------------------------------------------------
DBParam::DBParam(IDBParams* oParams) :
      DBBind(BIND_PARAMETER, (oParams->Count()+1)), IDBParam()
{
   this->oParams = dynamic_cast<DBParams*>(oParams);
   this->nParamType = SQL_PARAM_INPUT;
   this->OnParamPutData = NULL;
   this->OnParamGetData = NULL;
}
//---------------------------------------------------------------------------
DBParam::DBParam(IDBParams* oParams, SQLSMALLINT nParamNum, SQLSMALLINT nParamType) :
      DBBind(BIND_PARAMETER, nParamNum), IDBParam()
{
   this->oParams = dynamic_cast<DBParams*>(oParams);
   this->nParamType = nParamType;
   this->OnParamPutData = NULL;
   this->OnParamGetData = NULL;
   DescribeParam();
   if (this->oType != NULL)
      AllocData();
}
//---------------------------------------------------------------------------
DBParam::DBParam(IDBParams* oParams, SQLSMALLINT nParamNum, const String& sParamName,
         DataType eDataType, SQLULEN nDataSize, SQLSMALLINT nDecimalDigits, bool bNotNull, SQLSMALLINT nParamType) :
         DBBind(BIND_PARAMETER, nParamNum, sParamName, eDataType, nDataSize, nDecimalDigits, bNotNull), IDBParam()
{
   this->oParams = dynamic_cast<DBParams*>(oParams);
   this->nParamType = nParamType;
   this->OnParamPutData = NULL;
   this->OnParamGetData = NULL;
}
//---------------------------------------------------------------------------
DBParam::DBParam(IDBParams* oParams, SQLSMALLINT nParamNum, const String& sParamName,
         SQLPOINTER pPointer, DataType eDataType, SQLULEN nDataSize,
         SQLSMALLINT nDecimalDigits, bool bNotNull, SQLSMALLINT nParamType) :
         DBBind(BIND_PARAMETER, nParamNum, sParamName, pPointer, eDataType, nDataSize, nDecimalDigits, bNotNull), IDBParam()
{
   this->oParams = dynamic_cast<DBParams*>(oParams);
   this->nParamType = nParamType;
   this->OnParamPutData = NULL;
   this->OnParamGetData = NULL;
}
//---------------------------------------------------------------------------
DBParam::~DBParam() {}
//---------------------------------------------------------------------------
IDBParams* DBParam::getParams() {
   return oParams;
}
//---------------------------------------------------------------------------
SQLSMALLINT DBParam::getParamNum() {
   return nBindNum;
}
//---------------------------------------------------------------------------
const String& DBParam::getParamName() {
   return sBindName;
}
//---------------------------------------------------------------------------
SQLSMALLINT DBParam::getParamType() {
   return nParamType;
}
//---------------------------------------------------------------------------
void DBParam::setParamName(const String &Value) {
   sBindName = Value;
   std::transform(sBindName.begin(), sBindName.end(), sBindName.begin(), ::_totlower);
}
//---------------------------------------------------------------------------
void DBParam::setParamType(SQLSMALLINT Value) {
   nParamType = Value;
}
//---------------------------------------------------------------------------
void DBParam::SetOnParamPutData(ParamData pValue) {
   this->OnParamPutData = pValue;
}
//---------------------------------------------------------------------------
void DBParam::SetOnParamGetData(ParamData pValue) {
   this->OnParamGetData = pValue;
}
//---------------------------------------------------------------------------
SQLRETURN DBParam::DescribeParam()
{
   SQLSMALLINT nDataType = SQL_UNKNOWN_TYPE;
   SQLSMALLINT nNullable = SQL_NULLABLE;

   nRetcode = SQLDescribeParam(oParams->oStmt->hStmt, nBindNum,
         &nDataType, &nDataSize, &nDecimalDigits, &nNullable);
   if (!RC_SUCCESS(nRetcode)) {
      SgOStringStream oErrMsg;
      oErrMsg << _T("Unable to get info of parameter nº ") << this->nBindNum << _T("!");
      ExceptionByHandle(SQL_HANDLE_STMT, oParams->oStmt->hStmt,
            _T("DBParam::DescribeParam"),
            oErrMsg.str());
      return nRetcode;
   }
   //Correção para o caso de campos double e float que não retornam decimais
   if (nDecimalDigits == 0 &&
      (nDataType == SQL_DOUBLE || nDataType == SQL_FLOAT || nDataType == SQL_REAL))
      nDecimalDigits = (nDataType == SQL_REAL) ? FLT_DIG : DBL_DIG;

   this->bNotNull = (bool)(nNullable == SQL_NO_NULLS);
   this->oType = GetMapType(GetTypeBySQL(nDataType));
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN DBParam::Bind()
{
   SQLLEN nBufferLen = nDataSize;
   SQLPOINTER pPointer = NULL;

   GetLenForBind(nBufferLen);
   pPointer = (nBufferLen > 0) ? this->pPointer : (SQLPOINTER)nBindNum;

   nRetcode = SQLBindParameter(oParams->oStmt->hStmt, nBindNum,
         nParamType, oType->getNativeType(), oType->getSQLType(), nDataSize,
         nDecimalDigits, pPointer, nBufferLen, &nRetLength);
   if (!RC_SUCCESS(nRetcode)) {
      SgOStringStream oErrMsg;
      oErrMsg << _T("Unable to bind info of parameter nº ") << this->nBindNum << _T("!");
      ExceptionByHandle(SQL_HANDLE_STMT, oParams->oStmt->hStmt,
            _T("DBParam::BindParam"),
            oErrMsg.str());
      return nRetcode;
   }

   bIsBind = true;
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN DBParam::PutData()
{
   if (OnParamPutData != NULL) {
      OnParamPutData(this, pPointer, nRetLength);
   } else {
      nRetcode = DBBind::PutData(oParams->oStmt->hStmt);
      if (!RC_SUCCESS(nRetcode)) {
         SgOStringStream oErrMsg;
         oErrMsg << _T("Unable to put data info for parameter. sName: ") << sBindName << _T(" Number: ") << nBindNum;
         ExceptionByHandle(SQL_HANDLE_STMT, oParams->oStmt->hStmt,
               _T("DBParam::PutData"),
               oErrMsg.str());
         return nRetcode;
      }
   }
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN DBParam::GetData()
{
   if (OnParamGetData != NULL) {
      OnParamGetData(this, pPointer, nRetLength);
   } else {
      nRetcode = DBBind::GetData(oParams->oStmt->hStmt);
      if (!RC_SUCCESS(nRetcode)) {
         SgOStringStream oErrMsg;
         oErrMsg << _T("Unable to get data info for parameter. sName: ") << sBindName << _T(" Number: ") << nBindNum;
         ExceptionByHandle(SQL_HANDLE_STMT, oParams->oStmt->hStmt,
               _T("DBParam::GetData"),
               oErrMsg.str());
         return nRetcode;
      }
   }
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
IDBParams* GetDBParams1(
      IDBStmt* oStmt) {
   return new DBParams(oStmt);
}
//---------------------------------------------------------------------------
DBParams::DBParams(IDBStmt* oStmt, bool bAutoGenerate, SQLSMALLINT nParamsCount) : 
      DBObject(), IDBParams()
{
   this->nRetcode = SQL_SUCCESS;
   this->oStmt = dynamic_cast<DBStmt*>(oStmt);
   this->oListParams = new DBParamList();
   this->bAutoGenerate = false;
   if (this->oStmt != NULL) {
      if (this->oStmt->oParams != NULL) {
         if (this->oStmt->oParams->bAutoGenerate)
            delete this->oStmt->oParams;
      }
      this->oStmt->oParams = this;
   }
   if (bAutoGenerate && this->oStmt->hStmt != SQL_NULL_HSTMT)
      CreateListParams(nParamsCount);
}
//---------------------------------------------------------------------------
DBParams::~DBParams()
{
   this->UnbindParams();
   if(oListParams != NULL)
   {
      DBParamList::iterator i;
      for(i=oListParams->begin(); i != oListParams->end(); ++i)
         delete (*i);
      delete oListParams;
   }
   if (oStmt != NULL && oStmt->oParams == this) {
      oStmt->oParams = NULL;
   }
}
//---------------------------------------------------------------------------
IDBStmt* DBParams::getStmt() {
   return oStmt;
}
//---------------------------------------------------------------------------
SQLRETURN DBParams::CreateListParams(SQLSMALLINT nParamsCount)
{
   SQLSMALLINT iIndex;
   DBParam *oParam = NULL;

   if (nParamsCount <= 0) {
      nRetcode = SQLNumParams(oStmt->hStmt, &nParamsCount);
      if (!RC_SUCCESS(nRetcode)) {
         ExceptionByHandle(SQL_HANDLE_STMT, oStmt->hStmt,
               _T("DBParams::CreateListParams"),
               _T("Unable to get number of parameters for statement SQL!"));
         return nRetcode;
      }
   }

   oListParams->reserve(nParamsCount);
   ClearException();
   for(iIndex=0;iIndex<nParamsCount;iIndex++) {
      oParam = new DBParam(this, iIndex+1);
      if (GetLastDBException()->getErrMsg().length() > 0)
         return SQL_ERROR;
      oListParams->push_back(oParam);
   }
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
IDBParam* DBParams::ParamByIndex(int nIndex)
{
   if((size_t)nIndex >= oListParams->size()) {
      return NULL;
   } else {
      return oListParams->at(nIndex);
   }
}
//---------------------------------------------------------------------------
IDBParam* DBParams::ParamByName(const String& sName)
{
   String sNameL = sName;
   std::transform(sNameL.begin(), sNameL.end(), sNameL.begin(), ::_totlower);
   DBParamList::iterator iParam = oListParams->begin();
   for(; iParam != oListParams->end(); ++iParam)
      if(sNameL == ((*iParam)->sBindName))
         return (*iParam);
   return NULL;
}
//---------------------------------------------------------------------------
IDBParam* DBParams::Add(IDBParam* oParam)
{
   DBParam *oCastParam = dynamic_cast<DBParam*>(oParam);
   oListParams->push_back(oCastParam);
   oCastParam->nBindNum = oListParams->size();

   return oParam;
}
//---------------------------------------------------------------------------
IDBParam* DBParams::Insert(int nIndex, IDBParam* oParam)
{
   DBParam *oCastParam = dynamic_cast<DBParam*>(oParam);
   oCastParam->nBindNum = nIndex + 1;
   //Se inclui como o último item da lista
   if ((oListParams->size() == 0) ||(oListParams->size() <= (size_t)nIndex)) {
      oListParams->push_back(oCastParam);
      oCastParam->nBindNum = oListParams->size();
   } else { //Inclui no meio da lista e reordena a numeração dos parâmetros
      DBParamList::iterator iParam = oListParams->begin() + nIndex;

      oListParams->insert(iParam, oCastParam);

      iParam = oListParams->begin() + (nIndex + 1);
      while (iParam != oListParams->end()) {
         (*iParam)->nBindNum++;
         iParam++;
      }
   }

   return oParam;
}
//---------------------------------------------------------------------------
IDBParam* DBParams::Remove(int nIndex)
{
   DBParam *oParam = NULL;

   if(oListParams == NULL)
      return oParam;

   if((size_t)nIndex >= oListParams->size())
      return oParam;

   //Se retira do final da lista, apenas exclui
   if((oListParams->size() - 1) == (size_t)nIndex) {
      oParam = oListParams->at(oListParams->size()-1);
      oListParams->pop_back();
   } else { //Retira do meio da lista e reordena a numeração dos parâmetros
      DBParamList::iterator iParam = oListParams->begin() + nIndex;
      oParam = (*iParam);
      oListParams->erase(iParam);

      iParam = oListParams->begin() + nIndex;
      while (iParam != oListParams->end()) {
         (*iParam)->nBindNum--;
         iParam++;
      }
   }
   oParam->nBindNum = 0;
   return oParam;
}
//---------------------------------------------------------------------------
int DBParams::Count()
{
   if(oListParams == NULL)
      return 0;
   else
      return oListParams->size();
}
//---------------------------------------------------------------------------
SQLRETURN DBParams::BindParams()
{
   DBParamList::iterator iParam = oListParams->begin();
   for(; iParam != oListParams->end(); ++iParam) {
      if (!RC_SUCCESS((*iParam)->Bind()))
         return SQL_ERROR;
   }
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN DBParams::UnbindParams()
{
   if (oListParams->size() <= 0)
      return SQL_SUCCESS;

   if (!this->ParamByIndex(0)->IsBind())
      return SQL_SUCCESS;

   nRetcode = SQLFreeStmt(oStmt->hStmt, SQL_RESET_PARAMS);
   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_STMT, oStmt->hStmt,
            _T("DBParams::UnbindParams"),
            _T("Unable to reset parameters for statement handle!"));
      return nRetcode;
   }

   DBParamList::iterator iParam = oListParams->begin();
   for(; iParam != oListParams->end(); ++iParam) {
      if (!RC_SUCCESS((*iParam)->Unbind()))
         return SQL_ERROR;
   }

   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
IDBField* GetDBField1(
      IDBFields* oFields) {
   return new DBField(oFields);
}
IDBField* GetDBField2(
      IDBFields* oFields, SQLSMALLINT nFieldNum, const String& sFieldName,
      DataType eDataType, SQLULEN nDataSize, SQLSMALLINT nDecimalDigits,
      bool bNotNull, bool bIsPkKey, bool bIsVirtual) {
   return new DBField(oFields,nFieldNum,sFieldName,eDataType,nDataSize,nDecimalDigits,bNotNull,bIsPkKey,bIsVirtual);
}
IDBField* GetDBField3(
      IDBFields* oFields, SQLSMALLINT nFieldNum, const String& sFieldName,
      SQLPOINTER pPointer, DataType eDataType, SQLULEN nDataSize, SQLSMALLINT nDecimalDigits,
      bool bNotNull, bool bIsPkKey, bool bIsVirtual) {
   return new DBField(oFields,nFieldNum,sFieldName,pPointer,eDataType,nDataSize,nDecimalDigits,bNotNull,bIsPkKey,bIsVirtual);
}
//---------------------------------------------------------------------------
DBField::DBField(IDBFields* oFields) :
   DBBind(BIND_COLUMN, (oFields->Count()+1)), IDBField()
{
   this->oFields = dynamic_cast<DBFields*>(oFields);
   this->bReadOnly = true;
   this->bIsPkKey = false;
   this->bIsVirtual = true;
   this->bAutoIncrement = false;
   this->OnFieldPutData = NULL;
   this->OnFieldGetData = NULL;
}
//---------------------------------------------------------------------------
DBField::DBField(IDBFields* oFields, SQLSMALLINT nFieldNum) :
      DBBind(BIND_COLUMN, nFieldNum), IDBField()
{
   this->oFields = dynamic_cast<DBFields*>(oFields);
   this->bReadOnly = true;
   this->bIsPkKey = false;
   this->bAutoIncrement = false;
   this->OnFieldPutData = NULL;
   this->OnFieldGetData = NULL;
   if (RC_SUCCESS(DescribeColumn()))
      this->bIsVirtual = false;
   else
      this->bIsVirtual = true;
   if (this->oType != NULL)
      AllocData();
}
//---------------------------------------------------------------------------
DBField::DBField(IDBFields* oFields, SQLSMALLINT nFieldNum, const String& sFieldName, 
         DataType eDataType, SQLULEN nDataSize, SQLSMALLINT nDecimalDigits, 
         bool bNotNull, bool bIsPkKey, bool bIsVirtual) :
         DBBind(BIND_COLUMN, nFieldNum, sFieldName, eDataType, nDataSize, nDecimalDigits, bNotNull), IDBField()
{
   this->oFields = dynamic_cast<DBFields*>(oFields);
   this->bIsPkKey = (bIsPkKey && !bIsVirtual);
   this->bIsVirtual = bIsVirtual;
   this->bReadOnly = true;
   this->bAutoIncrement = false;
   this->OnFieldPutData = NULL;
   this->OnFieldGetData = NULL;
}
//---------------------------------------------------------------------------
DBField::DBField(IDBFields* oFields, SQLSMALLINT nFieldNum, const String& sFieldName,
         SQLPOINTER pPointer, DataType eDataType, SQLULEN nDataSize, SQLSMALLINT nDecimalDigits, 
         bool bNotNull, bool bIsPkKey, bool bIsVirtual) :
         DBBind(BIND_COLUMN, nFieldNum, sFieldName, pPointer, eDataType, nDataSize, nDecimalDigits, bNotNull), IDBField()
{
   this->oFields = dynamic_cast<DBFields*>(oFields);
   this->bIsPkKey = (bIsPkKey && !bIsVirtual);
   this->bIsVirtual = bIsVirtual;
   this->bReadOnly = true;
   this->bAutoIncrement = false;
   this->OnFieldPutData = NULL;
   this->OnFieldGetData = NULL;
}
//---------------------------------------------------------------------------
DBField::~DBField() {}
//---------------------------------------------------------------------------
IDBFields* DBField::getFields() {
   return oFields;
}
//---------------------------------------------------------------------------
SQLSMALLINT DBField::getFieldNum() {
   return nBindNum;
}
//---------------------------------------------------------------------------
const String& DBField::getFieldName() {
   return sBindName;
}
//---------------------------------------------------------------------------
void DBField::setFieldName(const String &Value) {
   sBindName = Value;
   std::transform(sBindName.begin(), sBindName.end(), sBindName.begin(), ::_totlower);
}
//---------------------------------------------------------------------------
bool DBField::getAutoIncrement() {
   return bAutoIncrement;
}
//---------------------------------------------------------------------------
SQLRETURN DBField::setAutoIncrement(const bool Value) {
   if (bIsVirtual && Value) {
      TCHAR cErrMsg[SHORT_MSG] = {0};
      SgFormatMsg(cErrMsg,SHORT_MSG,_T("Unable set autoincrement to virtual column. Name: %s Number: %d"),sBindName,nBindNum);
      SetExceptionInfo(_T("DBField::setAutoIncrement"),cErrMsg,_T("HY011"),_T("Attribute cannot be set now"),0);
      return SQL_ERROR;
   }
   bAutoIncrement = Value;
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
bool DBField::getIsPkKey() {
   return bIsPkKey;
}
//---------------------------------------------------------------------------
SQLRETURN DBField::setIsPkKey(const bool Value) {
   if (bIsVirtual && Value) {
      TCHAR cErrMsg[SHORT_MSG] = {0};
      SgFormatMsg(cErrMsg,SHORT_MSG,_T("Unable set primary key to virtual column. Name: %s Number: %d"),sBindName,nBindNum);
      SetExceptionInfo(_T("DBField::setIsPkKey"),cErrMsg,_T("HY011"),_T("Attribute cannot be set now"),0);
      return SQL_ERROR;
   }
   bIsPkKey = Value;
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
bool DBField::getIsVirtual() {
   return bIsVirtual;
}
//---------------------------------------------------------------------------
SQLRETURN DBField::setIsVirtual(const bool Value) {
   if (bIsBind) {
      TCHAR cErrMsg[SHORT_MSG] = {0};
      SgFormatMsg(cErrMsg,SHORT_MSG,_T("Unable set virtual after bind column. Name: %s Number: %d"),sBindName,nBindNum);
      SetExceptionInfo(_T("DBField::setIsVirtual"),cErrMsg,_T("HY011"),_T("Attribute cannot be set now"),0);
      return SQL_ERROR;
   }
   bIsVirtual = Value;
   if (bIsVirtual) {
      bIsPkKey = false;
      bAutoIncrement = false;
   }
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
bool DBField::getReadOnly() {
   return bReadOnly;
}
//---------------------------------------------------------------------------
void DBField::SetOnFieldPutData(FieldData pValue) {
   this->OnFieldPutData = pValue;
}
//---------------------------------------------------------------------------
void DBField::SetOnFieldGetData(FieldData pValue) {
   this->OnFieldGetData = pValue;
}
//---------------------------------------------------------------------------
SQLRETURN DBField::DescribeColumn()
{
   SQLSMALLINT nDataType = SQL_UNKNOWN_TYPE;
   SQLSMALLINT nNullable = SQL_NULLABLE;
   SQLTCHAR *cColumnName = NULL;
   SQLSMALLINT nBufferLen = oFields->oRecordset->oConnection->getMaxColumnName() + 1;
   SQLSMALLINT nNameLengthPtr = 0;
   SQLSMALLINT nStringLengthPtr = 0;
   SQLLEN nNumericAttributePtr = 0;

   cColumnName = new SQLTCHAR[nBufferLen];

   nRetcode = SQLDescribeCol(oFields->oRecordset->hStmt, nBindNum,
         cColumnName, nBufferLen, &nNameLengthPtr, &nDataType,
         &nDataSize, &nDecimalDigits, &nNullable);
   if (!RC_SUCCESS(nRetcode)) {
      SgOStringStream oErrMsg;
      oErrMsg << _T("Unable to get info of column nº ") << this->nBindNum << _T("!");
      ExceptionByHandle(SQL_HANDLE_STMT, oFields->oRecordset->hStmt,
            _T("DBField::DescribeColumn"),
            oErrMsg.str());
      delete[] cColumnName;
      return nRetcode;
   }
   sBindName = (TCHAR*)cColumnName;
   delete[] cColumnName;
   std::transform(sBindName.begin(), sBindName.end(), sBindName.begin(), ::_totlower);

   nRetcode = SQLColAttribute(oFields->oRecordset->hStmt,nBindNum,SQL_DESC_AUTO_UNIQUE_VALUE,NULL,sizeof(SQLLEN), &nStringLengthPtr, &nNumericAttributePtr);
   if (!RC_SUCCESS(nRetcode)) {
      SgOStringStream oErrMsg;
      oErrMsg << _T("Unable to get auto increment of column nº ") << this->nBindNum << _T("!");
      ExceptionByHandle(SQL_HANDLE_STMT, oFields->oRecordset->hStmt,
            _T("DBField::DescribeColumn"),
            oErrMsg.str());
      return nRetcode;
   }
   bAutoIncrement = (nNumericAttributePtr == SQL_TRUE);

   nRetcode = SQLColAttribute(oFields->oRecordset->hStmt,nBindNum,SQL_DESC_UNSIGNED,NULL,sizeof(SQLLEN), &nStringLengthPtr, &nNumericAttributePtr);
   if (!RC_SUCCESS(nRetcode)) {
      SgOStringStream oErrMsg;
      oErrMsg << _T("Unable to get unsigned pValue of column nº ") << this->nBindNum << _T("!");
      ExceptionByHandle(SQL_HANDLE_STMT, oFields->oRecordset->hStmt,
            _T("DBField::DescribeColumn"),
            oErrMsg.str());
      return nRetcode;
   }
   bUnsigned = (nNumericAttributePtr == SQL_TRUE);

   //Correção para o caso de campos double e float que não retornam decimais
   if (nDecimalDigits == 0 &&
      (nDataType == SQL_DOUBLE || nDataType == SQL_FLOAT || nDataType == SQL_REAL))
      nDecimalDigits = (nDataType == SQL_REAL) ? FLT_DIG : DBL_DIG;

   this->bNotNull = (bool)(nNullable == SQL_NO_NULLS);
   this->oType = GetMapType(GetTypeBySQL(nDataType, bUnsigned));

   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN DBField::Bind()
{
   if (this->bIsVirtual)
      return SQL_SUCCESS;
      
   SQLLEN nBufferLen = nDataSize;
   SQLPOINTER pPointer = NULL;

   GetLenForBind(nBufferLen);
   pPointer = (nBufferLen > 0) ? this->pPointer : (SQLPOINTER)nBindNum;

   nRetcode = SQLBindCol(oFields->oRecordset->hStmt, nBindNum,
         oType->getNativeType(), pPointer, nBufferLen, &nRetLength);
   if (!RC_SUCCESS(nRetcode)) {
      SgOStringStream oErrMsg;
      oErrMsg << _T("Unable to bind info of column nº ") << this->nBindNum << _T("!");
      ExceptionByHandle(SQL_HANDLE_STMT, oFields->oRecordset->hStmt,
            _T("DBField::Bind"),
            oErrMsg.str());
      return nRetcode;
   }
   bIsBind = true;
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN DBField::PutData()
{
   if (OnFieldPutData != NULL) {
      OnFieldPutData(this, pPointer, nRetLength);
   } else {
      nRetcode = DBBind::PutData(oFields->oRecordset->hStmt);
      if (!RC_SUCCESS(nRetcode)) {
         SgOStringStream oErrMsg;
         oErrMsg << _T("Unable to put data info for column. sName: ") << sBindName << _T(" Number: ") << nBindNum;
         ExceptionByHandle(SQL_HANDLE_STMT, oFields->oRecordset->hStmt,
               _T("DBField::PutData"),
               oErrMsg.str());
         return nRetcode;
      }
   }
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN DBField::GetData()
{
   if (OnFieldGetData != NULL) {
      OnFieldGetData(this, pPointer, nRetLength);
   } else {
      nRetcode = DBBind::GetData(oFields->oRecordset->hStmt);
      if (!RC_SUCCESS(nRetcode)) {
         SgOStringStream oErrMsg;
         oErrMsg << _T("Unable to get data info for column. sName: ") << sBindName << _T(" Number: ") << nBindNum;
         ExceptionByHandle(SQL_HANDLE_STMT, oFields->oRecordset->hStmt,
               _T("DBField::GetData"),
               oErrMsg.str());
         return nRetcode;
      }
   }
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
IDBFields* GetDBFields1(
      IDBRecordset* oRecordset) {
   return new DBFields(oRecordset);
}
//---------------------------------------------------------------------------
DBFields::DBFields(IDBRecordset* oRecordset, bool bAutoGenerate, SQLSMALLINT nFieldsCount) : 
      DBObject(), IDBFields()
{
   this->nRetcode = SQL_SUCCESS;
   this->oRecordset = dynamic_cast<DBRecordset*>(oRecordset);
   this->oListFields = new DBFieldList();
   this->bAutoGenerate = false;
   if (this->oRecordset != NULL) {
      if (this->oRecordset->oFields != NULL) {
         if (this->oRecordset->oFields->bAutoGenerate)
            delete this->oRecordset->oFields;
      }
      this->oRecordset->oFields = this;
   }
   if (bAutoGenerate && this->oRecordset->hStmt != SQL_NULL_HSTMT)
      CreateListFields(nFieldsCount);
}
//---------------------------------------------------------------------------
DBFields::~DBFields()
{
   this->UnbindFields();
   if(oListFields != NULL)
   {
      DBFieldList::iterator i;
      for(i=oListFields->begin(); i != oListFields->end(); ++i)
         delete (*i);
      delete oListFields;
   }
   if (oRecordset != NULL && oRecordset->oFields == this) {
      oRecordset->oFields = NULL;
   }
}
//---------------------------------------------------------------------------
IDBRecordset* DBFields::getRecordset() {
   return oRecordset;
}
//---------------------------------------------------------------------------
SQLRETURN DBFields::CreateListFields(SQLSMALLINT nFieldsCount)
{
   SQLSMALLINT iIndex;

   if (nFieldsCount <= 0) {
      nRetcode = SQLNumResultCols(oRecordset->hStmt, &nFieldsCount);
      if (!RC_SUCCESS(nRetcode)) {
         ExceptionByHandle(SQL_HANDLE_STMT, oRecordset->hStmt,
               _T("DBFields::CreateListFields"),
               _T("Unable to get number of columns for statement SQL!"));
         return nRetcode;
      }
   }

   oListFields->reserve(nFieldsCount);
   for(iIndex=0;iIndex<nFieldsCount;iIndex++) {
      oListFields->push_back(new DBField(this, iIndex+1));
   }

   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
IDBField* DBFields::FieldByIndex(int nIndex)
{
   if((size_t)nIndex >= oListFields->size()) {
      return NULL;
   } else {
      return oListFields->at(nIndex);
   }
}
//---------------------------------------------------------------------------
IDBField* DBFields::FieldByName(const String& sName)
{
   String sNameL = sName;
   std::transform(sNameL.begin(), sNameL.end(), sNameL.begin(), ::_totlower);
   DBFieldList::iterator iField = oListFields->begin();
   for(; iField != oListFields->end(); ++iField)
      if(sNameL == ((*iField)->sBindName))
         return (*iField);
   return NULL;
}
//---------------------------------------------------------------------------
IDBField* DBFields::Add(IDBField* oField)
{
   DBField *oCastField = dynamic_cast<DBField*>(oField);
   oListFields->push_back(oCastField);
   oCastField->nBindNum = oListFields->size();

   return oField;
}
//---------------------------------------------------------------------------
IDBField* DBFields::Insert(int nIndex, IDBField* oField)
{
   DBField *oCastField = dynamic_cast<DBField*>(oField);
   oCastField->nBindNum = nIndex + 1;
   //Se inclui como o último item da lista
   if ((oListFields->size() == 0) || (oListFields->size() <= (size_t)nIndex)) {
      oListFields->push_back(oCastField);
      oCastField->nBindNum = oListFields->size();
   } else { //Inclui no meio da lista e reordena a numeração dos parâmetros
      DBFieldList::iterator iField = oListFields->begin() + nIndex;

      oListFields->insert(iField, oCastField);

      iField = oListFields->begin() + (nIndex + 1);
      while (iField != oListFields->end()) {
         (*iField)->nBindNum++;
         iField++;
      }
   }

   return oField;
}
//---------------------------------------------------------------------------
IDBField* DBFields::Remove(int nIndex)
{
   DBField* oField = NULL;

   if(oListFields == NULL)
      return oField;

   if((size_t)nIndex >= oListFields->size())
      return oField;

   //Se retira do final da lista, apenas exclui
   if((oListFields->size() - 1) == (size_t)nIndex) {
      oField = oListFields->at(oListFields->size()-1);
      oListFields->pop_back();
   } else { //Retira do meio da lista e reordena a numeração dos parâmetros
      DBFieldList::iterator iField = oListFields->begin() + nIndex;
      oField = (*iField);
      oListFields->erase(iField);

      iField = oListFields->begin() + nIndex;
      while (iField != oListFields->end()) {
         (*iField)->nBindNum--;
         iField++;
      }
   }
   oField->nBindNum = 0;
   return oField;
}
//---------------------------------------------------------------------------
int DBFields::Count()
{
   if(oListFields == NULL)
      return 0;
   else
      return oListFields->size();
}
//---------------------------------------------------------------------------
SQLRETURN DBFields::BindFields()
{
   DBFieldList::iterator iField = oListFields->begin();
   for(; iField != oListFields->end(); ++iField) {
      if (!RC_SUCCESS((*iField)->Bind()))
         return SQL_ERROR;
   }
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN DBFields::UnbindFields()
{
   bool bIsBind = false;
   
   if (oListFields->size() <= 0)
      return SQL_SUCCESS;

   DBFieldList::iterator iField = oListFields->begin();
   for(; iField != oListFields->end() && !bIsBind; ++iField) {
      bIsBind = (*iField)->IsBind();
   }
   
   if (!bIsBind)
      return SQL_SUCCESS;

   nRetcode = SQLFreeStmt(oRecordset->hStmt, SQL_UNBIND);
   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_STMT, oRecordset->hStmt,
            _T("DBFields::UnbindFields"),
            _T("Unable to unbind columns for statement handle!"));
      return nRetcode;
   }

   iField = oListFields->begin();
   for(; iField != oListFields->end(); ++iField) {
      if (!RC_SUCCESS((*iField)->Unbind()))
         return SQL_ERROR;
   }

   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
/*
static DBStmt oStmt(&oDefaultConn);
static DBParams oParams(&oStmt);
static DBParam oParam(&oParams);
static DBRecordset oRecordset(&oDefaultConn);
static DBFields oFields(&oRecordset);
static DBField oField(&oFields);
*/
}; //end namespace sfg
//---------------------------------------------------------------------------
