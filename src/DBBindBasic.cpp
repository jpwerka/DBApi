#include <cstdio>
#include <cassert>
#include <climits>
#include <cfloat>
#include <map>
#include <typeinfo>
#include <algorithm>
#include "DBBindBasic.hpp"

#define STRA_DATE_FMT  "%04d-%02d-%02d"
#define STRW_DATE_FMT L"%04d-%02d-%02d"
#define STRA_HOUR_FMT  "%02d:%02d:%02d"
#define STRW_HOUR_FMT L"%02d:%02d:%02d"
#define STRA_DTTM_FMT  "%04d-%02d-%02d %02d:%02d:%02d"
#define STRW_DTTM_FMT L"%04d-%02d-%02d %02d:%02d:%02d"

//http://the-control-freak.com/DateTime/DateTime.htm
//---------------------------------------------------------------------------
namespace sfg {
//---------------------------------------------------------------------------
const long MlsInDay  = (long)(1000*60*60*24);
const long SecInDay  = (long)(60*60*24);
const long MinInDay  = (long)(60*24);
const long HourInDay = (long)(24);
//Funções Auxiliares Date and Times
long int MDY2GD ( int nMonth, int nDay, int nYear )
{  /*                                     Jan   Feb   Mar   Apr   May   Jun   Jly   Aug   Sep   Oct   Nov   Dec   */
   static const int  nDays [ 13 ] = {    0,    0,  -31,  306,  275,  245,  214,  184,  153,  122,   92,   61,   31 };
   static const int  nBias [ 13 ] = {    0,    1,    1,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0 };

   long  nYears, nDate;

   nYears = nYear - nBias [ nMonth ];

   nDate = ( ( nYears / 100 ) * 146097l ) / 4
         + ( ( nYears % 100 ) *    1461 ) / 4
         - nDays [ nMonth ]
         + nDay;

   return nDate;

}  /* long MDY2GD ( int nMonth, int nDay, int nYear ) */
double CAL2GD ( int nYear, int nMo, int nDy, int nHr, int nMn, int nSc, int nMs )
{
   double nDate = (double)(MDY2GD(nMo, nDy, nYear))
                + ((double)nHr * (1.0 / HourInDay))
                + ((double)nMn * (1.0 / MinInDay))
                + ((double)nSc * (1.0 / SecInDay))
                + ((double)nMs * (1.0 / MlsInDay));

   return nDate;
}

void GD2MDY ( long int lnDate, int * pnMonth, int * pnDay, int * pnYear )
{     /*                            Jan   Feb   Mar   Apr   May   Jun   Jly   Aug   Sep   Oct   Nov   Dec   */
   static const int  nBias [ 12 ] = {    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,  -10,  -10 };
   long              lnLD, lnYr, lnMo, lnDy;

   lnLD = 4 * ( lnDate + 306 ) - 1;

   lnYr = ( lnLD / 146097l ) * 100;
   lnDy = ( lnLD % 146097l ) /   4;

   lnLD = 4 * lnDy + 3;

   lnYr = ( lnLD / 1461 )     + lnYr;
   lnDy = ( lnLD % 1461 ) / 4 + 1;

   lnLD = 5 * lnDy - 3;

   lnMo = ( lnLD / 153 )     + 1;
   lnDy = ( lnLD % 153 ) / 5 + 1;

   * pnMonth   = ( int ) ( lnMo + nBias [ lnMo - 1 ] );
   * pnDay     = ( int ) ( lnDy );
   * pnYear    = ( int ) ( lnYr + lnMo/11 );

}  /* void GD2MDY ( long lnDate, int * pnMonth, int * pnDay, int * pnYear ) */
void GD2CAL ( double gdDateTime, int * lpnYear, int * lpnMo, int * lpnDy, int * lpnHr, int * lpnMn, int * lpnSc, int * lpnMs )
{
   GD2MDY((long)gdDateTime, lpnMo, lpnDy, lpnYear);

   long lnTime = ( long ) ( ( gdDateTime - ( double ) ( ( long ) gdDateTime ) ) * 86400000.0 );

   if ( lpnHr ) * lpnHr = ( int ) ( ( lnTime /  3600000L ) %   24L );   /* Hours          */
   if ( lpnMn ) * lpnMn = ( int ) ( ( lnTime /    60000L ) %   60L );   /* Minutes        */
   if ( lpnSc ) * lpnSc = ( int ) ( ( lnTime /     1000L ) %   60L );   /* Seconds        */

   if ( lpnMs ) * lpnMs = ( int ) ( ( lnTime /        1L ) % 1000L );   /* Milliseconds   */

}
//---------------------------------------------------------------------------
double DateTimeToDouble(const SQL_DATETIME_STRUCT *sqldt)
{
   return CAL2GD(sqldt->year, sqldt->month, sqldt->day,
                 sqldt->hour, sqldt->minute, sqldt->second, sqldt->fraction);
}
//---------------------------------------------------------------------------
void DoubleToDateTime(double dDateTime, SQL_DATETIME_STRUCT *sqldt)
{
   int y,m,d,h,n,s,f;
   GD2CAL(dDateTime,&y,&m,&d,&h,&n,&s,&f);
   SgZeroMemory(sqldt,sizeof(SQL_DATETIME_STRUCT));
   sqldt->year = y;
   sqldt->month = m;
   sqldt->day = d;
   sqldt->hour = h;
   sqldt->minute = n;
   sqldt->second = s;
   sqldt->fraction = f;
}
//---------------------------------------------------------------------------
TCHAR* CopyStr(TCHAR *cStrDest, SQLULEN &nStrLen, TCHAR *cStrSrc) {
   TCHAR *pStrDest = cStrDest;
   SQLULEN nLenRest = nStrLen;
   
   if (cStrDest == NULL) {
      nStrLen = 0;
      return cStrDest;
   }

   if (cStrSrc == NULL) {
      nStrLen = 0;
      *cStrDest = _T('\0');
      return cStrDest;
   }
      
   while (nLenRest && (*cStrSrc != _T('\0'))) {
      *pStrDest++= *cStrSrc++;
      nLenRest--;
   }
   
   if (nLenRest > 0) {
      SgZeroMemory(pStrDest+1,(SQLULEN)(pStrDest+(nLenRest*sizeof(TCHAR))));
   } else { //A String destino foi truncada
      pStrDest--;
      nLenRest++;
   }   
   *pStrDest = _T('\0');
   nStrLen = (pStrDest - cStrDest) / sizeof(TCHAR);
   return cStrDest;
}
//---------------------------------------------------------------------------
TCHAR* CopyStr(TCHAR *cStrDest, SQLULEN &nStrLen, const String &sStrSrc) {
   TCHAR *pStrDest = cStrDest;
   SQLULEN nLenRest = nStrLen;
   
   if (cStrDest == NULL) {
      nStrLen = 0;
      return cStrDest;
   }

   String::const_iterator iStrSrc = sStrSrc.begin();
   while (nLenRest && (iStrSrc != sStrSrc.end())) {
      *pStrDest++= *iStrSrc++;
      nLenRest--;
   }
   
   if (nLenRest > 0) {
      SgZeroMemory(pStrDest+1,(SQLULEN)(pStrDest+(nLenRest*sizeof(TCHAR))));
   } else { //A String destino foi truncada
      pStrDest--;
      nLenRest++;
   }   
   *pStrDest = _T('\0');
   nStrLen = (pStrDest - cStrDest) / sizeof(TCHAR);
   return cStrDest;
}
//---------------------------------------------------------------------------
static DBException oException;
IDBException* GetLastDBException(void) {
   return &oException;
}
//---------------------------------------------------------------------------
const String& DBException::getExType() {
   return sExType; 
}
//---------------------------------------------------------------------------
const String& DBException::getMethod() {
   return sMethod;
}
//---------------------------------------------------------------------------
const String& DBException::getErrMsg() {
   return sErrMsg; 
}
//---------------------------------------------------------------------------
const String& DBException::getSQLState() {
   return sSQLState; 
}
//---------------------------------------------------------------------------
const String& DBException::getSQLMsg() {
   return sSQLMsg; 
}
//---------------------------------------------------------------------------
const String& DBException::twhat() throw()
{
   SgOStringStream oErrMsg;
   oErrMsg << _T("Exception type: ") << sExType << std::endl;
   oErrMsg << _T("Method: ") << sMethod << std::endl;
   oErrMsg << _T("Error Message: ") << sErrMsg << std::endl;
   oErrMsg << _T("SQL State: ") << sSQLState << std::endl;
   oErrMsg << _T("Native Error: ") << nSQLErr << std::endl;
   oErrMsg << _T("Native Message: ") << sSQLMsg << std::endl;
   oErrMsg << std::ends;

   // this gymnastics is needed so result isn't destroyed
   // paste these two lines into all what() code
   whatbuf = oErrMsg.str();

   return whatbuf;
}
//---------------------------------------------------------------------------
void DBException::clear()
{
   this->sMethod.clear();
   this->sErrMsg.clear();
   this->sSQLState.clear();
   this->sSQLMsg.clear();
   this->nSQLErr = 0;
}
//---------------------------------------------------------------------------
void ClearException() {
   oException.clear();
}
//---------------------------------------------------------------------------
void SetExceptionInfo(const String& sMethod, const String& sErrMsg,
      const String& sSQLState,const String& sSQLMsg, SQLINTEGER nSQLErr) {
   oException.sMethod = sMethod;
   oException.sErrMsg = sErrMsg;
   oException.sSQLState = sSQLState;
   oException.sSQLMsg = sSQLMsg;
   oException.nSQLErr = nSQLErr;
}
//---------------------------------------------------------------------------
void ExceptionByHandle(SQLSMALLINT nHandleType, SQLHANDLE hHandle,
      const String& sMethod, const String& sErrMsg)
{
   SQLRETURN nRetcode;
   TCHAR cSqlState[6] = {0};
   SQLINTEGER nSQLErr;
   TCHAR cSqlMsg[SQL_MAX_MESSAGE_LENGTH] = {0};

   nRetcode = SQLGetDiagRec(nHandleType, hHandle, 1, (SQLTCHAR*)cSqlState, &nSQLErr, (SQLTCHAR*)cSqlMsg, SQL_MAX_MESSAGE_LENGTH, NULL);
  
   if (nRetcode == SQL_SUCCESS || nRetcode == SQL_SUCCESS_WITH_INFO) {
      SetExceptionInfo(sMethod,sErrMsg,(TCHAR*)cSqlState,(TCHAR*)cSqlMsg,nSQLErr);
   } else if (nRetcode == SQL_INVALID_HANDLE) {
      SetExceptionInfo(sMethod,sErrMsg,(TCHAR*)cSqlState,_T("Ocorreu um erro de identificador inválido!"),nSQLErr);
   } else {
      SetExceptionInfo(sMethod,sErrMsg,(TCHAR*)cSqlState,_T("Ocorreu um erro indeterminado!"),nSQLErr);
   }   
}
//---------------------------------------------------------------------------
TypeTranslation::TypeTranslation(DataType eDataType, SQLSMALLINT nSQLType,
      SQLSMALLINT nNativeType, DataKind eDataKind, SQLULEN nNativeSize)
{
   this->eDataType = eDataType;
   this->nSQLType = nSQLType;
   this->nNativeType = nNativeType;
   this->eDataKind = eDataKind;
   this->nNativeSize = nNativeSize;
}
//---------------------------------------------------------------------------
DataType GetTypeBySQL(SQLSMALLINT nSQLType, bool bUnsigned)
{
   switch(nSQLType) {
      case SQL_CHAR          : return TYPE_CHAR         ;
      case SQL_VARCHAR       : return TYPE_VARCHAR      ;
      case SQL_LONGVARCHAR   : return TYPE_LONGVARCHAR  ;
      case SQL_DECIMAL       : return TYPE_DECIMAL      ;
      case SQL_NUMERIC       : return TYPE_NUMERIC      ;
      case SQL_WCHAR         : return TYPE_WCHAR        ;
      case SQL_WVARCHAR      : return TYPE_WVARCHAR     ;
      case SQL_WLONGVARCHAR  : return TYPE_WLONGVARCHAR ;
      case SQL_BIT           : return TYPE_BIT          ;
      case SQL_TINYINT       : return (bUnsigned) ? TYPE_UTINYINT  : TYPE_STINYINT ;
      case SQL_SMALLINT      : return (bUnsigned) ? TYPE_USMALLINT : TYPE_SSMALLINT;
      case SQL_INTEGER       : return (bUnsigned) ? TYPE_UINTEGER  : TYPE_SINTEGER ;
      case SQL_BIGINT        : return (bUnsigned) ? TYPE_UBIGINT   : TYPE_SBIGINT  ;
      case SQL_REAL          : return TYPE_REAL         ;
      case SQL_DOUBLE        : return TYPE_DOUBLE       ;
      case SQL_FLOAT         : return TYPE_FLOAT        ;
      case SQL_BINARY        : return TYPE_BINARY       ;
      case SQL_VARBINARY     : return TYPE_VARBINARY    ;
      case SQL_LONGVARBINARY : return TYPE_LONGVARBINARY;
      case SQL_TYPE_DATE     : return TYPE_DATE         ;
      case SQL_TYPE_TIME     : return TYPE_TIME         ;
      case SQL_TYPE_TIMESTAMP: return TYPE_TIMESTAMP    ;
   }
   return TYPE_UNKNOWN;
}
//---------------------------------------------------------------------------
//Converting Data from C to SQL Data Types
//http://msdn.microsoft.com/en-us/library/windows/desktop/ms716298(v=vs.85).aspx
//C Data Types
//http://msdn.microsoft.com/en-us/library/windows/desktop/ms714556(v=vs.85).aspx
//Mapeando os tipos de SQL para C
class TypeMap : public std::map<int, TypeTranslation*>
{
private:
   void Build();
public:
   TypeMap();
   ~TypeMap();
   TypeTranslation* getDataType(int);
};
//---------------------------------------------------------------------------
TypeMap::TypeMap() : std::map<int, TypeTranslation*>()
{
   Build();
}
//---------------------------------------------------------------------------
TypeMap::~TypeMap()
{
   TypeMap::iterator iTypes = this->begin();
   for (; iTypes != this->end(); ++iTypes)
      delete iTypes->second;
}
//---------------------------------------------------------------------------
void TypeMap::Build()
{
   (*this)[TYPE_CHAR]          = new TypeTranslation(TYPE_CHAR,          SQL_CHAR,           SQL_C_CHAR,           TYPE_PRIMITIVE, sizeof(SQLCHAR));
   (*this)[TYPE_VARCHAR]       = new TypeTranslation(TYPE_VARCHAR,       SQL_VARCHAR,        SQL_C_CHAR,           TYPE_PRIMITIVE, sizeof(SQLCHAR));
   // (*this)[TYPE_LONGVARCHAR]   = new TypeTranslation(TYPE_LONGVARCHAR,   SQL_LONGVARCHAR,    SQL_C_CHAR,           TYPE_PRIMITIVE, sizeof(SQLCHAR));
   // (*this)[TYPE_DECIMAL]       = new TypeTranslation(TYPE_DECIMAL,       SQL_DECIMAL,        SQL_C_CHAR,           TYPE_PRIMITIVE, sizeof(SQLDECIMAL));
   // (*this)[TYPE_NUMERIC]       = new TypeTranslation(TYPE_NUMERIC,       SQL_NUMERIC,        SQL_C_CHAR,           TYPE_PRIMITIVE, sizeof(SQLNUMERIC));
   (*this)[TYPE_WCHAR]         = new TypeTranslation(TYPE_WCHAR,         SQL_WCHAR,          SQL_C_WCHAR,          TYPE_PRIMITIVE, sizeof(SQLWCHAR));
   (*this)[TYPE_WVARCHAR]      = new TypeTranslation(TYPE_WVARCHAR,      SQL_WVARCHAR,       SQL_C_WCHAR,          TYPE_PRIMITIVE, sizeof(SQLWCHAR));
   // (*this)[TYPE_WLONGVARCHAR]  = new TypeTranslation(TYPE_WLONGVARCHAR,  SQL_WLONGVARCHAR,   SQL_C_WCHAR,          TYPE_PRIMITIVE, sizeof(SQLWCHAR));
   (*this)[TYPE_BIT]           = new TypeTranslation(TYPE_BIT,           SQL_BIT,            SQL_C_BIT,            TYPE_PRIMITIVE, sizeof(SQLCHAR));
   (*this)[TYPE_STINYINT]      = new TypeTranslation(TYPE_STINYINT,      SQL_TINYINT,        SQL_C_STINYINT,       TYPE_PRIMITIVE, sizeof(SQLSCHAR)); //Signed
   (*this)[TYPE_UTINYINT]      = new TypeTranslation(TYPE_UTINYINT,      SQL_TINYINT,        SQL_C_UTINYINT,       TYPE_PRIMITIVE, sizeof(SQLCHAR));  //Unsigned
   (*this)[TYPE_SSMALLINT]     = new TypeTranslation(TYPE_SSMALLINT,     SQL_SMALLINT,       SQL_C_SSHORT,         TYPE_PRIMITIVE, sizeof(SQLSMALLINT));  //Signed
   (*this)[TYPE_USMALLINT]     = new TypeTranslation(TYPE_USMALLINT,     SQL_SMALLINT,       SQL_C_USHORT,         TYPE_PRIMITIVE, sizeof(SQLUSMALLINT)); //Unsigned
   (*this)[TYPE_SINTEGER]      = new TypeTranslation(TYPE_SINTEGER,      SQL_INTEGER,        SQL_C_SLONG,          TYPE_PRIMITIVE, sizeof(SQLINTEGER));  //Signed
   (*this)[TYPE_UINTEGER]      = new TypeTranslation(TYPE_UINTEGER,      SQL_INTEGER,        SQL_C_ULONG,          TYPE_PRIMITIVE, sizeof(SQLUINTEGER)); //Unsigned
   (*this)[TYPE_SBIGINT]       = new TypeTranslation(TYPE_SBIGINT,       SQL_BIGINT,         SQL_C_SBIGINT,        TYPE_PRIMITIVE, sizeof(SQLBIGINT));  //Signed
   (*this)[TYPE_UBIGINT]       = new TypeTranslation(TYPE_UBIGINT,       SQL_BIGINT,         SQL_C_UBIGINT,        TYPE_PRIMITIVE, sizeof(SQLUBIGINT)); //Unsigned
   (*this)[TYPE_REAL]          = new TypeTranslation(TYPE_REAL,          SQL_REAL,           SQL_C_FLOAT,          TYPE_PRIMITIVE, sizeof(SQLREAL));
   (*this)[TYPE_DECIMAL]       = new TypeTranslation(TYPE_DECIMAL,       SQL_DECIMAL,        SQL_C_DOUBLE,         TYPE_PRIMITIVE, sizeof(SQLDOUBLE));
   (*this)[TYPE_NUMERIC]       = new TypeTranslation(TYPE_NUMERIC,       SQL_NUMERIC,        SQL_C_DOUBLE,         TYPE_PRIMITIVE, sizeof(SQLDOUBLE));
   (*this)[TYPE_DOUBLE]        = new TypeTranslation(TYPE_DOUBLE,        SQL_DOUBLE,         SQL_C_DOUBLE,         TYPE_PRIMITIVE, sizeof(SQLDOUBLE));
   (*this)[TYPE_FLOAT]         = new TypeTranslation(TYPE_FLOAT,         SQL_FLOAT,          SQL_C_DOUBLE,         TYPE_PRIMITIVE, sizeof(SQLFLOAT));
   (*this)[TYPE_BINARY]        = new TypeTranslation(TYPE_BINARY,        SQL_BINARY,         SQL_C_BINARY,         TYPE_PRIMITIVE, sizeof(SQLCHAR));
   (*this)[TYPE_VARBINARY]     = new TypeTranslation(TYPE_VARBINARY,     SQL_VARBINARY,      SQL_C_BINARY,         TYPE_PRIMITIVE, sizeof(SQLCHAR));
   // (*this)[TYPE_LONGVARBINARY] = new TypeTranslation(TYPE_LONGVARBINARY, SQL_LONGVARBINARY,  SQL_C_BINARY,         TYPE_PRIMITIVE, sizeof(SQLCHAR));
   (*this)[TYPE_DATE]          = new TypeTranslation(TYPE_DATE,          SQL_TYPE_DATE,      SQL_C_TYPE_DATE,      TYPE_PRIMITIVE, sizeof(SQL_DATE_STRUCT));
   (*this)[TYPE_TIME]          = new TypeTranslation(TYPE_TIME,          SQL_TYPE_TIME,      SQL_C_TYPE_TIME,      TYPE_PRIMITIVE, sizeof(SQL_TIME_STRUCT));
   (*this)[TYPE_TIMESTAMP]     = new TypeTranslation(TYPE_TIMESTAMP,     SQL_TYPE_TIMESTAMP, SQL_C_TYPE_TIMESTAMP, TYPE_PRIMITIVE, sizeof(SQL_TIMESTAMP_STRUCT));

   (*this)[TYPE_STRINGA]       = new TypeTranslation(TYPE_STRINGA,       SQL_VARCHAR,        SQL_C_CHAR,           TYPE_COMPLEX, sizeof(AnsiString));
   (*this)[TYPE_LONGVARCHAR]   = new TypeTranslation(TYPE_LONGVARCHAR,   SQL_LONGVARCHAR,    SQL_C_CHAR,           TYPE_COMPLEX, sizeof(AnsiString));
   (*this)[TYPE_STRINGW]       = new TypeTranslation(TYPE_STRINGW,       SQL_WVARCHAR,       SQL_C_WCHAR,          TYPE_COMPLEX, sizeof(WideString));
   (*this)[TYPE_WLONGVARCHAR]  = new TypeTranslation(TYPE_WLONGVARCHAR,  SQL_WLONGVARCHAR,   SQL_C_WCHAR,          TYPE_COMPLEX, sizeof(WideString));
   (*this)[TYPE_BLOB]          = new TypeTranslation(TYPE_BLOB,          SQL_VARBINARY,      SQL_C_BINARY,         TYPE_COMPLEX, sizeof(Blob));
   (*this)[TYPE_LONGVARBINARY] = new TypeTranslation(TYPE_LONGVARBINARY, SQL_LONGVARBINARY,  SQL_C_BINARY,         TYPE_COMPLEX, sizeof(Blob));
}
//---------------------------------------------------------------------------
TypeTranslation* TypeMap::getDataType(int nIndex)
{
   return (*this)[nIndex];
}
//---------------------------------------------------------------------------
static TypeMap oTypeMaps;
//---------------------------------------------------------------------------
TypeTranslation* GetMapType(DataType eDataType)
{
   return oTypeMaps.getDataType(eDataType);
}
//---------------------------------------------------------------------------
class StrTypeMap : public std::map<AnsiString, TypeTranslation*>
{
private:
   void Build();
public:
   StrTypeMap();
   ~StrTypeMap();
   TypeTranslation* getDataType(const AnsiString& sStrType);
};
//---------------------------------------------------------------------------
StrTypeMap::StrTypeMap() : std::map<AnsiString, TypeTranslation*>()
{
   Build();
}
//---------------------------------------------------------------------------
StrTypeMap::~StrTypeMap()
{
   StrTypeMap::iterator iTypes = this->begin();
   for (; iTypes != this->end(); ++iTypes)
      delete iTypes->second;
}
//---------------------------------------------------------------------------
void StrTypeMap::Build()
{
   (*this)[typeid(SQLCHAR).name()]              = new TypeTranslation(TYPE_CHAR,          SQL_CHAR,           SQL_C_CHAR,           TYPE_PRIMITIVE, sizeof(SQLCHAR));
   (*this)[typeid(SQLWCHAR).name()]             = new TypeTranslation(TYPE_WCHAR,         SQL_WCHAR,          SQL_C_WCHAR,          TYPE_PRIMITIVE, sizeof(SQLWCHAR));
   (*this)[typeid(SQLSCHAR).name()]             = new TypeTranslation(TYPE_STINYINT,      SQL_TINYINT,        SQL_C_STINYINT,       TYPE_PRIMITIVE, sizeof(SQLSCHAR)); //Signed
   (*this)[typeid(SQLSMALLINT).name()]          = new TypeTranslation(TYPE_SSMALLINT,     SQL_SMALLINT,       SQL_C_SSHORT,         TYPE_PRIMITIVE, sizeof(SQLSMALLINT));  //Signed
   (*this)[typeid(SQLUSMALLINT).name()]         = new TypeTranslation(TYPE_USMALLINT,     SQL_SMALLINT,       SQL_C_USHORT,         TYPE_PRIMITIVE, sizeof(SQLUSMALLINT)); //Unsigned
   (*this)[typeid(SQLINTEGER).name()]           = new TypeTranslation(TYPE_SINTEGER,      SQL_INTEGER,        SQL_C_SLONG,          TYPE_PRIMITIVE, sizeof(SQLINTEGER));  //Signed
   (*this)[typeid(SQLUINTEGER).name()]          = new TypeTranslation(TYPE_UINTEGER,      SQL_INTEGER,        SQL_C_ULONG,          TYPE_PRIMITIVE, sizeof(SQLUINTEGER)); //Unsigned
   (*this)[typeid(SQLBIGINT).name()]            = new TypeTranslation(TYPE_SBIGINT,       SQL_BIGINT,         SQL_C_SBIGINT,        TYPE_PRIMITIVE, sizeof(SQLBIGINT));  //Signed
   (*this)[typeid(SQLUBIGINT).name()]           = new TypeTranslation(TYPE_UBIGINT,       SQL_BIGINT,         SQL_C_UBIGINT,        TYPE_PRIMITIVE, sizeof(SQLUBIGINT)); //Unsigned
   (*this)[typeid(SQLREAL).name()]              = new TypeTranslation(TYPE_REAL,          SQL_REAL,           SQL_C_FLOAT,          TYPE_PRIMITIVE, sizeof(SQLREAL));
   (*this)[typeid(SQLDOUBLE).name()]            = new TypeTranslation(TYPE_DOUBLE,        SQL_DOUBLE,         SQL_C_DOUBLE,         TYPE_PRIMITIVE, sizeof(SQLDOUBLE));
   (*this)[typeid(SQL_DATE_STRUCT).name()]      = new TypeTranslation(TYPE_DATE,          SQL_TYPE_DATE,      SQL_C_TYPE_DATE,      TYPE_PRIMITIVE, sizeof(SQL_DATE_STRUCT));
   (*this)[typeid(SQL_TIME_STRUCT).name()]      = new TypeTranslation(TYPE_TIME,          SQL_TYPE_TIME,      SQL_C_TYPE_TIME,      TYPE_PRIMITIVE, sizeof(SQL_TIME_STRUCT));
   (*this)[typeid(SQL_TIMESTAMP_STRUCT).name()] = new TypeTranslation(TYPE_TIMESTAMP,     SQL_TYPE_TIMESTAMP, SQL_C_TYPE_TIMESTAMP, TYPE_PRIMITIVE, sizeof(SQL_TIMESTAMP_STRUCT));

   (*this)[typeid(AnsiString).name()]           = new TypeTranslation(TYPE_STRINGA,       SQL_VARCHAR,        SQL_C_CHAR,           TYPE_COMPLEX, sizeof(AnsiString));
   (*this)[typeid(WideString).name()]           = new TypeTranslation(TYPE_STRINGW,       SQL_WVARCHAR,       SQL_C_WCHAR,          TYPE_COMPLEX, sizeof(WideString));
   (*this)[typeid(Blob).name()]                 = new TypeTranslation(TYPE_BLOB,          SQL_VARBINARY,      SQL_C_BINARY,         TYPE_COMPLEX, sizeof(Blob));
}
//---------------------------------------------------------------------------
TypeTranslation* StrTypeMap::getDataType(const AnsiString& sStrType)
{
   return (*this)[sStrType];
}
//---------------------------------------------------------------------------
static StrTypeMap oStrTypeMaps;
//---------------------------------------------------------------------------
TypeTranslation* GetStrMapType(const AnsiString& sStrType)
{
   return oStrTypeMaps.getDataType(sStrType);
}
//---------------------------------------------------------------------------
DBObject::DBObject() : IDBObject()
{
   this->pTag = NULL;
   this->nRefCount = 1;
}
//---------------------------------------------------------------------------
void DBObject::setTag(void* Value) {
   this->pTag = Value;
}
//---------------------------------------------------------------------------
void* DBObject::getTag() {
   return this->pTag;
}
//---------------------------------------------------------------------------
void DBObject::AddRef() {
   this->nRefCount++;
}
//---------------------------------------------------------------------------
void DBObject::Release() {
   this->nRefCount--;
   if (this->nRefCount == 0)
      delete this;
}
//---------------------------------------------------------------------------
DBBind::DBBind(BindType eBindType, SQLSMALLINT nBindNum) :
      DBObject(), IDBBind()
{
   SgOStringStream oBindName;

   if (eBindType == BIND_COLUMN)
      oBindName << _T("Field") << nBindNum;
   else
      oBindName << _T("Param") << nBindNum;

   this->bAutoGenerate = true;
   this->oType = NULL;
   InternalInit(eBindType, nBindNum, oBindName.str(), 0, 0, false);
}
//---------------------------------------------------------------------------
DBBind::DBBind(BindType eBindType, SQLSMALLINT nBindNum, const String &sBindName,
         DataType eDataType, SQLULEN nDataSize, SQLSMALLINT nDecimalDigits, bool bNotNull) :
         DBObject(), IDBBind()
{
   this->bAutoGenerate = false;
   this->oType = oTypeMaps.getDataType(eDataType);
   InternalInit(eBindType, nBindNum, sBindName, nDataSize, nDecimalDigits, bNotNull);
   if (this->oType != NULL)
      AllocData();
   if (eDataType == TYPE_UTINYINT  ||
       eDataType == TYPE_USMALLINT ||
       eDataType == TYPE_UINTEGER  ||
       eDataType == TYPE_UBIGINT) {
      this->bUnsigned = true;
   }
}
//---------------------------------------------------------------------------
DBBind::DBBind(BindType eBindType, SQLSMALLINT nBindNum, const String &sBindName, SQLPOINTER pPointer,
         DataType eDataType, SQLULEN nDataSize, SQLSMALLINT nDecimalDigits, bool bNotNull) :
         DBObject(), IDBBind()
{
   SQLLEN nBufferLen = nDataSize;

   this->bAutoGenerate = false;
   this->oType = oTypeMaps.getDataType(eDataType);
   InternalInit(eBindType, nBindNum, sBindName, nDataSize, nDecimalDigits, bNotNull);
   if (this->oType != NULL) {
      this->bInternalValue = false;
      this->pPointer = pPointer;
   }
   if (eDataType == TYPE_UTINYINT  ||
       eDataType == TYPE_USMALLINT ||
       eDataType == TYPE_UINTEGER  ||
       eDataType == TYPE_UBIGINT) {
      this->bUnsigned = true;
   }
   GetLenForBind(nBufferLen);
   if (nBufferLen > 0) {
      switch (eDataType) {
         case TYPE_CHAR:
         case TYPE_VARCHAR:
         case TYPE_WCHAR:
         case TYPE_WVARCHAR:
            this->nRetLength = SQL_NTS;
            break;
         case TYPE_BINARY:
         case TYPE_VARBINARY:
            this->nRetLength = (oType->getNativeSize() * nDataSize);
            break;
         default:
            this->nRetLength = oType->getNativeSize();
      }
   } else {
      this->nRetLength = SQL_LEN_DATA_AT_EXEC(0);
   }
}
//---------------------------------------------------------------------------
DBBind::~DBBind()
{
   ReleasePointer();
}
//---------------------------------------------------------------------------
void DBBind::InternalInit(BindType eBindType, SQLSMALLINT nBindNum, const String &sBindName,
         SQLULEN nDataSize, SQLSMALLINT nDecimalDigits, bool bNotNull)
{
   this->eBindType = eBindType;
   this->nBindNum = nBindNum;
   this->sBindName = sBindName;
   this->nDataSize = nDataSize;
   this->nDecimalDigits = nDecimalDigits;
   this->bNotNull = bNotNull;
   this->nRetLength = SQL_NULL_DATA;
   this->nRetcode = SQL_SUCCESS;
   this->bIsBind = false;
   this->pPointer = NULL;
   this->bUnsigned = false;
   this->bInternalValue = true;
   this->AfterSetValue = NULL;
   //Transformando o nome do campo sempre para minusculo
   std::transform(this->sBindName.begin(), this->sBindName.end(), this->sBindName.begin(), ::_totlower);
}
//---------------------------------------------------------------------------
void DBBind::AllocData()
{
   size_t nSize = 0;

   this->nRetLength = SQL_NULL_DATA;

   switch(oType->getDataType()) {
      case TYPE_CHAR:
      case TYPE_WCHAR:
      case TYPE_VARCHAR:
      case TYPE_WVARCHAR:
         nSize = oType->getNativeSize() * (nDataSize + 1);
         this->pPointer = malloc(nSize);
         break;
      case TYPE_STRINGA:
      case TYPE_LONGVARCHAR:
         this->pPointer = (SQLPOINTER) new AnsiString();
         break;
      case TYPE_STRINGW:
      case TYPE_WLONGVARCHAR:
         this->pPointer = (SQLPOINTER) new WideString();
         break;
      case TYPE_BLOB:
      case TYPE_LONGVARBINARY:
         this->pPointer = (SQLPOINTER) new Blob();
         break;
      default:
         nSize = oType->getNativeSize();
         this->pPointer = malloc(nSize);
   }
}
//---------------------------------------------------------------------------
void DBBind::ReleasePointer()
{
   this->nRetLength = SQL_NULL_DATA;
   //Se o ponteiro não é interno, não exclui
   if (!bInternalValue)
      return;

   //Limpando a memória alocada para o parâmetro
   if (this->oType != NULL && this->oType->IsComplex()) {
      //Deleta o ponteiro
      switch(oType->getDataType()) {
         case TYPE_STRINGA:
         case TYPE_LONGVARCHAR:
            delete reinterpret_cast<AnsiString*>(pPointer);
            break;
         case TYPE_STRINGW:
         case TYPE_WLONGVARCHAR:
            delete reinterpret_cast<WideString*>(pPointer);
            break;
         case TYPE_BLOB:
         case TYPE_LONGVARBINARY:
            delete reinterpret_cast<Blob*>(pPointer);
            break;
         default:
            break;
      }
   } else {
      if (pPointer != NULL)
         free(pPointer);
   }
}
//---------------------------------------------------------------------------
void DBBind::GetLenForBind(SQLLEN &nBufferLen)
{
   switch(oType->getDataType()) {
      case TYPE_STRINGA:
      case TYPE_STRINGW:
      case TYPE_BLOB:
      case TYPE_LONGVARCHAR:
      case TYPE_WLONGVARCHAR:
      case TYPE_LONGVARBINARY:
         nBufferLen = 0;
         break;
      case TYPE_CHAR:
      case TYPE_VARCHAR:
      case TYPE_WCHAR:
      case TYPE_WVARCHAR:
         nBufferLen = oType->getNativeSize() * (nDataSize + 1);
         break;
      case TYPE_BINARY:
      case TYPE_VARBINARY:
         nBufferLen = oType->getNativeSize() * nDataSize;
         break;
      default:
         nBufferLen = oType->getNativeSize();
   }
}
//---------------------------------------------------------------------------
SQLRETURN DBBind::PutData(SQLHSTMT hStmt)
{
   SQLLEN StrLenOrIndPtr = 0;
   switch(oType->getDataType()) {
      case TYPE_STRINGA:
      case TYPE_LONGVARCHAR:
         StrLenOrIndPtr = reinterpret_cast<AnsiString*>(pPointer)->length() * sizeof(SQLCHAR);
         nRetcode = SQLPutData(hStmt, (SQLPOINTER)reinterpret_cast<AnsiString*>(pPointer)->data(), StrLenOrIndPtr);
         break;
      case TYPE_STRINGW:
      case TYPE_WLONGVARCHAR:
         StrLenOrIndPtr = reinterpret_cast<WideString*>(pPointer)->length() * sizeof(SQLWCHAR);
         nRetcode = SQLPutData(hStmt, (SQLPOINTER)reinterpret_cast<WideString*>(pPointer)->data(), StrLenOrIndPtr);
         break;
      case TYPE_BLOB:
      case TYPE_LONGVARBINARY:
         StrLenOrIndPtr = reinterpret_cast<Blob*>(pPointer)->length() * sizeof(SQLCHAR);
         nRetcode = SQLPutData(hStmt, (SQLPOINTER)reinterpret_cast<Blob*>(pPointer)->data(), StrLenOrIndPtr);
         break;
      default:
         StrLenOrIndPtr = oType->getNativeSize() * nDataSize;
         nRetcode = SQLPutData(hStmt, pPointer, StrLenOrIndPtr);
   }
   return nRetcode;
}
//---------------------------------------------------------------------------
SQLRETURN DBBind::GetData(SQLHSTMT hStmt)
{
   SQLLEN StrLenOrIndPtr = 0;
   switch(oType->getDataType()) {
      case TYPE_STRINGA:
      case TYPE_LONGVARCHAR:
      {
         AnsiString *sStrAnsi = reinterpret_cast<AnsiString*>(pPointer);
         sStrAnsi->clear();
         sStrAnsi->reserve(nRetLength);
         nRetcode = SQLGetData(hStmt, nBindNum, oType->getNativeType(), (SQLPOINTER)sStrAnsi->data(), nDataSize * sizeof(char), &StrLenOrIndPtr);
         (*sStrAnsi)[nRetLength] = '\0';
      }
      break;
      case TYPE_BLOB:
      case TYPE_LONGVARBINARY:
      {
         Blob* oBlob = reinterpret_cast<Blob*>(pPointer);
         oBlob->reserve(nRetLength);
         nRetcode = SQLGetData(hStmt, nBindNum, oType->getNativeType(), (SQLPOINTER)&oBlob[0], nDataSize * sizeof(BYTE), &StrLenOrIndPtr);
      }
      break;
      case TYPE_STRINGW:
      case TYPE_WLONGVARCHAR:
      {
         WideString *sStrWide = reinterpret_cast<WideString*>(pPointer);
         sStrWide->clear();
         sStrWide->reserve(nRetLength / sizeof(WCHAR));
         nRetcode = SQLGetData(hStmt, nBindNum, oType->getNativeType(), (SQLPOINTER)sStrWide->data(), nDataSize * sizeof(WCHAR), &StrLenOrIndPtr);
        (*sStrWide)[(nRetLength / sizeof(WCHAR))] = L'\0';
      }
      break;
      default:
         nRetcode = SQLGetData(hStmt, nBindNum, oType->getNativeType(), (SQLPOINTER)pPointer, nDataSize * oType->getNativeSize(), &StrLenOrIndPtr);
   }
   return nRetcode;
}
//---------------------------------------------------------------------------
BindType DBBind::getBindType() {
   return eBindType;
}
//---------------------------------------------------------------------------
DataType DBBind::getDataType() {
   return oType->getDataType();
}
//---------------------------------------------------------------------------
SQLSMALLINT DBBind::getSQLType() {
   return oType->getSQLType();
}
//---------------------------------------------------------------------------
SQLSMALLINT DBBind::getNativeType() {
   return oType->getNativeType();
}
//---------------------------------------------------------------------------
SQLULEN DBBind::getNativeSize() {
   return oType->getNativeSize();
}
//---------------------------------------------------------------------------
SQLULEN DBBind::getDataSize() {
   return nDataSize;
}
//---------------------------------------------------------------------------
SQLSMALLINT DBBind::getDecimalDigits() {
   return nDecimalDigits;
}
//---------------------------------------------------------------------------
bool DBBind::getNotNull() {
   return bNotNull;
}
//---------------------------------------------------------------------------
SQLPOINTER DBBind::getPointer() {
   return pPointer;
}
//---------------------------------------------------------------------------
bool DBBind::IsBind()
{
   return bIsBind;
}
//---------------------------------------------------------------------------
SQLRETURN DBBind::Bind()
{
   bIsBind = true;
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN DBBind::Unbind()
{
   bIsBind = false;
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
bool DBBind::IsNull()
{
   return (nRetLength == SQL_NULL_DATA);
};
//---------------------------------------------------------------------------
SQLRETURN DBBind::setDataType(const DataType Value)
{
   if (bIsBind) {
      TCHAR cErrMsg[SHORT_MSG] = {0};
      if (eBindType == BIND_PARAMETER)
         SgFormatMsg(cErrMsg,SHORT_MSG,_T("Unable set data type after bind parameter. Name: %s Number: %d"),sBindName,nBindNum);
      else
         SgFormatMsg(cErrMsg,SHORT_MSG,_T("Unable set data type after bind column. Name: %s Number: %d"),sBindName,nBindNum);

      SetExceptionInfo(_T("DBBind::setDataType"),cErrMsg,_T("HY011"),_T("Attribute cannot be set now"),0);
      return SQL_ERROR;
   }

   if (oType->getDataType() == Value)
      return SQL_SUCCESS;

   if (bInternalValue)
      ReleasePointer();

   this->oType = oTypeMaps.getDataType(Value);

   if (bInternalValue)
      AllocData();
   
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN DBBind::setDataSize(const SQLULEN Value)
{
   if (bIsBind) {
      TCHAR cErrMsg[SHORT_MSG] = {0};
      if (eBindType == BIND_PARAMETER)
         SgFormatMsg(cErrMsg,SHORT_MSG,_T("Unable set data size after bind parameter. Name: %s Number: %d"),sBindName,nBindNum);
      else
         SgFormatMsg(cErrMsg,SHORT_MSG,_T("Unable set data size after bind column. Name: %s Number: %d"),sBindName,nBindNum);

      SetExceptionInfo(_T("DBBind::setDataSize"),cErrMsg,_T("HY011"),_T("Attribute cannot be set now"),0);
      return SQL_ERROR;
   }
   nDataSize = Value;
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN DBBind::setDecimalDigits(SQLSMALLINT Value)
{
   if (bIsBind && eBindType == BIND_PARAMETER) {
      TCHAR cErrMsg[SHORT_MSG] = {0};
      if (eBindType == BIND_PARAMETER)
         SgFormatMsg(cErrMsg,SHORT_MSG,_T("Unable set decimal digits after bind parameter. Name: %s Number: %d"),sBindName,nBindNum);
      else
         SgFormatMsg(cErrMsg,SHORT_MSG,_T("Unable set decimal digits after bind column. Name: %s Number: %d"),sBindName,nBindNum);

      SetExceptionInfo(_T("DBBind::setDecimalDigits"),cErrMsg,_T("HY011"),_T("Attribute cannot be set now"),0);
      return SQL_ERROR;
   }
   nDecimalDigits = Value;
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN DBBind::setNotNull(bool Value)
{
   bNotNull = Value;
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN DBBind::SetNull()
{
   if (bNotNull) {
      TCHAR cErrMsg[NORMAL_MSG] = {0};
      if (eBindType == BIND_PARAMETER)
         SgFormatMsg(cErrMsg,NORMAL_MSG,_T("Unable set NULL value for parameter, because is set to not nul. Name: %s Number: %d"),sBindName,nBindNum);
      else
         SgFormatMsg(cErrMsg,NORMAL_MSG,_T("Unable set NULL value for column, because is set to not nul. Name: %s Number: %d"),sBindName,nBindNum);

      SetExceptionInfo(_T("DBBind::setDecimalDigits"),cErrMsg,_T("HY009"),_T("Invalid use of null pointer"),0);
      return SQL_ERROR;
   }
   nRetLength = SQL_NULL_DATA;
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
void DBBind::SetAfterSetValue(BindNotify pValue) {
   this->AfterSetValue = pValue;
}
//---------------------------------------------------------------------------
SQLRETURN DBBind::SetValue(const char* Value, size_t nLength)
{
   TCHAR cErrMsg[SHORT_MSG] = {0};
   char* cStrAnsi = NULL;
   WCHAR* cStrWide = NULL;

   if (nLength > nDataSize) {
      SgFormatMsg(cErrMsg,SHORT_MSG,_T("Data type suporte max %d data length. Informed %d data length."),nDataSize,nLength);
      SetExceptionInfo(_T("DBBind::SetValue(char*)"),cErrMsg,_T("HY090"),_T("Invalid string or buffer length"),0);
      return SQL_ERROR;
   }

   switch(oType->getDataType()) {
      case TYPE_CHAR:
      case TYPE_VARCHAR:
         cStrAnsi = reinterpret_cast<char*>(pPointer);
         strncpy(cStrAnsi, Value, nLength);
         cStrAnsi[nLength] = '\0';
         nRetLength = SQL_NTS;
         break;
      case TYPE_BINARY:
      case TYPE_VARBINARY:
         strncpy((char*)pPointer, Value, nLength);
         nRetLength = (oType->getNativeSize() * nLength);
         break;
      case TYPE_WCHAR:
      case TYPE_WVARCHAR:
         //Deve transformar o char* em wchar_t*
         cStrWide = reinterpret_cast<WCHAR*>(pPointer);
         mbstowcs (cStrWide, Value, nLength);
         cStrWide[nLength] = L'\0';
         nRetLength = SQL_NTS;
         break;
      case TYPE_STRINGA:
      case TYPE_LONGVARCHAR:
         reinterpret_cast<AnsiString*>(pPointer)->assign(Value);
         nRetLength = SQL_LEN_DATA_AT_EXEC(0);
         break;
      case TYPE_STRINGW:
      case TYPE_WLONGVARCHAR:
         cStrWide = new WCHAR[nLength+1];
         mbstowcs (cStrWide, Value, nLength);
         cStrWide[nLength] = L'\0';
         reinterpret_cast<WideString*>(pPointer)->assign(cStrWide);
         delete[] cStrWide;
         nRetLength = SQL_LEN_DATA_AT_EXEC(0);
         break;
      case TYPE_BLOB:
      case TYPE_LONGVARBINARY:
         reinterpret_cast<Blob*>(pPointer)->assign((BYTE*)Value);
         nRetLength = SQL_LEN_DATA_AT_EXEC(0);
         break;
      case TYPE_STINYINT :
      case TYPE_UTINYINT :
      case TYPE_USMALLINT:
      case TYPE_SSMALLINT:
      case TYPE_UINTEGER :
      case TYPE_SINTEGER :
      case TYPE_UBIGINT  :
      case TYPE_SBIGINT  :
      { 
         long nValue = strtol(Value,NULL,10);
         return SetValue((long)nValue);
      }
      case TYPE_REAL   :
      case TYPE_DECIMAL:
      case TYPE_NUMERIC:
      case TYPE_DOUBLE :
      case TYPE_FLOAT  :
      {
         double nValue = strtod(Value,NULL);
         return SetValue((double)nValue,nDecimalDigits);
      }
      case TYPE_DATE     :
      {
         SQL_DATE_STRUCT *dDate = reinterpret_cast<SQL_DATE_STRUCT*>(pPointer);
         sscanf(Value,STRA_DATE_FMT,&dDate->year,&dDate->month,&dDate->day);
         nRetLength = oType->getNativeSize();
      }
      break;
      case TYPE_TIME     :
      {
         SQL_TIME_STRUCT *dTime = reinterpret_cast<SQL_TIME_STRUCT*>(pPointer);
         sscanf(Value,STRA_HOUR_FMT,&dTime->hour,&dTime->minute,&dTime->second);
         nRetLength = oType->getNativeSize();
      }
      break;
      case TYPE_TIMESTAMP:
      {
         SQL_DATETIME_STRUCT *dDateTime = reinterpret_cast<SQL_DATETIME_STRUCT*>(pPointer);
         sscanf(Value,STRA_DTTM_FMT,&dDateTime->year,&dDateTime->month,&dDateTime->day,&dDateTime->hour,&dDateTime->minute,&dDateTime->second);
         nRetLength = oType->getNativeSize();
      }
      break;
      default:
         SetExceptionInfo(_T("DBBind::SetValue(char*)"),_T("Data type don't suport SetValue by char*"),_T("HY004"),_T("Invalid SQL data type"),0);
         return SQL_ERROR;
   }
   if (AfterSetValue != NULL) {
      AfterSetValue(this);
   }
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN DBBind::SetValue(const WCHAR* Value, size_t nLength)
{
   TCHAR cErrMsg[SHORT_MSG] = {0};
   char* cStrAnsi = NULL;
   WCHAR* cStrWide = NULL;

   if (nLength > nDataSize) {
      SgFormatMsg(cErrMsg,SHORT_MSG,_T("Data type suporte max %d data length. Informed %d data length."),nDataSize,nLength);
      SetExceptionInfo(_T("DBBind::SetValue(WCHAR*)"),cErrMsg,_T("HY090"),_T("Invalid string or buffer length"),0);
      return SQL_ERROR;
   }

   switch(oType->getDataType()) {
      case TYPE_CHAR:
      case TYPE_VARCHAR:
         cStrAnsi = new char[nLength+1];
         wcstombs(cStrAnsi, Value, nLength);
         cStrAnsi[nLength] = '\0';
         strncpy(reinterpret_cast<char*>(pPointer), cStrAnsi, nLength+1);
         delete[] cStrAnsi;
         nRetLength = SQL_NTS;
         break;
      case TYPE_BINARY:
      case TYPE_VARBINARY:
         cStrAnsi = new char[nLength+1];
         wcstombs(cStrAnsi, Value, nLength);
         strncpy(reinterpret_cast<char*>(pPointer), cStrAnsi, nLength);
         delete[] cStrAnsi;
         nRetLength = (oType->getNativeSize() * nLength);
         break;
      case TYPE_WCHAR:
      case TYPE_WVARCHAR:
         cStrWide = reinterpret_cast<WCHAR*>(pPointer);
         wcsncpy(cStrWide, Value, nLength);
         cStrWide[nLength] = L'\0';
         nRetLength = SQL_NTS;
         break;
      case TYPE_STRINGA:
      case TYPE_LONGVARCHAR:
         cStrAnsi = new char[nLength+1];
         wcstombs(cStrAnsi, Value, nLength);
         cStrAnsi[nLength] = '\0';
         reinterpret_cast<AnsiString*>(pPointer)->assign(cStrAnsi);
         delete[] cStrAnsi;
         nRetLength = SQL_LEN_DATA_AT_EXEC(0);
         break;
      case TYPE_STRINGW:
      case TYPE_WLONGVARCHAR:
         reinterpret_cast<WideString*>(pPointer)->assign(Value);
         nRetLength = SQL_LEN_DATA_AT_EXEC(0);
         break;
      case TYPE_BLOB:
      case TYPE_LONGVARBINARY:
         cStrAnsi = new char[nLength+1];
         wcstombs(cStrAnsi, Value, nLength);
         reinterpret_cast<Blob*>(pPointer)->assign(reinterpret_cast<BYTE*>(cStrAnsi));
         delete[] cStrAnsi;
         nRetLength = SQL_LEN_DATA_AT_EXEC(0);
         break;
      case TYPE_STINYINT :
      case TYPE_UTINYINT :
      case TYPE_USMALLINT:
      case TYPE_SSMALLINT:
      case TYPE_UINTEGER :
      case TYPE_SINTEGER :
      case TYPE_UBIGINT  :
      case TYPE_SBIGINT  :
      { 
         long nValue = wcstol(Value,NULL,10);
         return SetValue((long)nValue);
      }
      case TYPE_REAL   :
      case TYPE_DECIMAL:
      case TYPE_NUMERIC:
      case TYPE_DOUBLE :
      case TYPE_FLOAT  :
      {
         double nValue = wcstod(Value,NULL);
         return SetValue((double)nValue,nDecimalDigits);
      }
      case TYPE_DATE     :
      {
         SQL_DATE_STRUCT *dDate = reinterpret_cast<SQL_DATE_STRUCT*>(pPointer);
         swscanf(Value,STRW_DATE_FMT,&dDate->year,&dDate->month,&dDate->day);
         nRetLength = oType->getNativeSize();
      }
      break;
      case TYPE_TIME     :
      {
         SQL_TIME_STRUCT *dTime = reinterpret_cast<SQL_TIME_STRUCT*>(pPointer);
         swscanf(Value,STRW_HOUR_FMT,&dTime->hour,&dTime->minute,&dTime->second);
         nRetLength = oType->getNativeSize();
      }
      break;
      case TYPE_TIMESTAMP:
      {
         SQL_DATETIME_STRUCT *dDateTime = reinterpret_cast<SQL_DATETIME_STRUCT*>(pPointer);
         swscanf(Value,STRW_DTTM_FMT,&dDateTime->year,&dDateTime->month,&dDateTime->day,&dDateTime->hour,&dDateTime->minute,&dDateTime->second);
         nRetLength = oType->getNativeSize();
      }
      break;
      default:
         SetExceptionInfo(_T("DBBind::SetValue(WCHAR*)"),_T("Data type don't suport SetValue by WCHAR*"),_T("HY004"),_T("Invalid SQL data type"),0);
         return SQL_ERROR;
   }
   if (AfterSetValue != NULL) {
      AfterSetValue(this);
   }
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN DBBind::SetValue(const bool Value)
{
   char* cStrAnsi = NULL;
   WCHAR* cStrWide = NULL;

   switch(oType->getDataType()) {
      case TYPE_CHAR:
      case TYPE_VARCHAR:
         cStrAnsi = reinterpret_cast<char*>(pPointer);
         cStrAnsi[0] = (Value) ? '1' : '0';
         if (nDataSize > 1) {
            cStrAnsi[1] = '\0';
            nRetLength = SQL_NTS;
         } else {
            nRetLength = oType->getNativeSize();
         }
         break;
      case TYPE_BINARY:
      case TYPE_VARBINARY:
         cStrAnsi = reinterpret_cast<char*>(pPointer);
         cStrAnsi[0] = (Value) ? '1' : '0';
         nRetLength = oType->getNativeSize();
         break;
      case TYPE_WCHAR:
      case TYPE_WVARCHAR:
         cStrWide = reinterpret_cast<WCHAR*>(pPointer);
         cStrWide[0] = (Value) ? L'1' : L'0';
         if (nDataSize > 1) {
            cStrWide[1] = L'\0';
            nRetLength = SQL_NTS;
         } else {
            nRetLength = oType->getNativeSize();
         }
         break;
      case TYPE_STRINGA:
      case TYPE_LONGVARCHAR:
      {
         AnsiString* str = reinterpret_cast<AnsiString*>(pPointer);
         str->clear();
         str->resize(2);
         str[0] = (Value) ? '1' : '0';
         str[1] = '\0';
         nRetLength = SQL_LEN_DATA_AT_EXEC(0);
      }
         break;
      case TYPE_STRINGW:
      case TYPE_WLONGVARCHAR:
      {
         WideString* str = reinterpret_cast<WideString*>(pPointer);
         str->clear();
         str->resize(2);
         str[0] = (Value) ? L'1' : L'0';
         str[1] = L'\0';
         nRetLength = SQL_LEN_DATA_AT_EXEC(0);
      }
         break;
      case TYPE_BLOB:
      case TYPE_LONGVARBINARY:
      {
         Blob* blob = reinterpret_cast<Blob*>(pPointer);
         BYTE b = (Value) ? '1' : '0';
         blob->assign(1,b);
         nRetLength = SQL_LEN_DATA_AT_EXEC(0);
      }
         break;
      case TYPE_BIT      :
      case TYPE_STINYINT :
      case TYPE_UTINYINT :
         reinterpret_cast<char*>(pPointer)[0] = (Value) ? 1 : 0;
         nRetLength = oType->getNativeSize();
         break;
      case TYPE_SSMALLINT:
      case TYPE_USMALLINT:
         *(reinterpret_cast<short*>(pPointer)) = (Value) ? 1 : 0;
         nRetLength = oType->getNativeSize();
         break;
      case TYPE_SINTEGER :
      case TYPE_UINTEGER :
         *(reinterpret_cast<int*>(pPointer)) = (Value) ? 1 : 0;
         nRetLength = oType->getNativeSize();
         break;
      case TYPE_SBIGINT  :
      case TYPE_UBIGINT  :
         *(reinterpret_cast<__int64*>(pPointer)) = (Value) ? 1 : 0;
         nRetLength = oType->getNativeSize();
         break;
      case TYPE_REAL     :
         *(reinterpret_cast<float*>(pPointer)) = (Value) ? 1.0 : 0.0;
         nRetLength = oType->getNativeSize();
         break;
      case TYPE_DECIMAL:
      case TYPE_NUMERIC:
      case TYPE_DOUBLE :
      case TYPE_FLOAT  :
         *(reinterpret_cast<double*>(pPointer)) = (Value) ? 1.0 : 0.0;
         nRetLength = oType->getNativeSize();
         break;
      default:
         SetExceptionInfo(_T("DBBind::SetValue(bool)"),_T("Data type don't suport SetValue by bool"),_T("HY004"),_T("Invalid SQL data type"),0);
         return SQL_ERROR;
   }
   if (AfterSetValue != NULL) {
      AfterSetValue(this);
   }
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN DBBind::SetValue(const short Value)
{
   TCHAR cErrMsg[SHORT_MSG] = {0};
   char cStrAnsi[7];
   WCHAR cStrWide[7];
   SQLULEN nLength = nDataSize;

   switch(oType->getDataType()) {
      case TYPE_CHAR:
      case TYPE_VARCHAR:
         nLength = _snprintf(cStrAnsi, 7, "%hd", Value);
         if (nLength > nDataSize) {
            SgFormatMsg(cErrMsg,SHORT_MSG,_T("Data type suporte max %d data length. Informed %d data length."),nDataSize,nLength);
            SetExceptionInfo(_T("DBBind::SetValue(short)"),cErrMsg,_T("HY090"),_T("Invalid string or buffer length"),0);
            return SQL_ERROR;
         }
         strncpy(reinterpret_cast<char*>(pPointer), cStrAnsi, nLength+1);
         nRetLength = SQL_NTS;
         break;
      case TYPE_BINARY:
      case TYPE_VARBINARY:
         nLength = _snprintf(cStrAnsi, 7, "%hd", Value);
         if (nLength > nDataSize) {
            SgFormatMsg(cErrMsg,SHORT_MSG,_T("Data type suporte max %d data length. Informed %d data length."),nDataSize,nLength);
            SetExceptionInfo(_T("DBBind::SetValue(short)"),cErrMsg,_T("HY090"),_T("Invalid string or buffer length"),0);
            return SQL_ERROR;
         }
         strncpy(reinterpret_cast<char*>(pPointer), cStrAnsi, nLength);
         nRetLength = (oType->getNativeSize() * nLength);
         break;
      case TYPE_WCHAR:
      case TYPE_WVARCHAR:
         nLength = _snwprintf(cStrWide, 7, L"%hd", Value);
         if (nLength > nDataSize) {
            SgFormatMsg(cErrMsg,SHORT_MSG,_T("Data type suporte max %d data length. Informed %d data length."),nDataSize,nLength);
            SetExceptionInfo(_T("DBBind::SetValue(short)"),cErrMsg,_T("HY090"),_T("Invalid string or buffer length"),0);
            return SQL_ERROR;
         }
         wcsncpy(reinterpret_cast<WCHAR*>(pPointer), cStrWide, nLength+1);
         nRetLength = SQL_NTS;
         break;
      case TYPE_STRINGA:
      case TYPE_LONGVARCHAR:
         nLength = _snprintf(cStrAnsi, 7, "%hd", Value);
         reinterpret_cast<AnsiString*>(pPointer)->assign(cStrAnsi);
         nRetLength = SQL_LEN_DATA_AT_EXEC(0);
         break;
      case TYPE_STRINGW:
      case TYPE_WLONGVARCHAR:
         nLength = _snwprintf(cStrWide, 7, L"%hd", Value);
         reinterpret_cast<WideString*>(pPointer)->assign(cStrWide);
         nRetLength = SQL_LEN_DATA_AT_EXEC(0);
         break;
      case TYPE_BLOB:
      case TYPE_LONGVARBINARY:
         nLength = _snprintf(cStrAnsi, 7, "%hd", Value);
         reinterpret_cast<Blob*>(pPointer)->assign(reinterpret_cast<BYTE*>(cStrAnsi));
         nRetLength = SQL_LEN_DATA_AT_EXEC(0);
         break;
      case TYPE_STINYINT:
         if (Value < SCHAR_MIN || Value > SCHAR_MAX) {
            SetExceptionInfo(_T("DBBind::SetValue(short)"),_T("Value assigned by short overflow signed char limits"),_T("22003"),_T("Numeric value out of range"),0);
            return SQL_ERROR;
         }
         *(reinterpret_cast<signed char*>(pPointer)) = Value;
         nRetLength = oType->getNativeSize();
         break;
      case TYPE_UTINYINT:
         if (Value < 0) {
            SetExceptionInfo(_T("DBBind::SetValue(short)"),_T("Can't assign negative values to an unsigned value"),_T("22003"),_T("Numeric value out of range"),0);
            return SQL_ERROR;
         }
         if (Value > UCHAR_MAX) {
            SetExceptionInfo(_T("DBBind::SetValue(short)"),_T("Value assigned by short overflow unsigned char limits"),_T("22003"),_T("Numeric value out of range"),0);
            return SQL_ERROR;
         }
         *(reinterpret_cast<unsigned char*>(pPointer)) = Value;
         nRetLength = oType->getNativeSize();
         break;
      case TYPE_USMALLINT:
         if (Value < 0) {
            SetExceptionInfo(_T("DBBind::SetValue(short)"),_T("Can't assign negative values to an unsigned value"),_T("22003"),_T("Numeric value out of range"),0);
            return SQL_ERROR;
         }
      case TYPE_SSMALLINT:
         *(reinterpret_cast<short*>(pPointer)) = Value;
         nRetLength = oType->getNativeSize();
         break;
      case TYPE_UINTEGER :
         if (Value < 0) {
            SetExceptionInfo(_T("DBBind::SetValue(short)"),_T("Can't assign negative values to an unsigned value"),_T("22003"),_T("Numeric value out of range"),0);
            return SQL_ERROR;
         }
      case TYPE_SINTEGER :
         *(reinterpret_cast<int*>(pPointer)) = Value;
         nRetLength = oType->getNativeSize();
         break;
      case TYPE_UBIGINT  :
         if (Value < 0) {
            SetExceptionInfo(_T("DBBind::SetValue(short)"),_T("Can't assign negative values to an unsigned value"),_T("22003"),_T("Numeric value out of range"),0);
            return SQL_ERROR;
         }
      case TYPE_SBIGINT  :
         *(reinterpret_cast<__int64*>(pPointer)) = Value;
         nRetLength = oType->getNativeSize();
         break;
      case TYPE_REAL     :
         *(reinterpret_cast<float*>(pPointer)) = Value;
         nRetLength = oType->getNativeSize();
         break;
      case TYPE_DECIMAL:
      case TYPE_NUMERIC:
      case TYPE_DOUBLE :
      case TYPE_FLOAT  :
         *(reinterpret_cast<double*>(pPointer)) = Value;
         nRetLength = oType->getNativeSize();
         break;
      default:
         SetExceptionInfo(_T("DBBind::SetValue(short)"),_T("Data type don't suport SetValue by short"),_T("HY004"),_T("Invalid SQL data type"),0);
         return SQL_ERROR;
   }
   if (AfterSetValue != NULL) {
      AfterSetValue(this);
   }
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN DBBind::SetValue(const int Value)
{
   return SetValue((long)Value);
}
//---------------------------------------------------------------------------
SQLRETURN DBBind::SetValue(const long Value)
{
   TCHAR cErrMsg[SHORT_MSG] = {0};
   char cStrAnsi[12];  //LIMITS -2147483647 - 2147483647
   WCHAR cStrWide[12];
   size_t nLength = nDataSize;

   switch(oType->getDataType()) {
      case TYPE_CHAR:
      case TYPE_VARCHAR:
         nLength = _snprintf(cStrAnsi, 12, "%ld", Value);
         if (nLength > nDataSize) {
            SgFormatMsg(cErrMsg,SHORT_MSG,_T("Data type suporte max %d data length. Informed %d data length."),nDataSize,nLength);
            SetExceptionInfo(_T("DBBind::SetValue(long)"),cErrMsg,_T("HY090"),_T("Invalid string or buffer length"),0);
            return SQL_ERROR;
         }
         strncpy(reinterpret_cast<char*>(pPointer), cStrAnsi, nLength+1);
         nRetLength = SQL_NTS;
         break;
      case TYPE_BINARY:
      case TYPE_VARBINARY:
         nLength = _snprintf(cStrAnsi, 12, "%ld", Value);
         if (nLength > nDataSize) {
            SgFormatMsg(cErrMsg,SHORT_MSG,_T("Data type suporte max %d data length. Informed %d data length."),nDataSize,nLength);
            SetExceptionInfo(_T("DBBind::SetValue(long)"),cErrMsg,_T("HY090"),_T("Invalid string or buffer length"),0);
            return SQL_ERROR;
         }
         strncpy(reinterpret_cast<char*>(pPointer), cStrAnsi, nLength);
         nRetLength = (oType->getNativeSize() * nLength);
         break;
      case TYPE_WCHAR:
      case TYPE_WVARCHAR:
         nLength = _snwprintf(cStrWide, 12, L"%ld", Value);
         if (nLength > nDataSize) {
            SgFormatMsg(cErrMsg,SHORT_MSG,_T("Data type suporte max %d data length. Informed %d data length."),nDataSize,nLength);
            SetExceptionInfo(_T("DBBind::SetValue(long)"),cErrMsg,_T("HY090"),_T("Invalid string or buffer length"),0);
            return SQL_ERROR;
         }
         wcsncpy(reinterpret_cast<WCHAR*>(pPointer), cStrWide, nLength+1);
         nRetLength = SQL_NTS;
         break;
      case TYPE_STRINGA:
      case TYPE_LONGVARCHAR:
         nLength = _snprintf(cStrAnsi, 12, "%ld", Value);
         reinterpret_cast<AnsiString*>(pPointer)->assign(cStrAnsi);
         nRetLength = SQL_LEN_DATA_AT_EXEC(0);
         break;
      case TYPE_STRINGW:
      case TYPE_WLONGVARCHAR:
         nLength = _snwprintf(cStrWide, 12, L"%ld", Value);
         reinterpret_cast<WideString*>(pPointer)->assign(cStrWide);
         nRetLength = SQL_LEN_DATA_AT_EXEC(0);
         break;
      case TYPE_BLOB:
      case TYPE_LONGVARBINARY:
         nLength = _snprintf(cStrAnsi, 12, "%ld", Value);
         reinterpret_cast<Blob*>(pPointer)->assign(reinterpret_cast<BYTE*>(cStrAnsi));
         nRetLength = SQL_LEN_DATA_AT_EXEC(0);
         break;
      case TYPE_STINYINT:
         if (Value < SCHAR_MIN || Value > SCHAR_MAX) {
            SetExceptionInfo(_T("DBBind::SetValue(long)"),_T("Value assigned by long overflow signed char limits"),_T("22003"),_T("Numeric value out of range"),0);
            return SQL_ERROR;
         }
         *(reinterpret_cast<signed char*>(pPointer)) = Value;
         nRetLength = oType->getNativeSize();
         break;
      case TYPE_UTINYINT:
         if (Value < 0) {
            SetExceptionInfo(_T("DBBind::SetValue(long)"),_T("Can't assign negative values to an unsigned value"),_T("22003"),_T("Numeric value out of range"),0);
            return SQL_ERROR;
         }
         if (Value > UCHAR_MAX) {
            SetExceptionInfo(_T("DBBind::SetValue(long)"),_T("Value assigned by long overflow signed char limits"),_T("22003"),_T("Numeric value out of range"),0);
            return SQL_ERROR;
         }
         *(reinterpret_cast<unsigned char*>(pPointer)) = Value;
         nRetLength = oType->getNativeSize();
         break;
      case TYPE_USMALLINT:
         if (Value < 0) {
            SetExceptionInfo(_T("DBBind::SetValue(long)"),_T("Can't assign negative values to an unsigned value"),_T("22003"),_T("Numeric value out of range"),0);
            return SQL_ERROR;
         }
         if (Value > USHRT_MAX) {
            SetExceptionInfo(_T("DBBind::SetValue(long)"),_T("Value assigned by long overflow unsigned short limits"),_T("22003"),_T("Numeric value out of range"),0);
            return SQL_ERROR;
         }
         *(reinterpret_cast<unsigned short*>(pPointer)) = Value;
         nRetLength = oType->getNativeSize();
         break;
      case TYPE_SSMALLINT:
         if (Value < SHRT_MIN || Value > SHRT_MAX) {
            SetExceptionInfo(_T("DBBind::SetValue(long)"),_T("Value assigned by long overflow short limits"),_T("22003"),_T("Numeric value out of range"),0);
            return SQL_ERROR;
         }
         *(reinterpret_cast<short*>(pPointer)) = Value;
         nRetLength = oType->getNativeSize();
         break;
      case TYPE_UINTEGER :
         if (Value < 0) {
            SetExceptionInfo(_T("DBBind::SetValue(long)"),_T("Can't assign negative values to an unsigned value"),_T("22003"),_T("Numeric value out of range"),0);
            return SQL_ERROR;
         }
      case TYPE_SINTEGER :
         *(reinterpret_cast<long*>(pPointer)) = Value;
         nRetLength = oType->getNativeSize();
         break;
      case TYPE_UBIGINT  :
         if (Value < 0) {
            SetExceptionInfo(_T("DBBind::SetValue(long)"),_T("Can't assign negative values to an unsigned value"),_T("22003"),_T("Numeric value out of range"),0);
            return SQL_ERROR;
         }
      case TYPE_SBIGINT  :
         *(reinterpret_cast<__int64*>(pPointer)) = Value;
         nRetLength = oType->getNativeSize();
         break;
      case TYPE_REAL     :
         *(reinterpret_cast<float*>(pPointer)) = Value;
         nRetLength = oType->getNativeSize();
         break;
      case TYPE_DECIMAL:
      case TYPE_NUMERIC:
      case TYPE_DOUBLE :
      case TYPE_FLOAT  :
         *(reinterpret_cast<double*>(pPointer)) = Value;
         nRetLength = oType->getNativeSize();
         break;
      case TYPE_DATE     :
      {
         SQL_DATETIME_STRUCT dDateTime;
         DoubleToDateTime(Value,&dDateTime);
         SQL_DATE_STRUCT *dDate = reinterpret_cast<SQL_DATE_STRUCT*>(pPointer);
         dDate->day = dDateTime.day;
         dDate->month = dDateTime.month;
         dDate->year = dDateTime.year;
         nRetLength = oType->getNativeSize();
      }
      break;
      case TYPE_TIMESTAMP:
      {
         SQL_DATETIME_STRUCT *dDateTime = reinterpret_cast<SQL_DATETIME_STRUCT*>(pPointer);
         DoubleToDateTime(Value,dDateTime);
         nRetLength = oType->getNativeSize();
      }
      break;
      default:
         SetExceptionInfo(_T("DBBind::SetValue(long)"),_T("Data type don't suport SetValue by long"),_T("HY004"),_T("Invalid SQL data type"),0);
         return SQL_ERROR;
   }
   if (AfterSetValue != NULL) {
      AfterSetValue(this);
   }
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN DBBind::SetValue(const __int64 Value)
{
   TCHAR cErrMsg[SHORT_MSG] = {0};
   char cStrAnsi[21];  //LIMITS -9223372036854775807 - 9223372036854775807
   WCHAR cStrWide[21];
   size_t nLength = nDataSize;

   switch(oType->getDataType()) {
      case TYPE_CHAR:
      case TYPE_VARCHAR:
         nLength = _snprintf(cStrAnsi, 21, "%lld", Value);
         if (nLength > nDataSize) {
            SgFormatMsg(cErrMsg,SHORT_MSG,_T("Data type suporte max %d data length. Informed %d data length."),nDataSize,nLength);
            SetExceptionInfo(_T("DBBind::SetValue(__int64)"),cErrMsg,_T("HY090"),_T("Invalid string or buffer length"),0);
            return SQL_ERROR;
         }
         strncpy(reinterpret_cast<char*>(pPointer), cStrAnsi, nLength+1);
         nRetLength = SQL_NTS;
         break;
      case TYPE_BINARY:
      case TYPE_VARBINARY:
         nLength = _snprintf(cStrAnsi, 21, "%lld", Value);
         if (nLength > nDataSize) {
            SgFormatMsg(cErrMsg,SHORT_MSG,_T("Data type suporte max %d data length. Informed %d data length."),nDataSize,nLength);
            SetExceptionInfo(_T("DBBind::SetValue(__int64)"),cErrMsg,_T("HY090"),_T("Invalid string or buffer length"),0);
            return SQL_ERROR;
         }
         strncpy(reinterpret_cast<char*>(pPointer), cStrAnsi, nLength);
         nRetLength = (oType->getNativeSize() * nLength);
         break;
      case TYPE_WCHAR:
      case TYPE_WVARCHAR:
         nLength = _snwprintf(cStrWide, 21, L"%lld", Value);
         if (nLength > nDataSize) {
            SgFormatMsg(cErrMsg,SHORT_MSG,_T("Data type suporte max %d data length. Informed %d data length."),nDataSize,nLength);
            SetExceptionInfo(_T("DBBind::SetValue(__int64)"),cErrMsg,_T("HY090"),_T("Invalid string or buffer length"),0);
            return SQL_ERROR;
         }
         wcsncpy(reinterpret_cast<WCHAR*>(pPointer), cStrWide, nLength+1);
         nRetLength = SQL_NTS;
         break;
      case TYPE_STRINGA:
      case TYPE_LONGVARCHAR:
         nLength = _snprintf(cStrAnsi, 21, "%lld", Value);
         reinterpret_cast<AnsiString*>(pPointer)->assign(cStrAnsi);
         nRetLength = SQL_LEN_DATA_AT_EXEC(0);
         break;
      case TYPE_STRINGW:
      case TYPE_WLONGVARCHAR:
         nLength = _snwprintf(cStrWide, 21, L"%lld", Value);
         reinterpret_cast<WideString*>(pPointer)->assign(cStrWide);
         nRetLength = SQL_LEN_DATA_AT_EXEC(0);
         break;
      case TYPE_BLOB:
      case TYPE_LONGVARBINARY:
         nLength = _snprintf(cStrAnsi, 21, "%lld", Value);
         reinterpret_cast<Blob*>(pPointer)->assign(reinterpret_cast<BYTE*>(cStrAnsi));
         nRetLength = SQL_LEN_DATA_AT_EXEC(0);
         break;
      case TYPE_USMALLINT:
         if (Value < 0) {
            SetExceptionInfo(_T("DBBind::SetValue(__int64)"),_T("Can't assign negative values to an unsigned value"),_T("22003"),_T("Numeric value out of range"),0);
            return SQL_ERROR;
         }
         if (Value > USHRT_MAX) {
            SetExceptionInfo(_T("DBBind::SetValue(__int64)"),_T("Value assigned by __int64 overflow unsigned short limits"),_T("22003"),_T("Numeric value out of range"),0);
            return SQL_ERROR;
         }
         *(reinterpret_cast<unsigned short*>(pPointer)) = Value;
         nRetLength = oType->getNativeSize();
         break;
      case TYPE_SSMALLINT:
         if (Value < SHRT_MIN || Value > SHRT_MAX) {
            SetExceptionInfo(_T("DBBind::SetValue(__int64)"),_T("Value assigned by __int64 overflow short limits"),_T("22003"),_T("Numeric value out of range"),0);
            return SQL_ERROR;
         }
         *(reinterpret_cast<short*>(pPointer)) = Value;
         nRetLength = oType->getNativeSize();
         break;
      case TYPE_UINTEGER :
         if (Value < 0) {
            SetExceptionInfo(_T("DBBind::SetValue(__int64)"),_T("Can't assign negative values to an unsigned value"),_T("22003"),_T("Numeric value out of range"),0);
            return SQL_ERROR;
         }
         if (Value > ULONG_MAX) {
            SetExceptionInfo(_T("DBBind::SetValue(__int64)"),_T("Value assigned by __int64 overflow unsigned long limits"),_T("22003"),_T("Numeric value out of range"),0);
            return SQL_ERROR;
         }
         *(reinterpret_cast<unsigned long*>(pPointer)) = Value;
         nRetLength = oType->getNativeSize();
         break;
      case TYPE_SINTEGER :
         if (Value < LONG_MIN || Value > LONG_MAX) {
            SetExceptionInfo(_T("DBBind::SetValue(__int64)"),_T("Value assigned by __int64 overflow long limits"),_T("22003"),_T("Numeric value out of range"),0);
            return SQL_ERROR;
         }
         *(reinterpret_cast<long*>(pPointer)) = Value;
         nRetLength = oType->getNativeSize();
         break;
      case TYPE_UBIGINT  :
         if (Value < 0) {
            SetExceptionInfo(_T("DBBind::SetValue(__int64)"),_T("Can't assign negative values to an unsigned value"),_T("22003"),_T("Numeric value out of range"),0);
            return SQL_ERROR;
         }
      case TYPE_SBIGINT  :
         *(reinterpret_cast<__int64*>(pPointer)) = Value;
         nRetLength = oType->getNativeSize();
         break;
      default:
         SetExceptionInfo(_T("DBBind::SetValue(__int64)"),_T("Data type don't suport SetValue by __int64"),_T("HY004"),_T("Invalid SQL data type"),0);
         return SQL_ERROR;
   }
   if (AfterSetValue != NULL) {
      AfterSetValue(this);
   }
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN DBBind::SetValue(const float Value, size_t nDecimal = 6)
{
   return SetValue((double)Value, nDecimal);
}
//---------------------------------------------------------------------------
SQLRETURN DBBind::SetValue(const double Value, size_t nDecimal = 6)
{
   TCHAR cErrMsg[SHORT_MSG] = {0};
   char cStrAnsi[FLT_MAX_10_EXP + FLT_DIG + 4];  //LIMITS -1E+37 - 1E+37
   WCHAR cStrWide[FLT_MAX_10_EXP + FLT_DIG + 4];
   SQLULEN nLength = nDataSize;
   char cMaskAnsi[6];
   WCHAR cMaskWide[6];

   if (nDecimal > 10)
      nDecimal = 10;

   switch(oType->getDataType()) {
      case TYPE_CHAR:
      case TYPE_VARCHAR:
         _snprintf(cMaskAnsi,6,"%s%u%c","%.",nDecimal,'f');
         nLength = _snprintf(cStrAnsi, (FLT_MAX_10_EXP + FLT_DIG + 4), cMaskAnsi, Value);
         if (nLength > nDataSize) {
            SgFormatMsg(cErrMsg,SHORT_MSG,_T("Data type suporte max %d data length. Informed %d data length."),nDataSize,nLength);
            SetExceptionInfo(_T("DBBind::SetValue(double)"),cErrMsg,_T("HY090"),_T("Invalid string or buffer length"),0);
            return SQL_ERROR;
         }
         strncpy(reinterpret_cast<char*>(pPointer), cStrAnsi, nLength+1);
         nRetLength = SQL_NTS;
         break;
      case TYPE_BINARY:
      case TYPE_VARBINARY:
         _snprintf(cMaskAnsi,6,"%s%u%c","%.",nDecimal,'f');
         nLength = _snprintf(cStrAnsi, (FLT_MAX_10_EXP + FLT_DIG + 4), cMaskAnsi, Value);
         if (nLength > nDataSize) {
            SgFormatMsg(cErrMsg,SHORT_MSG,_T("Data type suporte max %d data length. Informed %d data length."),nDataSize,nLength);
            SetExceptionInfo(_T("DBBind::SetValue(double)"),cErrMsg,_T("HY090"),_T("Invalid string or buffer length"),0);
            return SQL_ERROR;
         }
         strncpy(reinterpret_cast<char*>(pPointer), cStrAnsi, nLength);
         nRetLength = (oType->getNativeSize() * nLength);
         break;
      case TYPE_WCHAR:
      case TYPE_WVARCHAR:
         _snwprintf(cMaskWide,6,L"%s%u%c",L"%.",nDecimal,L'f');
         nLength = _snwprintf(cStrWide, (FLT_MAX_10_EXP + FLT_DIG + 4), cMaskWide, Value);
         if (nLength > nDataSize) {
            SgFormatMsg(cErrMsg,SHORT_MSG,_T("Data type suporte max %d data length. Informed %d data length."),nDataSize,nLength);
            SetExceptionInfo(_T("DBBind::SetValue(double)"),cErrMsg,_T("HY090"),_T("Invalid string or buffer length"),0);
            return SQL_ERROR;
         }
         wcsncpy(reinterpret_cast<WCHAR*>(pPointer), cStrWide, nLength+1);
         nRetLength = SQL_NTS;
         break;
      case TYPE_STRINGA:
      case TYPE_LONGVARCHAR:
         _snprintf(cMaskAnsi,6,"%s%u%c","%.",nDecimal,'f');
         nLength = _snprintf(cStrAnsi, (FLT_MAX_10_EXP + FLT_DIG + 4), cMaskAnsi, Value);
         reinterpret_cast<AnsiString*>(pPointer)->assign(cStrAnsi);
         nRetLength = SQL_LEN_DATA_AT_EXEC(0);
         break;
      case TYPE_STRINGW:
      case TYPE_WLONGVARCHAR:
         _snwprintf(cMaskWide,6,L"%s%u%c",L"%.",nDecimal,L'f');
         nLength = _snwprintf(cStrWide, (FLT_MAX_10_EXP + FLT_DIG + 4), cMaskWide, Value);
         reinterpret_cast<WideString*>(pPointer)->assign(cStrWide);
         nRetLength = SQL_LEN_DATA_AT_EXEC(0);
         break;
      case TYPE_BLOB:
      case TYPE_LONGVARBINARY:
         _snprintf(cMaskAnsi,6,"%s%u%c","%.",nDecimal,'f');
         nLength = _snprintf(cStrAnsi, (FLT_MAX_10_EXP + FLT_DIG + 4), cMaskAnsi, Value);
         reinterpret_cast<Blob*>(pPointer)->assign(reinterpret_cast<BYTE*>(cStrAnsi));
         nRetLength = SQL_LEN_DATA_AT_EXEC(0);
         break;
      case TYPE_REAL     :
         *(reinterpret_cast<float*>(pPointer)) = Value;
         nRetLength = oType->getNativeSize();
         break;
      case TYPE_DECIMAL:
      case TYPE_NUMERIC:
      case TYPE_DOUBLE :
      case TYPE_FLOAT  :
         *(reinterpret_cast<double*>(pPointer)) = Value;
         nRetLength = oType->getNativeSize();
         break;
      case TYPE_DATE     :
      {
         SQL_DATETIME_STRUCT dDateTime;
         DoubleToDateTime(Value,&dDateTime);
         SQL_DATE_STRUCT *dDate = reinterpret_cast<SQL_DATE_STRUCT*>(pPointer);
         dDate->day = dDateTime.day;
         dDate->month = dDateTime.month;
         dDate->year = dDateTime.year;
         nRetLength = oType->getNativeSize();
      }
      break;
      case TYPE_TIMESTAMP:
      {
         SQL_DATETIME_STRUCT *dDateTime = reinterpret_cast<SQL_DATETIME_STRUCT*>(pPointer);
         DoubleToDateTime(Value,dDateTime);
         nRetLength = oType->getNativeSize();
      }
      break;
      default:
         SetExceptionInfo(_T("DBBind::SetValue(double)"),_T("Data type don't suport SetValue by double"),_T("HY004"),_T("Invalid SQL data type"),0);
         return SQL_ERROR;
   }
   if (AfterSetValue != NULL) {
      AfterSetValue(this);
   }
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN DBBind::SetValue(const AnsiString* Value)
{
   return SetValue((char*)Value->data(), Value->length());
}
//---------------------------------------------------------------------------
SQLRETURN DBBind::SetValue(const WideString* Value)
{
   return SetValue((WCHAR*)Value->data(), Value->length());
}
//---------------------------------------------------------------------------
SQLRETURN DBBind::SetValue(const Blob* Value)
{
   return SetValue((char*)Value->data(), Value->length());
}
//---------------------------------------------------------------------------
SQLRETURN DBBind::SetValue(const AnsiString &Value)
{
   return SetValue((char*)Value.data(), Value.length());
}
//---------------------------------------------------------------------------
SQLRETURN DBBind::SetValue(const WideString &Value)
{
   return SetValue((WCHAR*)Value.data(), Value.length());
}
//---------------------------------------------------------------------------
SQLRETURN DBBind::SetValue(const Blob &Value)
{
   return SetValue((char*)Value.data(), Value.length());
}
//---------------------------------------------------------------------------
SQLRETURN DBBind::SetValue(SQL_DATE_STRUCT Value)
{
   TCHAR cErrMsg[SHORT_MSG] = {0};
   char cStrAnsi[11];  //LIMITS 01/01/1900
   WCHAR cStrWide[11];

   switch(oType->getDataType()) {
      case TYPE_CHAR:
      case TYPE_VARCHAR:
         if (nDataSize < 10) {
            SgFormatMsg(cErrMsg,SHORT_MSG,_T("Data type suporte max %d data length. Informed %d data length."),nDataSize,10);
            SetExceptionInfo(_T("DBBind::SetValue(SQL_DATE_STRUCT)"),cErrMsg,_T("HY090"),_T("Invalid string or buffer length"),0);
            return SQL_ERROR;
         }
         _snprintf(cStrAnsi,11,STRA_DATE_FMT,Value.year,Value.month,Value.day);
         nRetLength = SQL_NTS;
         break;
      case TYPE_WCHAR:
      case TYPE_WVARCHAR:
         if (nDataSize < 10) {
            SgFormatMsg(cErrMsg,SHORT_MSG,_T("Data type suporte max %d data length. Informed %d data length."),nDataSize,10);
            SetExceptionInfo(_T("DBBind::SetValue(SQL_DATE_STRUCT)"),cErrMsg,_T("HY090"),_T("Invalid string or buffer length"),0);
            return SQL_ERROR;
         }
         _snwprintf(cStrWide, 11,STRW_DATE_FMT,Value.year,Value.month,Value.day);
         nRetLength = SQL_NTS;
         break;
      case TYPE_STRINGA:
      case TYPE_LONGVARCHAR:
         _snprintf(cStrAnsi,11,STRA_DATE_FMT,Value.year,Value.month,Value.day);
         reinterpret_cast<AnsiString*>(pPointer)->assign(cStrAnsi);
         nRetLength = SQL_LEN_DATA_AT_EXEC(0);
         break;
      case TYPE_STRINGW:
      case TYPE_WLONGVARCHAR:
         _snwprintf(cStrWide,11,STRW_DATE_FMT,Value.year,Value.month,Value.day);
         reinterpret_cast<WideString*>(pPointer)->assign(cStrWide);
         nRetLength = SQL_LEN_DATA_AT_EXEC(0);
         break;
      case TYPE_SINTEGER :
      case TYPE_UINTEGER :
         {
         SQL_DATETIME_STRUCT dDateTime;
         SgZeroMemory(&dDateTime,sizeof(SQL_DATETIME_STRUCT));
         dDateTime.day = Value.day;
         dDateTime.month = Value.month;
         dDateTime.year = Value.year;
         *(reinterpret_cast<int*>(pPointer)) = (int)DateTimeToDouble(&dDateTime);
         nRetLength = oType->getNativeSize();
         }
         break;
      case TYPE_REAL   :
      case TYPE_DECIMAL:
      case TYPE_NUMERIC:
      case TYPE_DOUBLE :
      case TYPE_FLOAT  :
         {
         SQL_DATETIME_STRUCT dDateTime;
         SgZeroMemory(&dDateTime,sizeof(SQL_DATETIME_STRUCT));
         dDateTime.day = Value.day;
         dDateTime.month = Value.month;
         dDateTime.year = Value.year;
         *(reinterpret_cast<double*>(pPointer)) = DateTimeToDouble(&dDateTime);
         nRetLength = oType->getNativeSize();
         }
         break;
      case TYPE_DATE     :
      {
         SQL_DATE_STRUCT *dDate = reinterpret_cast<SQL_DATE_STRUCT*>(pPointer);
         dDate->day = Value.day;
         dDate->month = Value.month;
         dDate->year = Value.year;
         nRetLength = oType->getNativeSize();
      }
      break;
      case TYPE_TIMESTAMP:
      {
         SQL_DATETIME_STRUCT *dDateTime = reinterpret_cast<SQL_DATETIME_STRUCT*>(pPointer);
         SgZeroMemory(pPointer,sizeof(SQL_DATETIME_STRUCT));
         dDateTime->day = Value.day;
         dDateTime->month = Value.month;
         dDateTime->year = Value.year;
         nRetLength = oType->getNativeSize();
      }
      break;
      default:
         SetExceptionInfo(_T("DBBind::SetValue(SQL_DATE_STRUCT)"),_T("Data type don't suport SetValue by SQL_DATE_STRUCT"),_T("HY004"),_T("Invalid SQL data type"),0);
         return SQL_ERROR;
   }
   if (AfterSetValue != NULL) {
      AfterSetValue(this);
   }
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN DBBind::SetValue(SQL_TIME_STRUCT Value)
{
   TCHAR cErrMsg[SHORT_MSG] = {0};
   char cStrAnsi[9];  //LIMITS 00:00:00
   WCHAR cStrWide[9];

   switch(oType->getDataType()) {
      case TYPE_CHAR:
      case TYPE_VARCHAR:
         if (nDataSize < 8) {
            SgFormatMsg(cErrMsg,SHORT_MSG,_T("Data type suporte max %d data length. Informed %d data length."),nDataSize,8);
            SetExceptionInfo(_T("DBBind::SetValue(SQL_TIME_STRUCT)"),cErrMsg,_T("HY090"),_T("Invalid string or buffer length"),0);
            return SQL_ERROR;
         }
         _snprintf(reinterpret_cast<char*>(pPointer), 9, STRA_HOUR_FMT, Value.hour, Value.minute, Value.second);
         nRetLength = SQL_NTS;
         break;
      case TYPE_WCHAR:
      case TYPE_WVARCHAR:
         if (nDataSize < 8) {
            SgFormatMsg(cErrMsg,SHORT_MSG,_T("Data type suporte max %d data length. Informed %d data length."),nDataSize,8);
            SetExceptionInfo(_T("DBBind::SetValue(SQL_TIME_STRUCT)"),cErrMsg,_T("HY090"),_T("Invalid string or buffer length"),0);
            return SQL_ERROR;
         }
         _snwprintf(reinterpret_cast<WCHAR*>(pPointer), 9, STRW_HOUR_FMT, Value.hour, Value.minute, Value.second);
         nRetLength = SQL_NTS;
         break;
      case TYPE_STRINGA:
      case TYPE_LONGVARCHAR:
         _snprintf(cStrAnsi, 9, STRA_HOUR_FMT, Value.hour, Value.minute, Value.second);
         reinterpret_cast<AnsiString*>(pPointer)->assign(cStrAnsi);
         nRetLength = SQL_LEN_DATA_AT_EXEC(0);
         break;
      case TYPE_STRINGW:
      case TYPE_WLONGVARCHAR:
         _snwprintf(cStrWide, 9, STRW_HOUR_FMT, Value.hour, Value.minute, Value.second);
         reinterpret_cast<WideString*>(pPointer)->assign(cStrWide);
         nRetLength = SQL_LEN_DATA_AT_EXEC(0);
         break;
      case TYPE_TIME     :
      {
         SQL_TIME_STRUCT *dTime = reinterpret_cast<SQL_TIME_STRUCT*>(pPointer);
         dTime->hour = Value.hour;
         dTime->minute = Value.minute;
         dTime->second = Value.second;
         nRetLength = oType->getNativeSize();
      }
      break;
      default:
         SetExceptionInfo(_T("DBBind::SetValue(SQL_TIME_STRUCT)"),_T("Data type don't suport SetValue by SQL_TIME_STRUCT"),_T("HY004"),_T("Invalid SQL data type"),0);
         return SQL_ERROR;
   }
   if (AfterSetValue != NULL) {
      AfterSetValue(this);
   }
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN DBBind::SetValue(SQL_DATETIME_STRUCT Value)
{
   TCHAR cErrMsg[SHORT_MSG] = {0};
   char cStrAnsi[20];  //LIMITS 1900-01-01 00:00:00
   WCHAR cStrWide[20];

   switch(oType->getDataType()) {
      case TYPE_CHAR:
      case TYPE_VARCHAR:
         if (nDataSize < 19) {
            SgFormatMsg(cErrMsg,SHORT_MSG,_T("Data type suporte max %d data length. Informed %d data length."),nDataSize,19);
            SetExceptionInfo(_T("DBBind::SetValue(SQL_DATETIME_STRUCT)"),cErrMsg,_T("HY090"),_T("Invalid string or buffer length"),0);
            return SQL_ERROR;
         }
         _snprintf(reinterpret_cast<char*>(pPointer),20,STRA_DTTM_FMT,Value.year,Value.month,Value.day,Value.hour,Value.minute,Value.second);
         nRetLength = SQL_NTS;
         break;
      case TYPE_WCHAR:
      case TYPE_WVARCHAR:
         if (nDataSize < 19) {
            SgFormatMsg(cErrMsg,SHORT_MSG,_T("Data type suporte max %d data length. Informed %d data length."),nDataSize,19);
            SetExceptionInfo(_T("DBBind::SetValue(SQL_DATETIME_STRUCT)"),cErrMsg,_T("HY090"),_T("Invalid string or buffer length"),0);
            return SQL_ERROR;
         }
         _snwprintf(reinterpret_cast<WCHAR*>(pPointer),20,STRW_DTTM_FMT,Value.year,Value.month,Value.day,Value.hour,Value.minute,Value.second);
         nRetLength = SQL_NTS;
         break;
      case TYPE_STRINGA:
      case TYPE_LONGVARCHAR:
         _snprintf(cStrAnsi,20,STRA_DTTM_FMT,Value.year,Value.month,Value.day,Value.hour,Value.minute,Value.second);
         reinterpret_cast<AnsiString*>(pPointer)->assign(cStrAnsi);
         nRetLength = SQL_LEN_DATA_AT_EXEC(0);
         break;
      case TYPE_STRINGW:
      case TYPE_WLONGVARCHAR:
         _snwprintf(cStrWide,20,STRW_DTTM_FMT,Value.year,Value.month,Value.day,Value.hour,Value.minute,Value.second);
         reinterpret_cast<WideString*>(pPointer)->assign(cStrWide);
         nRetLength = SQL_LEN_DATA_AT_EXEC(0);
         break;
      case TYPE_REAL   :
         *(reinterpret_cast<float*>(pPointer)) = (float)DateTimeToDouble(&Value);
         nRetLength = oType->getNativeSize();
         break;
      case TYPE_DECIMAL:
      case TYPE_NUMERIC:
      case TYPE_DOUBLE :
      case TYPE_FLOAT  :
         *(reinterpret_cast<double*>(pPointer)) = DateTimeToDouble(&Value);
         nRetLength = oType->getNativeSize();
         break;
      case TYPE_DATE     :
      {
         SQL_DATE_STRUCT *dDate = reinterpret_cast<SQL_DATE_STRUCT*>(pPointer);
         dDate->day = Value.day;
         dDate->month = Value.month;
         dDate->year = Value.year;
         nRetLength = oType->getNativeSize();
      }
      break;
      case TYPE_TIME     :
      {
         SQL_TIME_STRUCT *dTime = reinterpret_cast<SQL_TIME_STRUCT*>(pPointer);
         dTime->hour = Value.hour;
         dTime->minute = Value.minute;
         dTime->second = Value.second;
         nRetLength = oType->getNativeSize();
      }
      break;
      case TYPE_TIMESTAMP:
         memcpy(pPointer,(void*)&Value,sizeof(SQL_DATETIME_STRUCT));
         nRetLength = oType->getNativeSize();
         break;
      default:
         SetExceptionInfo(_T("DBBind::SetValue(SQL_DATETIME_STRUCT)"),_T("Data type don't suport SetValue by SQL_DATETIME_STRUCT"),_T("HY004"),_T("Invalid SQL data type"),0);
         return SQL_ERROR;
   }
   if (AfterSetValue != NULL) {
      AfterSetValue(this);
   }
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
const String& DBBind::AsString()
{
   DataType eDataType = oType->getDataType();
   sStrValue.clear();

   if (IsNull())
      return sStrValue;

   switch(eDataType) {
      case TYPE_CHAR:
      case TYPE_VARCHAR:
#if defined(_UNICODE)
      {
         //Deve transformar o char* em wchar_t*
         WCHAR *cStrWide = new WCHAR[nRetLength+1];
         mbstowcs (cStrWide, reinterpret_cast<char*>(pPointer), nRetLength);
         cStrWide[nRetLength] = L'\0';
         sStrValue.assign(cStrWide);
         delete[] cStrWide;
      }
#else
         sStrValue.assign((char*)pPointer);
#endif
         break;
      case TYPE_WCHAR:
      case TYPE_WVARCHAR:
#if defined(_UNICODE)
         sStrValue.assign((TCHAR*)pPointer);
#else
      {
         int nLength = (nRetLength / oType->getNativeSize());
         char *cStrAnsi = new char[nLength+1];
         wcstombs(cStrAnsi, reinterpret_cast<WCHAR*>(pPointer), nLength);
         cStrAnsi[nLength] = '\0';
         sStrValue.assign(cStrAnsi);
         delete[] cStrAnsi;
      }
#endif
         break;
      case TYPE_BIT      :
      case TYPE_STINYINT :
      case TYPE_UTINYINT :
      case TYPE_SSMALLINT:
      case TYPE_USMALLINT:
      case TYPE_SINTEGER :
      case TYPE_UINTEGER :
      {
         int nReturn = 0;
         if (eDataType == TYPE_BIT || eDataType == TYPE_UTINYINT)
            nReturn = (int)*((SQLCHAR*)(pPointer));
         else if (eDataType == TYPE_STINYINT)
            nReturn = (int)*((SQLSCHAR*)(pPointer));
         else if (eDataType == TYPE_SSMALLINT)
            nReturn = (int)*((SQLSMALLINT*)(pPointer));
         else if (eDataType == TYPE_USMALLINT)
            nReturn = (int)*((SQLUSMALLINT*)(pPointer));
         else if (eDataType == TYPE_SINTEGER)
            nReturn = (int)*((SQLINTEGER*)(pPointer));
         else if (eDataType == TYPE_UINTEGER)
            nReturn = (int)*((SQLUINTEGER*)(pPointer));
         TCHAR cBuffer[12] = {0};
         SgFormatMsg(cBuffer, 12, _T("%d"), nReturn);
         sStrValue.assign(cBuffer);
      }
      break;
      case TYPE_SBIGINT  :
      case TYPE_UBIGINT  :
      {
         SQLBIGINT nReturn = 0;
         if (eDataType == TYPE_SBIGINT)
            nReturn = (SQLBIGINT)*((SQLBIGINT*)(pPointer));
         else if (eDataType == TYPE_UBIGINT)
            nReturn = (SQLBIGINT)*((SQLUBIGINT*)(pPointer));
         TCHAR cBuffer[21] = {0};
         SgFormatMsg(cBuffer, 21, _T("%lld"), nReturn);
         sStrValue.assign(cBuffer);
     }
      case TYPE_REAL   :
      case TYPE_DECIMAL:
      case TYPE_NUMERIC:
      case TYPE_DOUBLE :
      case TYPE_FLOAT  :
      {
         double nReturn = 0;
         if (eDataType == TYPE_REAL)
            nReturn = (double)*((float*)(pPointer));
         else
            nReturn = (double)*((double*)(pPointer));
         TCHAR mask[6] = {0};
         TCHAR cBuffer[FLT_MAX_10_EXP + FLT_DIG + 4] = {0};
         SgFormatMsg(mask,6,_T("%s%u%c"),_T("%."),nDecimalDigits,_T('f'));
         SgFormatMsg(cBuffer, (FLT_MAX_10_EXP + FLT_DIG + 4), mask, nReturn);
         sStrValue.assign(cBuffer);
      }
      break;
      case TYPE_DATE:
      {
         SQL_DATE_STRUCT *dDate = reinterpret_cast<SQL_DATE_STRUCT*>(pPointer);
         TCHAR cBuffer[11] = {0};
         SgFormatMsg(cBuffer,11,_T(STRA_DATE_FMT),dDate->year,dDate->month,dDate->day);
         sStrValue.assign(cBuffer);
      }
      break;
      case TYPE_TIME:
      {
         SQL_TIME_STRUCT *dTime = reinterpret_cast<SQL_TIME_STRUCT*>(pPointer);
         TCHAR cBuffer[9] = {0};
         SgFormatMsg(cBuffer,9,_T(STRA_HOUR_FMT),dTime->hour,dTime->minute,dTime->second);
         sStrValue.assign(cBuffer);
      }
      break;
      case TYPE_TIMESTAMP:
      {
         TCHAR cBuffer[20] = {0};
         SQL_DATETIME_STRUCT *dDateTime = reinterpret_cast<SQL_DATETIME_STRUCT*>(pPointer);
         SgFormatMsg(cBuffer,20,_T(STRA_DTTM_FMT),dDateTime->year,dDateTime->month,dDateTime->day,dDateTime->hour,dDateTime->minute,dDateTime->second);
         sStrValue.assign(cBuffer);
      }
      break;
      case TYPE_STRINGW:
      case TYPE_WLONGVARCHAR:
#if defined(_UNICODE)
         sStrValue = reinterpret_cast<WideString*>(pPointer);
#else
      {
         //Deve transformar o wchar_t* em char*
         WideString *Value = reinterpret_cast<WideString*>(pPointer);
         int nLength = Value->length();
         char *cStrAnsi = new char[nLength+1];
         wcstombs(cStrAnsi, Value->data(), nLength);
         cStrAnsi[nLength] = '\0';
         sStrValue.assign(cStrAnsi);
         delete[] cStrAnsi;
      }
#endif
      break;
      case TYPE_STRINGA:
      case TYPE_LONGVARCHAR:
#if defined(_UNICODE)
      {
         //Deve transformar o char* em wchar_t*
         AnsiString *Value = reinterpret_cast<AnsiString*>(pPointer);
         int nLength = Value->length();
         WCHAR *cStrWide = new WCHAR[nLength+1];
         mbstowcs (cStrWide, Value->data(), nLength);
         cStrWide[nLength] = L'\0';
         sStrValue.assign(cStrWide);
         delete[] cStrWide;
      }
#else
         sStrValue.assign(reinterpret_cast<AnsiString*>(pPointer)->c_str());
#endif
      break;
      default: //nothing
         break;
   }
   return sStrValue;
}
//---------------------------------------------------------------------------
int DBBind::AsInteger()
{
   DataType eDataType = oType->getDataType();
   int nReturn = 0;

   if (IsNull())
      return nReturn;

   switch(eDataType) {
      case TYPE_CHAR:
      case TYPE_VARCHAR:
         nReturn = strtol((char*)pPointer,NULL,10);
         return nReturn;
      case TYPE_BIT      :
      case TYPE_UTINYINT :
         nReturn = (int)*((SQLCHAR*)(pPointer));
         return nReturn;
      case TYPE_STINYINT :
         nReturn = (int)*((SQLSCHAR*)(pPointer));
         return nReturn;
      case TYPE_SSMALLINT:
         nReturn = (int)*((SQLSMALLINT*)(pPointer));
         return nReturn;
      case TYPE_USMALLINT:
         nReturn = (int)*((SQLUSMALLINT*)(pPointer));
         return nReturn;
      case TYPE_SINTEGER :
         nReturn = (int)*((SQLINTEGER*)(pPointer));
         return nReturn;
      case TYPE_UINTEGER :
         nReturn = (int)*((SQLUINTEGER*)(pPointer));
         return nReturn;
      case TYPE_SBIGINT  :
         nReturn = (int)*((SQLBIGINT*)(pPointer));
         return nReturn;
      case TYPE_UBIGINT  :
         nReturn = (int)*((SQLUBIGINT*)(pPointer));
         return nReturn;
      case TYPE_REAL   :
         nReturn = (int)*((float*)(pPointer));
         return nReturn;
      case TYPE_DECIMAL:
      case TYPE_NUMERIC:
      case TYPE_DOUBLE :
      case TYPE_FLOAT  :
         nReturn = (int)*((double*)(pPointer));
         return nReturn;
      case TYPE_WCHAR:
      case TYPE_WVARCHAR:
         nReturn = wcstol((WCHAR*)pPointer,NULL,10);
         return nReturn;
      case TYPE_DATE:
      {
         SQL_DATE_STRUCT *dDate = reinterpret_cast<SQL_DATE_STRUCT*>(pPointer);
         SQL_DATETIME_STRUCT dDateTime;
         SgZeroMemory(&dDateTime,sizeof(SQL_DATETIME_STRUCT));
         dDateTime.day = dDate->day;
         dDateTime.month = dDate->month;
         dDateTime.year = dDate->year;
         nReturn = (int)DateTimeToDouble(&dDateTime);
         return nReturn;
      }
      case TYPE_TIMESTAMP:
         nReturn = (int)DateTimeToDouble(reinterpret_cast<SQL_DATETIME_STRUCT*>(pPointer));
         return nReturn;
      case TYPE_STRINGA:
         nReturn = strtol(reinterpret_cast<AnsiString*>(pPointer)->data(),NULL,10);
         return nReturn;
      case TYPE_STRINGW:
         nReturn = wcstol(reinterpret_cast<WideString*>(pPointer)->data(),NULL,10);
         return nReturn;
      default: //nothing
         break;
   }
   return nReturn;
}
//---------------------------------------------------------------------------
float DBBind::AsFloat()
{
   return (float)AsDouble();
}
//---------------------------------------------------------------------------
double DBBind::AsDouble()
{
   DataType eDataType = oType->getDataType();
   double nReturn = 0;

   if (IsNull())
      return nReturn;

   switch(eDataType) {
      case TYPE_CHAR:
      case TYPE_VARCHAR:
         nReturn = strtod((char*)pPointer,NULL);
         return nReturn;
      case TYPE_BIT      :
      case TYPE_UTINYINT :
         nReturn = (double)*((SQLCHAR*)(pPointer));
         return nReturn;
      case TYPE_STINYINT :
         nReturn = (double)*((SQLSCHAR*)(pPointer));
         return nReturn;
      case TYPE_SSMALLINT:
         nReturn = (double)*((SQLSMALLINT*)(pPointer));
         return nReturn;
      case TYPE_USMALLINT:
         nReturn = (double)*((SQLUSMALLINT*)(pPointer));
         return nReturn;
      case TYPE_SINTEGER :
         nReturn = (double)*((SQLINTEGER*)(pPointer));
         return nReturn;
      case TYPE_UINTEGER :
         nReturn = (double)*((SQLUINTEGER*)(pPointer));
         return nReturn;
      case TYPE_SBIGINT  :
         nReturn = (double)*((SQLBIGINT*)(pPointer));
         return nReturn;
      case TYPE_UBIGINT  :
         nReturn = (double)*((SQLUBIGINT*)(pPointer));
         return nReturn;
      case TYPE_REAL   :
         nReturn = (double)*((float*)(pPointer));
         return nReturn;
      case TYPE_DECIMAL:
      case TYPE_NUMERIC:
      case TYPE_DOUBLE :
      case TYPE_FLOAT  :
         nReturn = (double)*((double*)(pPointer));
         return nReturn;
      case TYPE_WCHAR:
      case TYPE_WVARCHAR:
         nReturn = wcstod((WCHAR*)pPointer,NULL);
         return nReturn;
      case TYPE_DATE:
      {
         SQL_DATE_STRUCT *dDate = reinterpret_cast<SQL_DATE_STRUCT*>(pPointer);
         SQL_DATETIME_STRUCT dDateTime;
         SgZeroMemory(&dDateTime,sizeof(SQL_DATETIME_STRUCT));
         dDateTime.day = dDate->day;
         dDateTime.month = dDate->month;
         dDateTime.year = dDate->year;
         nReturn = DateTimeToDouble(&dDateTime);
         return nReturn;
      }
      case TYPE_TIMESTAMP:
         nReturn = DateTimeToDouble(reinterpret_cast<SQL_DATETIME_STRUCT*>(pPointer));
         return nReturn;
      case TYPE_STRINGA:
         nReturn = strtod(reinterpret_cast<AnsiString*>(pPointer)->data(),NULL);
         return nReturn;
      case TYPE_STRINGW:
         nReturn = wcstod(reinterpret_cast<WideString*>(pPointer)->data(),NULL);
         return nReturn;
      default: //nothing
         break;
   }
   return nReturn;
}
//---------------------------------------------------------------------------
bool DBBind::AsBoolean()
{
   int nReturn = AsInteger();
   return (nReturn != 0) ? true : false;
}
//---------------------------------------------------------------------------
SQL_DATETIME_STRUCT DBBind::AsDateTime()
{
   DataType eDataType = oType->getDataType();
   SQL_DATETIME_STRUCT dReturn;
   SgZeroMemory(&dReturn,sizeof(SQL_DATETIME_STRUCT));

   if (IsNull())
      return dReturn;

   switch(eDataType) {
      case TYPE_SSMALLINT:
      case TYPE_USMALLINT:
      case TYPE_SINTEGER :
      case TYPE_UINTEGER :
      case TYPE_SBIGINT  :
      case TYPE_UBIGINT  :
      {
         int nReturn = 0;
         if (eDataType == TYPE_SSMALLINT)
            nReturn = (int)*((SQLSMALLINT*)(pPointer));
         else if (eDataType == TYPE_USMALLINT)
            nReturn = (int)*((SQLUSMALLINT*)(pPointer));
         else if (eDataType == TYPE_SINTEGER)
            nReturn = (int)*((SQLINTEGER*)(pPointer));
         else if (eDataType == TYPE_UINTEGER)
            nReturn = (int)*((SQLUINTEGER*)(pPointer));
         else if (eDataType == TYPE_SBIGINT)
            nReturn = (int)*((SQLBIGINT*)(pPointer));
         else if (eDataType == TYPE_UBIGINT)
            nReturn = (int)*((SQLUBIGINT*)(pPointer));
         DoubleToDateTime((double)nReturn,&dReturn);
         return dReturn;
      }
      case TYPE_REAL   :
      case TYPE_DECIMAL:
      case TYPE_NUMERIC:
      case TYPE_DOUBLE :
      case TYPE_FLOAT  :
      {
         double nReturn = 0;
         if (eDataType == TYPE_REAL)
            nReturn = (double)*((float*)(pPointer));
         else
            nReturn = (double)*((double*)(pPointer));
         DoubleToDateTime(nReturn,&dReturn);
         return dReturn;
      }
      case TYPE_DATE:
      {
         SQL_DATE_STRUCT *dDate = reinterpret_cast<SQL_DATE_STRUCT*>(pPointer);
         dReturn.day = dDate->day;
         dReturn.month = dDate->month;
         dReturn.year = dDate->year;
         return dReturn;
      }
      case TYPE_TIME:
      {
         SQL_TIME_STRUCT *dTime = reinterpret_cast<SQL_TIME_STRUCT*>(pPointer);
         dReturn.hour = dTime->hour;
         dReturn.minute = dTime->minute;
         dReturn.second = dTime->second;
         return dReturn;
      }
      case TYPE_TIMESTAMP:
         memcpy(&dReturn,pPointer,sizeof(SQL_DATETIME_STRUCT));
         return dReturn;
      default: //nothing
         break;
   }
   return dReturn;
}
//---------------------------------------------------------------------------
SQLRETURN DBBind::CopyValue(IDBBind* oBind)
{
   DataType eDataType = oBind->getDataType();

   if (this->IsNull()) {
      return oBind->SetNull();
   }

   switch(eDataType) {
      case TYPE_CHAR        :
      case TYPE_VARCHAR     :
      case TYPE_LONGVARCHAR :
      case TYPE_WCHAR       :
      case TYPE_WVARCHAR    :
      case TYPE_WLONGVARCHAR:
      case TYPE_STRINGA     :
      case TYPE_STRINGW     :
      {
         String Value = this->AsString();
         return oBind->SetValue(Value.data(),Value.length());
      } break;

      case TYPE_BIT      :
         return oBind->SetValue(this->AsBoolean());
         break;

      case TYPE_STINYINT :
      case TYPE_UTINYINT :
      case TYPE_SSMALLINT:
      case TYPE_USMALLINT:
      case TYPE_SINTEGER :
      case TYPE_UINTEGER :
         return oBind->SetValue(this->AsInteger());
         break;

      case TYPE_SBIGINT:
      case TYPE_UBIGINT:
      {
         __int64 Value = *((__int64*)this->pPointer);
         return oBind->SetValue(Value);
      } break;

      case TYPE_REAL   :
      case TYPE_DECIMAL:
      case TYPE_NUMERIC:
      case TYPE_DOUBLE :
      case TYPE_FLOAT  :
         return oBind->SetValue((double)this->AsDouble(),this->getDecimalDigits());
         break;

      case TYPE_BINARY   :
      case TYPE_VARBINARY:
         return oBind->SetValue((char*)(this->pPointer),nRetLength);
         break;

      case TYPE_BLOB         :
      case TYPE_LONGVARBINARY:
         return oBind->SetValue(reinterpret_cast<Blob*>(pPointer));
         break;

      case TYPE_DATE     :
      case TYPE_TIME     :
      case TYPE_TIMESTAMP:
         return oBind->SetValue(this->AsDateTime());
         break;

       default:
         SetExceptionInfo(_T("DBBind::CopyValue()"),_T("Destination field data type don't suport CopyValue"),_T("HY004"),_T("Invalid SQL data type"),0);
         return SQL_ERROR;
   }
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN DBBind::operator<<(const char* Value) {return SetValue(Value,strlen(Value));}
//---------------------------------------------------------------------------
SQLRETURN DBBind::operator<<(const WCHAR* Value) {return SetValue(Value,wcslen(Value));}
//---------------------------------------------------------------------------
SQLRETURN DBBind::operator<<(const bool Value) {return SetValue(Value);}
//---------------------------------------------------------------------------
SQLRETURN DBBind::operator<<(const short Value) {return SetValue(Value);}
//---------------------------------------------------------------------------
SQLRETURN DBBind::operator<<(const int Value) {return SetValue(Value);}
//---------------------------------------------------------------------------
SQLRETURN DBBind::operator<<(const long Value) {return SetValue(Value);}
//---------------------------------------------------------------------------
SQLRETURN DBBind::operator<<(const __int64 Value) {return SetValue(Value);}
//---------------------------------------------------------------------------
SQLRETURN DBBind::operator<<(const float Value) {return SetValue(Value);}
//---------------------------------------------------------------------------
SQLRETURN DBBind::operator<<(const double Value) {return SetValue(Value);}
//---------------------------------------------------------------------------
SQLRETURN DBBind::operator<<(const SQL_DATE_STRUCT Value) {return SetValue(Value);}
//---------------------------------------------------------------------------
SQLRETURN DBBind::operator<<(const SQL_TIME_STRUCT Value) {return SetValue(Value);}
//---------------------------------------------------------------------------
SQLRETURN DBBind::operator<<(const SQL_DATETIME_STRUCT Value) {return SetValue(Value);}
//---------------------------------------------------------------------------
SQLRETURN DBBind::operator<<(const AnsiString* Value) {return SetValue(Value);}
//---------------------------------------------------------------------------
SQLRETURN DBBind::operator<<(const WideString* Value) {return SetValue(Value);}
//---------------------------------------------------------------------------
SQLRETURN DBBind::operator<<(const Blob* Value) {return SetValue(Value);}
//---------------------------------------------------------------------------
SQLRETURN DBBind::operator<<(const AnsiString &Value) {return SetValue(Value);}
//---------------------------------------------------------------------------
SQLRETURN DBBind::operator<<(const WideString &Value) {return SetValue(Value);}
//---------------------------------------------------------------------------
SQLRETURN DBBind::operator<<(const Blob &Value) {return SetValue(Value);}
//---------------------------------------------------------------------------
}; //end namespace sfg
//---------------------------------------------------------------------------

