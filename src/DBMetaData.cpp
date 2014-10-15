//---------------------------------------------------------------------------
#include <map>
#include <boost/thread/mutex.hpp>
#include "DBMetaData.hpp"

namespace sfg {

//---------------------------------------------------------------------------
typedef std::map<String,DBMetaTable*> DBMetaTables;
class DBConnTables : public std::map<IDBConnection*,DBMetaTables*>
{
public:
   DBConnTables();
   ~DBConnTables();
};
static boost::mutex oMutex;
static DBConnTables oConnTables;
//---------------------------------------------------------------------------
DBConnTables::DBConnTables() : std::map<IDBConnection*,DBMetaTables*>(){}
//---------------------------------------------------------------------------
DBConnTables::~DBConnTables()
{
   DBConnTables::iterator iMetaTables = this->begin();
   for(; iMetaTables != this->end(); ++iMetaTables) {
      DBMetaTables::iterator iMetaTable = iMetaTables->second->begin();
      for(; iMetaTable != iMetaTables->second->end(); ++iMetaTable)
         delete iMetaTable->second;
      delete iMetaTables->second;
   }
}
//---------------------------------------------------------------------------
IDBMetaTable* GetDBMetaTable1(
      IDBConnection* oConnection) {
   return new DBMetaTable(oConnection);
}
IDBMetaTable* GetDBMetaTable2(
      IDBConnection* oConnection, const String& sTableName, bool bCreateFields) {
   return new DBMetaTable(oConnection,sTableName,bCreateFields);
}
//---------------------------------------------------------------------------
DBMetaTable::DBMetaTable(IDBConnection* oConnection) : 
      DBObject(), IDBMetaTable()
{
   this->oConnection = dynamic_cast<DBConnection*>(oConnection);
   this->sTableName = _T("");
   this->oMetaFields = new DBMetaFields(this);
   this->oMetaFields->bAutoGenerate = true;
}
//---------------------------------------------------------------------------
DBMetaTable::DBMetaTable(IDBConnection* oConnection, const String& sTableName, bool bCreateFields) :
      DBObject(), IDBMetaTable()
{
   this->oConnection = dynamic_cast<DBConnection*>(oConnection);
   this->oMetaFields = NULL;
   if (bCreateFields) {
      this->setTableName(sTableName);
   } else {
      this->sTableName = sTableName;
      this->oMetaFields = new DBMetaFields(this);
      this->oMetaFields->bAutoGenerate = true;
   }      
}
//---------------------------------------------------------------------------
DBMetaTable::~DBMetaTable()
{
   if (this->oMetaFields != NULL) {
      if (this->oMetaFields->bAutoGenerate)
         delete this->oMetaFields;
   }
}
//---------------------------------------------------------------------------
void DBMetaTable::setMetaFields(IDBMetaFields* oFields) {
   if (this->oMetaFields != NULL) {
      if (this->oMetaFields->bAutoGenerate)
         delete this->oMetaFields;
   }
   this->oMetaFields = dynamic_cast<DBMetaFields*>(oMetaFields);
   this->oMetaFields->oMetaTable = this;
}
//---------------------------------------------------------------------------
IDBMetaFields* DBMetaTable::getMetaFields() {
   return oMetaFields;
}
//---------------------------------------------------------------------------
IDBConnection* DBMetaTable::getConnection() {
   return oConnection;
}
//---------------------------------------------------------------------------
IDBMetaField* DBMetaTable::FieldByIndex(int nIndex) {
   return oMetaFields->FieldByIndex(nIndex);
}
//---------------------------------------------------------------------------
IDBMetaField* DBMetaTable::FieldByName(const String& sName) {
   return oMetaFields->FieldByName(sName);
}
//---------------------------------------------------------------------------
const String& DBMetaTable::getTableName() {
   return sTableName;
}
//---------------------------------------------------------------------------
SQLRETURN DBMetaTable::setTableName(const String& Value)
{
   SQLRETURN nRetcode = SQL_SUCCESS;

   if (sTableName == Value)
      return SQL_SUCCESS;
   sTableName = Value;
   //Deve excluir dos componentes todos os campos já existentes
   if (oMetaFields != NULL) {
      if (oMetaFields->bAutoGenerate) {
         delete oMetaFields;
         oMetaFields = NULL;
      }
   }
   if (oMetaFields == NULL) {
      oMetaFields = new DBMetaFields(this);
      oMetaFields->bAutoGenerate = true;
   }
   
   //Deve carregar do metadados todos os campos da tabela
   DBMetaField* oMetaField = NULL;
   int nProperties = 0;

   DBRecordset *QyMetaData = new DBRecordset(oConnection);
   nRetcode = QyMetaData->Open(_T("\
SELECT column_order, column_name, column_datatype, column_typename, column_size, \
       decimal_digits, column_label, column_editmask, column_properties, \
       column_checkvalues, column_editcmp, column_format, fk_table_name, fk_column_name\
  FROM tbl_meta_fiedls\
 WHERE table_name = '") + sTableName + TEXT("'\
 ORDER BY column_order"));
   if (!RC_SUCCESS(nRetcode)) 
      return nRetcode;
   nRetcode = QyMetaData->MoveFirst();
   if (!RC_SUCCESS(nRetcode)) 
      return nRetcode;
   while (!QyMetaData->getEOF()) {
      oMetaField = new DBMetaField(oMetaFields);
      oMetaFields->oListFields->push_back(oMetaField);
      oMetaField->sFieldName = QyMetaData->FieldByName(_T("column_name"))->AsString();
      oMetaField->sFieldLabel = QyMetaData->FieldByName(_T("column_label"))->AsString();
      oMetaField->sEditMask = QyMetaData->FieldByName(_T("column_editmask"))->AsString();
      oMetaField->sCheckValues = QyMetaData->FieldByName(_T("column_checkvalues"))->AsString();
      /*oMetaField->eDataType = ConvertDbcToDatasetType(
                            ConvertMySQLTypeToSQLType(
                            QyMetaData->FieldByName(_T("column_datatype"))->AsString,
                            QyMetaData->FieldByName(_T("column_typename"))->AsString));
      if (oMetaField->eDataType == ftString ||
         oMetaField->eDataType == ftMemo ||
         oMetaField->eDataType == ftFmtMemo ||
         oMetaField->eDataType == ftVarBytes ||
         oMetaField->eDataType == ftBytes ||
         oMetaField->eDataType == ftBlob ||
         oMetaField->eDataType == ftGraphic) {
         oMetaField->Size = QyMetaData->FieldByName(_T("column_size"))->AsInteger;
         oMetaField->nDecimalDigits = 0;
      } else
      if (oMetaField->eDataType == ftBCD ||
         oMetaField->eDataType == ftFMTBcd) {
         oMetaField->Size = QyMetaData->FieldByName(_T("column_precision")))->AsInteger;
         oMetaField->nDecimalDigits = QyMetaData->FieldByName(_T("column_size")))->AsInteger;
      } else {
         oMetaField->Size = 0;
         oMetaField->nDecimalDigits = 15;
      }*/
      oMetaField->nDataSize = QyMetaData->FieldByName(_T("column_size"))->AsInteger();
      oMetaField->nDecimalDigits = QyMetaData->FieldByName(_T("decimal_digits"))->AsInteger();
      oMetaField->nProperties = QyMetaData->FieldByName(_T("column_properties"))->AsInteger();
      oMetaField->eEditCmp = (MetaFieldEdit)QyMetaData->FieldByName(_T("column_editcmp"))->AsInteger();
      oMetaField->sFkTableName = QyMetaData->FieldByName(_T("fk_table_name"))->AsString();
      oMetaField->sFkColumnName = QyMetaData->FieldByName(_T("fk_column_name"))->AsString();
      nRetcode = QyMetaData->MoveNext();
      if (!RC_SUCCESS(nRetcode)) 
         return nRetcode;
   }
   return nRetcode;
}
//---------------------------------------------------------------------------
IDBMetaField* GetDBMetaField1(
      IDBMetaFields* oMetaFields) {
   return new DBMetaField(oMetaFields);
}
IDBMetaField* GetDBMetaField2(
      IDBMetaFields* oMetaFields, int nFieldNum, const String& sFieldName, const String& sFieldLabel,
      MetaFieldEdit eEditCmp, DataType eDataType, int nDataSize, int nDecimalDigits, 
      bool bNotNull, bool bIsPkKey, bool bAutoIncrement, int nProperties,
      const String& sEditMask, const String& sCheckValues, 
      const String& sFkTableName, const String& sFkColumnName) {
   return new DBMetaField(oMetaFields,nFieldNum,sFieldName,sFieldLabel,
      eEditCmp,eDataType,nDataSize,nDecimalDigits, 
      bNotNull,bIsPkKey,bAutoIncrement,nProperties,
      sEditMask,sCheckValues,sFkTableName,sFkColumnName);
}
//---------------------------------------------------------------------------
DBMetaField::DBMetaField(IDBMetaFields* oMetaFields) : 
      DBObject(), IDBMetaField()
{
   this->oMetaFields = dynamic_cast<DBMetaFields*>(oMetaFields);
   this->nFieldNum = 0;
   this->sFieldName = _T("");
   this->sFieldLabel = _T("");
   this->eDataType = TYPE_UNKNOWN;
   this->nDataSize = 0;
   this->nDecimalDigits = 0;
   this->nProperties = 0;
   this->eEditCmp = MFE_MASK_EDIT;
   this->sEditMask = _T("");
   this->sCheckValues = _T("1;0");
   this->sFkTableName = _T("");
   this->sFkColumnName = _T("");
   this->bIsVirtual = true;
}
//---------------------------------------------------------------------------
DBMetaField::DBMetaField(IDBMetaFields* oMetaFields, 
      int nFieldNum, const String& sFieldName, const String& sFieldLabel,
      MetaFieldEdit eEditCmp, DataType eDataType, int nDataSize, int nDecimalDigits, 
      bool bNotNull, bool bIsPkKey, bool bAutoIncrement, int nProperties,
      const String& sEditMask, const String& sCheckValues, 
      const String& sFkTableName, const String& sFkColumnName)
{
   this->oMetaFields = dynamic_cast<DBMetaFields*>(oMetaFields);
   this->nFieldNum = nFieldNum;
   this->sFieldName = sFieldName;
   this->sFieldLabel = sFieldLabel;
   this->eEditCmp = eEditCmp;
   this->eDataType = eDataType;
   this->nDataSize = nDataSize;
   this->nDecimalDigits = nDecimalDigits;
   this->nProperties = nProperties;
   this->setNotNull(bNotNull);
   this->setIsPkKey(bIsPkKey);
   this->setAutoIncrement(bAutoIncrement);
   this->sEditMask = sEditMask;
   this->sCheckValues = sCheckValues;
   this->sFkTableName = sFkTableName;
   this->sFkColumnName = sFkColumnName;
   if (sFkColumnName.length() > 0)
      this->setIsFkKey(true);
   this->bIsVirtual = false;
}
//---------------------------------------------------------------------------
DBMetaField::~DBMetaField() {}
//---------------------------------------------------------------------------
IDBMetaFields* DBMetaField::getMetaFields() {
   return oMetaFields;
}
//---------------------------------------------------------------------------
void DBMetaField::setNotNull(const bool Value) {
   nProperties = (Value) ? (nProperties|MFA_NOTNULL) : (nProperties&~MFA_NOTNULL);
}
//---------------------------------------------------------------------------
void DBMetaField::setAutoIncrement(const bool Value) {
   nProperties = (Value) ? (nProperties|MFA_AUTOINC) : (nProperties&~MFA_AUTOINC);
}
//---------------------------------------------------------------------------
void DBMetaField::setIsPkKey(const bool Value) {
   nProperties = (Value) ? (nProperties|MFA_PKKEY) : (nProperties&~MFA_PKKEY);
}
//---------------------------------------------------------------------------
void DBMetaField::setIsFkKey(const bool Value) {
   nProperties = (Value) ? (nProperties|MFA_FKKEY) : (nProperties&~MFA_FKKEY);
}
//---------------------------------------------------------------------------
void DBMetaField::setIsIndex(const bool Value) {
   nProperties = (Value) ? (nProperties|MFA_INDEX) : (nProperties&~MFA_INDEX);
}
//---------------------------------------------------------------------------
void DBMetaField::setIsFkList(const bool Value) {
   nProperties = (Value) ? (nProperties|MFA_FKLIST) : (nProperties&~MFA_FKLIST);
}
//---------------------------------------------------------------------------
void DBMetaField::setIsBrowse(const bool Value) {
   nProperties = (Value) ? (nProperties|MFA_BROWSE) : (nProperties&~MFA_BROWSE);
}
//---------------------------------------------------------------------------
void DBMetaField::setIsFilter(const bool Value) {
   nProperties = (Value) ? (nProperties|MFA_FILTER) : (nProperties&~MFA_FILTER);
}
//---------------------------------------------------------------------------
void DBMetaField::setIsSearch(const bool Value) {
   nProperties = (Value) ? (nProperties|MFA_SEARCH) : (nProperties&~MFA_SEARCH);
}
//---------------------------------------------------------------------------
void DBMetaField::setIsOrder(const bool Value) {
   nProperties = (Value) ? (nProperties|MFA_ORDER) : (nProperties&~MFA_ORDER);
}
//---------------------------------------------------------------------------
void DBMetaField::setIsMoney(const bool Value) {
   nProperties = (Value) ? (nProperties|MFA_MONEY) : (nProperties&~MFA_MONEY);
}
//---------------------------------------------------------------------------
IDBMetaFields* GetDBMetaFields1(
      IDBMetaTable* oMetaTable) {
   return new DBMetaFields(oMetaTable);
}
//---------------------------------------------------------------------------
DBMetaFields::DBMetaFields(IDBMetaTable* oMetaTable) : 
      DBObject(), IDBMetaFields()
{
   this->oMetaTable = dynamic_cast<DBMetaTable*>(oMetaTable);
   this->oListFields = new DBMetaFieldList();
   this->bAutoGenerate = false;
}
//---------------------------------------------------------------------------
DBMetaFields::~DBMetaFields()
{
   if(oListFields != NULL)
   {
      DBMetaFieldList::iterator iMetaField = oListFields->begin();
      for(; iMetaField != oListFields->end(); ++iMetaField)
         delete (*iMetaField);
      delete oListFields;
   }
}
//---------------------------------------------------------------------------
IDBMetaTable* DBMetaFields::getMetaTable() {
   return oMetaTable;
}
//---------------------------------------------------------------------------
IDBMetaField* DBMetaFields::FieldByIndex(int nIndex)
{
   if((size_t)nIndex >= oListFields->size()) {
      return NULL;
   } else {
      return oListFields->at(nIndex);
   }
}
//---------------------------------------------------------------------------
IDBMetaField* DBMetaFields::FieldByName(const String& sName)
{
   DBMetaFieldList::iterator iMetaField = oListFields->begin();
   for(; iMetaField != oListFields->end(); ++iMetaField)
      if(sName == ((*iMetaField)->sFieldName))
         return (*iMetaField);
   return NULL;
}
//---------------------------------------------------------------------------
IDBMetaField* DBMetaFields::Add(IDBMetaField* oMetaField)
{
   DBMetaField* oCastField = dynamic_cast<DBMetaField*>(oMetaField);
   oCastField->oMetaFields = this;
   oListFields->push_back(oCastField);
   oCastField->nFieldNum = oListFields->size();

   return oMetaField;
}
//---------------------------------------------------------------------------
IDBMetaField* DBMetaFields::Insert(int nIndex, IDBMetaField* oMetaField)
{
   DBMetaField* oCastField = dynamic_cast<DBMetaField*>(oMetaField);
   oCastField->oMetaFields = this;
   oCastField->nFieldNum = nIndex + 1;
   //Se inclui como o último item da lista
   if ((oListFields->size() == 0) || (oListFields->size() <= (size_t)nIndex)) {
      oListFields->push_back(oCastField);
      oCastField->nFieldNum = oListFields->size();
   } else { //Inclui no meio da lista e reordena a numeração dos parâmetros
      DBMetaFieldList::iterator iMetaField = oListFields->begin() + nIndex;

      oListFields->insert(iMetaField, oCastField);

      iMetaField = oListFields->begin() + (nIndex + 1);
      while (iMetaField != oListFields->end()) {
         (*iMetaField)->nFieldNum++;
         iMetaField++;
      }
   }

   return oMetaField;
}
//---------------------------------------------------------------------------
IDBMetaField* DBMetaFields::Remove(int nIndex)
{
   DBMetaField* oMetaField = NULL;
   
   if(oListFields == NULL)
      return oMetaField;

   if((size_t)nIndex >= oListFields->size())
      return oMetaField;

   //Se retira do final da lista, apenas exclui
   if((oListFields->size() - 1) == (size_t)nIndex) {
      oMetaField = oListFields->at(oListFields->size()-1);
      oListFields->pop_back();
   } else { //Retira do meio da lista e reordena a numeração dos parâmetros
      DBMetaFieldList::iterator iMetaField = oListFields->begin() + nIndex;
      oMetaField = (*iMetaField);
      oListFields->erase(iMetaField);

      iMetaField = oListFields->begin() + nIndex;
      while (iMetaField != oListFields->end()) {
         (*iMetaField)->nFieldNum--;
         iMetaField++;
      }
   }
   oMetaField->nFieldNum = 0;
   oMetaField->oMetaFields = NULL;
   return oMetaField;
}
//---------------------------------------------------------------------------
int DBMetaFields::Count()
{
   if(oListFields == NULL)
      return 0;
   else
      return oListFields->size();
}
//---------------------------------------------------------------------------
DBRecordsetSchema::DBRecordsetSchema(IDBConnection* oConnection)
		: DBRecordset(oConnection), IDBRecordsetSchema(), bMetadataID(false) 
{}
//---------------------------------------------------------------------------
SQLRETURN DBRecordsetSchema::Initialize()
{
	nRetcode = SQL_SUCCESS;
   if (!DBStmt::IsAllocated()) {
      nRetcode = DBStmt::Initialize(false);
		if (!RC_SUCCESS(nRetcode))
         return nRetcode;
   }

   //Deve fazer desta forma, pois se o driver não suporta esta opção gera um erro HYC00
   /*SQLRETURN nRetcode = SQLSetStmtAttr(this->getHSTMT(), SQL_ATTR_METADATA_ID, (SQLPOINTER)SQL_TRUE, SQL_IS_INTEGER);
   if (!RC_SUCCESS(nRetcode)) {
      SgOStringStream oErrMsg;
      oErrMsg << _T("Unable to set statement attribute for ")
                 << _T("Attribute: ") << SQL_ATTR_METADATA_ID << _T(_T(" Value: ") << SQL_TRUE;
      try {
         ExceptionByHandle(SQL_HANDLE_STMT, _T("CreateRecordsetTables"),
               oErrMsg.str(), oConnection->getEnvironment(), oConnection, this);
      } catch(DBException oException) {
         if (oException.getSQLState() == String(_T("HYC00")))
            this->bMetadataID = false;
         else
            throw oException;
      }
   } else
      this->bMetadataID = true;*/
	return nRetcode;
}
//---------------------------------------------------------------------------
DBRecordsetSchema* CreateRecordsetTables(IDBConnection* oConnection)
{
   DBRecordsetSchema* oRecordset = new DBRecordsetSchema(oConnection);
   DBFields* oFields = new DBFields(oRecordset, false);

   oFields->Add(new DBField(oFields, 1 /*nFieldNum*/,_T("TABLE_CAT"),TYPE_TVARCHAR,oConnection->getMaxCatalogName()));
   oFields->Add(new DBField(oFields, 2 /*nFieldNum*/,_T("TABLE_SCHEM"),TYPE_TVARCHAR,oConnection->getMaxSchemaName()));
   oFields->Add(new DBField(oFields, 3 /*nFieldNum*/,_T("TABLE_NAME"),TYPE_TVARCHAR,oConnection->getMaxTableName()));
   oFields->Add(new DBField(oFields, 4 /*nFieldNum*/,_T("TABLE_TYPE"),TYPE_TVARCHAR,128));
   oFields->Add(new DBField(oFields, 5 /*nFieldNum*/,_T("REMARKS"),TYPE_STRING,255));
   oRecordset->SetStmtAttr(SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_STATIC, SQL_IS_INTEGER);
   //oRecordset->SetStmtAttr(SQL_ATTR_CURSOR_SCROLLABLE, (SQLPOINTER)SQL_SCROLLABLE, SQL_IS_INTEGER);
   oRecordset->SetStmtAttr(SQL_ATTR_CONCURRENCY, (SQLPOINTER)SQL_CONCUR_READ_ONLY, SQL_IS_INTEGER);
   oRecordset->Initialize();
   oFields->BindFields();

   return oRecordset;
}
//---------------------------------------------------------------------------
DBRecordsetSchema* CreateRecordsetColumns(IDBConnection* oConnection)
{
   DBRecordsetSchema* oRecordset = new DBRecordsetSchema(oConnection);
   DBFields* oFields = new DBFields(oRecordset, false);

   oFields->Add(new DBField(oFields, 1 /*nFieldNum*/,_T("TABLE_CAT"),TYPE_TVARCHAR,oConnection->getMaxCatalogName()));
   oFields->Add(new DBField(oFields, 2 /*nFieldNum*/,_T("TABLE_SCHEM"),TYPE_TVARCHAR,oConnection->getMaxSchemaName()));
   oFields->Add(new DBField(oFields, 3 /*nFieldNum*/,_T("TABLE_NAME"),TYPE_TVARCHAR,oConnection->getMaxTableName(),0,true));
   oFields->Add(new DBField(oFields, 4 /*nFieldNum*/,_T("COLUMN_NAME"),TYPE_TVARCHAR,oConnection->getMaxColumnName(),0,true));
   oFields->Add(new DBField(oFields, 5 /*nFieldNum*/,_T("DATA_TYPE"),TYPE_SSMALLINT,5,0,true));
   oFields->Add(new DBField(oFields, 6 /*nFieldNum*/,_T("TYPE_NAME"),TYPE_TVARCHAR,128,0,true));
   oFields->Add(new DBField(oFields, 7 /*nFieldNum*/,_T("COLUMN_SIZE"),TYPE_SINTEGER,10));
   oFields->Add(new DBField(oFields, 8 /*nFieldNum*/,_T("BUFFER_LENGTH"),TYPE_SINTEGER,10));
   oFields->Add(new DBField(oFields, 9 /*nFieldNum*/,_T("DECIMAL_DIGITS"),TYPE_SSMALLINT,5));
   oFields->Add(new DBField(oFields,10 /*nFieldNum*/,_T("NUM_PREC_RADIX"),TYPE_SSMALLINT,5));
   oFields->Add(new DBField(oFields,11 /*nFieldNum*/,_T("NULLABLE"),TYPE_SSMALLINT,5,0,true));
   oFields->Add(new DBField(oFields,12 /*nFieldNum*/,_T("REMARKS"),TYPE_STRING,255));
   oFields->Add(new DBField(oFields,13 /*nFieldNum*/,_T("COLUMN_DEF"),TYPE_STRING,255));
   oFields->Add(new DBField(oFields,14 /*nFieldNum*/,_T("SQL_DATA_TYPE"),TYPE_SSMALLINT,5,0,true));
   oFields->Add(new DBField(oFields,15 /*nFieldNum*/,_T("SQL_DATETIME_SUB"),TYPE_SSMALLINT,5));
   oFields->Add(new DBField(oFields,16 /*nFieldNum*/,_T("CHAR_OCTET_LENGTH"),TYPE_SINTEGER,10));
   oFields->Add(new DBField(oFields,17 /*nFieldNum*/,_T("ORDINAL_POSITION"),TYPE_SINTEGER,10,0,true));
   oFields->Add(new DBField(oFields,18 /*nFieldNum*/,_T("IS_NULLABLE"),TYPE_TVARCHAR,3));

   oRecordset->SetStmtAttr(SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_STATIC, SQL_IS_INTEGER);
   //oRecordset->SetStmtAttr(SQL_ATTR_CURSOR_SCROLLABLE, (SQLPOINTER)SQL_SCROLLABLE, SQL_IS_INTEGER);
   oRecordset->SetStmtAttr(SQL_ATTR_CONCURRENCY, (SQLPOINTER)SQL_CONCUR_READ_ONLY, SQL_IS_INTEGER);
   oRecordset->Initialize();
   oFields->BindFields();

   return oRecordset;
}
//---------------------------------------------------------------------------
DBRecordsetSchema* CreateRecordsetPrimaryKeys(IDBConnection* oConnection)
{
   DBRecordsetSchema* oRecordset = new DBRecordsetSchema(oConnection);
   DBFields* oFields = new DBFields(oRecordset, false);

   oFields->Add(new DBField(oFields, 1 /*nFieldNum*/,_T("TABLE_CAT"),TYPE_TVARCHAR,oConnection->getMaxCatalogName()));
   oFields->Add(new DBField(oFields, 2 /*nFieldNum*/,_T("TABLE_SCHEM"),TYPE_TVARCHAR,oConnection->getMaxSchemaName()));
   oFields->Add(new DBField(oFields, 3 /*nFieldNum*/,_T("TABLE_NAME"),TYPE_TVARCHAR,oConnection->getMaxTableName(),0,true));
   oFields->Add(new DBField(oFields, 4 /*nFieldNum*/,_T("COLUMN_NAME"),TYPE_TVARCHAR,oConnection->getMaxColumnName(),0,true));
   oFields->Add(new DBField(oFields, 5 /*nFieldNum*/,_T("ORDINAL_POSITION"),TYPE_SSMALLINT,5,0,true));
   oFields->Add(new DBField(oFields, 6 /*nFieldNum*/,_T("PK_NAME"),TYPE_TVARCHAR,128));

   oRecordset->SetStmtAttr(SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_STATIC, SQL_IS_INTEGER);
   //oRecordset->SetStmtAttr(SQL_ATTR_CURSOR_SCROLLABLE, (SQLPOINTER)SQL_SCROLLABLE, SQL_IS_INTEGER);
   oRecordset->SetStmtAttr(SQL_ATTR_CONCURRENCY, (SQLPOINTER)SQL_CONCUR_READ_ONLY, SQL_IS_INTEGER);
   oRecordset->Initialize();
   oFields->BindFields();

   return oRecordset;
}
//---------------------------------------------------------------------------
DBRecordsetSchema* CreateRecordsetForeignKeys(IDBConnection* oConnection)
{
   DBRecordsetSchema* oRecordset = new DBRecordsetSchema(oConnection);
   DBFields* oFields = new DBFields(oRecordset, false);

   oFields->Add(new DBField(oFields, 1 /*nFieldNum*/,_T("PKTABLE_CAT"),TYPE_TVARCHAR,oConnection->getMaxCatalogName()));
   oFields->Add(new DBField(oFields, 2 /*nFieldNum*/,_T("PKTABLE_SCHEM"),TYPE_TVARCHAR,oConnection->getMaxSchemaName()));
   oFields->Add(new DBField(oFields, 3 /*nFieldNum*/,_T("PKTABLE_NAME"),TYPE_TVARCHAR,oConnection->getMaxTableName(),0,true));
   oFields->Add(new DBField(oFields, 4 /*nFieldNum*/,_T("PKCOLUMN_NAME"),TYPE_TVARCHAR,oConnection->getMaxColumnName(),0,true));
   oFields->Add(new DBField(oFields, 5 /*nFieldNum*/,_T("FKTABLE_CAT"),TYPE_TVARCHAR,oConnection->getMaxCatalogName()));
   oFields->Add(new DBField(oFields, 6 /*nFieldNum*/,_T("FKTABLE_SCHEM"),TYPE_TVARCHAR,oConnection->getMaxSchemaName()));
   oFields->Add(new DBField(oFields, 7 /*nFieldNum*/,_T("FKTABLE_NAME"),TYPE_TVARCHAR,oConnection->getMaxTableName(),0,true));
   oFields->Add(new DBField(oFields, 8 /*nFieldNum*/,_T("FKCOLUMN_NAME"),TYPE_TVARCHAR,oConnection->getMaxColumnName(),0,true));
   oFields->Add(new DBField(oFields, 9 /*nFieldNum*/,_T("KEY_SEQ"),TYPE_SSMALLINT,5,0,true));
   oFields->Add(new DBField(oFields,10 /*nFieldNum*/,_T("UPDATE_RULE"),TYPE_SSMALLINT,5));
   oFields->Add(new DBField(oFields,11 /*nFieldNum*/,_T("DELETE_RULE"),TYPE_SSMALLINT,5));
   oFields->Add(new DBField(oFields,12 /*nFieldNum*/,_T("FK_NAME"),TYPE_TVARCHAR,128));
   oFields->Add(new DBField(oFields,13 /*nFieldNum*/,_T("PK_NAME"),TYPE_TVARCHAR,128));
   oFields->Add(new DBField(oFields,14 /*nFieldNum*/,_T("DEFERRABILITY"),TYPE_SSMALLINT,5));

   oRecordset->SetStmtAttr(SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_STATIC, SQL_IS_INTEGER);
   //oRecordset->SetStmtAttr(SQL_ATTR_CURSOR_SCROLLABLE, (SQLPOINTER)SQL_SCROLLABLE, SQL_IS_INTEGER);
   oRecordset->SetStmtAttr(SQL_ATTR_CONCURRENCY, (SQLPOINTER)SQL_CONCUR_READ_ONLY, SQL_IS_INTEGER);
   oRecordset->Initialize();
   oFields->BindFields();

   return oRecordset;
}
//---------------------------------------------------------------------------
String EscapeUnderscore(String& sStrValue)
{
   size_t nLength = sStrValue.size();
   const TCHAR *cStrValue = sStrValue.c_str();
   TCHAR cLast = 0;
   String sStrReturn = _T("");

   for (size_t i = 0, j = 0; i < nLength; i++, j++) {
      if (cStrValue[i] == _T('_')) {
         if (cLast == _T('\\')) {
            sStrReturn += cStrValue[i];
         } else {
            sStrReturn += _T('\\');
            sStrReturn += cStrValue[i];
         }
      } else {
         sStrReturn += cStrValue[i];
      }
      cLast = cStrValue[i];
   }
   return sStrReturn;
}
//---------------------------------------------------------------------------
bool ExistTable(IDBConnection* oConnection, const String& sTableNameC)
{
   bool bReturn = false;
   SQLRETURN nRetcode = SQL_SUCCESS;
   DBRecordsetSchema* oRecordset = CreateRecordsetTables(oConnection);
   String sCatalogName = oConnection->getCatalogName();
   String sSchemaName = oConnection->getSchemaName();
   String sTableName = sTableNameC;
   SQLTCHAR* cSchemaName = NULL;
   SQLSMALLINT nSchemaLength = 0;
   SQLTCHAR* cCatalogName = NULL;
   SQLSMALLINT nCatalogLength = 0;

   if (!oRecordset->getMetadataID()) {
      if (sCatalogName.length())
         sCatalogName.assign(EscapeUnderscore(sCatalogName));
      if (sSchemaName.length())
         sSchemaName.assign(EscapeUnderscore(sSchemaName));
      sTableName.assign(EscapeUnderscore(sTableName));
   }

   if (sCatalogName.length()) {
      cCatalogName = (SQLTCHAR*)sCatalogName.c_str();
      nCatalogLength = SQL_NTS;
   }

   if (sSchemaName.length()) {
      cSchemaName = (SQLTCHAR*)sSchemaName.c_str();
      nSchemaLength = SQL_NTS;
   }

   nRetcode = SQLTables(oRecordset->getHSTMT(),
         cCatalogName, nCatalogLength, cSchemaName, nSchemaLength,
         (SQLTCHAR*)sTableName.c_str(), SQL_NTS, (SQLTCHAR*)_T("TABLE,VIEWS"), SQL_NTS); //(SQLTCHAR*)_T(""), SQL_NTS

   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_STMT, oRecordset->getHSTMT(), 
            _T("ExistTable"),
            _T("Unable to get if exists table by SQLTables!"));
      return false;
   }

   nRetcode = oRecordset->MoveFirst();
   bReturn = RC_SUCCESS(nRetcode) ? true : false;
   delete oRecordset;

   return bReturn;
}
//---------------------------------------------------------------------------
bool ExistColumn(IDBConnection* oConnection, const String& sTableNameC, const String& sColumnNameC)
{
   bool bReturn = false;
   SQLRETURN nRetcode = SQL_SUCCESS;
   DBRecordsetSchema* oRecordset = CreateRecordsetColumns(oConnection);
   String sCatalogName = oConnection->getCatalogName();
   String sSchemaName = oConnection->getSchemaName();
   String sTableName = sTableNameC;
   String sColumnName = sColumnNameC;
   SQLTCHAR* cSchemaName = NULL;
   SQLSMALLINT nSchemaLength = 0;
   SQLTCHAR* cCatalogName = NULL;
   SQLSMALLINT nCatalogLength = 0;

   if (!oRecordset->getMetadataID()) {
      if (sSchemaName.length())
         sSchemaName.assign(EscapeUnderscore(sSchemaName));
      sTableName.assign(EscapeUnderscore(sTableName));
      sColumnName.assign(EscapeUnderscore(sColumnName));
   }

   if (sCatalogName.length()) {
      cCatalogName = (SQLTCHAR*)sCatalogName.c_str();
      nCatalogLength = SQL_NTS;
   }

   if (sSchemaName.length()) {
      cSchemaName = (SQLTCHAR*)sSchemaName.c_str();
      nSchemaLength = SQL_NTS;
   }

   nRetcode = SQLColumns(oRecordset->getHSTMT(),
         cCatalogName, nCatalogLength, cSchemaName, nSchemaLength,
         (SQLTCHAR*)sTableName.c_str(), SQL_NTS, (SQLTCHAR*)sColumnName.c_str(), SQL_NTS);

   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_STMT, oRecordset->getHSTMT(), 
            _T("ExistTable"),
            _T("Unable to get if exists table column by SQLColumns!"));
      return false;
   }

   nRetcode = oRecordset->MoveFirst();
   bReturn = RC_SUCCESS(nRetcode) ? true : false;
   delete oRecordset;

   return bReturn;
}
//---------------------------------------------------------------------------
SQLRETURN GetSchemaTables(IDBRecordsetSchema** oRecordsetShema, IDBConnection* oConnection, const String& sTableNameC)
{
   SQLRETURN nRetcode = SQL_SUCCESS;
   DBRecordsetSchema* oRecordset = CreateRecordsetTables(oConnection);
   String sCatalogName = oConnection->getCatalogName();
   String sSchemaName = oConnection->getSchemaName();
   String sTableName = sTableNameC;
   SQLTCHAR* cSchemaName = NULL;
   SQLSMALLINT nSchemaLength = 0;
   SQLTCHAR* cCatalogName = NULL;
   SQLSMALLINT nCatalogLength = 0;

   *oRecordsetShema = NULL;
   if (!oRecordset->getMetadataID()) {
      if (sCatalogName.length())
         sCatalogName.assign(EscapeUnderscore(sCatalogName));
      if (sSchemaName.length())
         sSchemaName.assign(EscapeUnderscore(sSchemaName));
      sTableName.assign(EscapeUnderscore(sTableName));
   }

   if (sCatalogName.length()) {
      cCatalogName = (SQLTCHAR*)sCatalogName.c_str();
      nCatalogLength = SQL_NTS;
   }

   if (sSchemaName.length()) {
      cSchemaName = (SQLTCHAR*)sSchemaName.c_str();
      nSchemaLength = SQL_NTS;
   }

   nRetcode = SQLTables(oRecordset->getHSTMT(),
         cCatalogName, nCatalogLength, cSchemaName, nSchemaLength,
         (SQLTCHAR*)sTableName.c_str(), SQL_NTS, NULL, 0); //(SQLTCHAR*)_T(""), SQL_NTS

   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_STMT, oRecordset->getHSTMT(),
            _T("GetSchemaTables"),
            _T("Unable to get tables by SQLTables!"));
      delete oRecordset;
      return nRetcode;
   }

   *oRecordsetShema = oRecordset;
   return nRetcode;
}
//---------------------------------------------------------------------------
SQLRETURN GetSchemaColumns(IDBRecordsetSchema** oRecordsetShema, IDBConnection* oConnection, const String& sTableNameC, const String& sColumnNameC)
{
   SQLRETURN nRetcode = SQL_SUCCESS;
   DBRecordsetSchema* oRecordset = CreateRecordsetColumns(oConnection);
   String sCatalogName = oConnection->getCatalogName();
   String sSchemaName = oConnection->getSchemaName();
   String sTableName = sTableNameC;
   String sColumnName = sColumnNameC;
   SQLTCHAR* cSchemaName = NULL;
   SQLSMALLINT nSchemaLength = 0;
   SQLTCHAR* cCatalogName = NULL;
   SQLSMALLINT nCatalogLength = 0;

   *oRecordsetShema = NULL;
   if (!oRecordset->getMetadataID()) {
      if (sSchemaName.length())
         sSchemaName.assign(EscapeUnderscore(sSchemaName));
      sTableName.assign(EscapeUnderscore(sTableName));
      sColumnName.assign(EscapeUnderscore(sColumnName));
   }

   if (sCatalogName.length()) {
      cCatalogName = (SQLTCHAR*)sCatalogName.c_str();
      nCatalogLength = SQL_NTS;
   }

   if (sSchemaName.length()) {
      cSchemaName = (SQLTCHAR*)sSchemaName.c_str();
      nSchemaLength = SQL_NTS;
   }

   nRetcode = SQLColumns(oRecordset->getHSTMT(),
         cCatalogName, nCatalogLength, cSchemaName, nSchemaLength,
         (SQLTCHAR*)sTableName.c_str(), SQL_NTS, (SQLTCHAR*)sColumnName.c_str(), SQL_NTS);

   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_STMT, oRecordset->getHSTMT(),
            _T("GetSchemaColumns"),
            _T("Unable to get table columns by SQLColumns!"));
      delete oRecordset;
      return nRetcode;
   }

   *oRecordsetShema = oRecordset;
   return nRetcode;
}
//---------------------------------------------------------------------------
SQLRETURN GetSchemaPrimaryKeys(IDBRecordsetSchema** oRecordsetShema, IDBConnection* oConnection, const String& sTableNameC)
{
   SQLRETURN nRetcode = SQL_SUCCESS;
   DBRecordsetSchema* oRecordset = CreateRecordsetPrimaryKeys(oConnection);
   String sCatalogName = oConnection->getCatalogName();
   String sSchemaName = oConnection->getSchemaName();
   String sTableName = sTableNameC;
   SQLTCHAR* cSchemaName = NULL;
   SQLSMALLINT nSchemaLength = 0;
   SQLTCHAR* cCatalogName = NULL;
   SQLSMALLINT nCatalogLength = 0;

   *oRecordsetShema = NULL;
   if (sCatalogName.length()) {
      cCatalogName = (SQLTCHAR*)sCatalogName.c_str();
      nCatalogLength = SQL_NTS;
   }

   if (sSchemaName.length()) {
      cSchemaName = (SQLTCHAR*)sSchemaName.c_str();
      nSchemaLength = SQL_NTS;
   }

   nRetcode = SQLPrimaryKeys(oRecordset->getHSTMT(),
         cCatalogName, nCatalogLength, cSchemaName, nSchemaLength,
         (SQLTCHAR*)sTableName.c_str(), SQL_NTS);

   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_STMT, oRecordset->getHSTMT(),
            _T("GetSchemaPrimaryKeys"),
            _T("Unable to get table primary keys by SQLPrimaryKeys!"));
      delete oRecordset;
      return nRetcode;
   }

   *oRecordsetShema = oRecordset;
   return nRetcode;
}
//---------------------------------------------------------------------------
SQLRETURN GetSchemaForeignKeys(IDBRecordsetSchema** oRecordsetSchema, IDBConnection* oConnection, const String& sTableNameC)
{
   SQLRETURN nRetcode = SQL_SUCCESS;
   DBRecordsetSchema* oRecordset = CreateRecordsetForeignKeys(oConnection);
   String sCatalogName = oConnection->getCatalogName();
   String sSchemaName = oConnection->getSchemaName();
   String sTableName = sTableNameC;
   SQLTCHAR* cSchemaName = NULL;
   SQLSMALLINT nSchemaLength = 0;
   SQLTCHAR* cCatalogName = NULL;
   SQLSMALLINT nCatalogLength = 0;

   *oRecordsetSchema = NULL;
   if (sCatalogName.length()) {
      cCatalogName = (SQLTCHAR*)sCatalogName.c_str();
      nCatalogLength = SQL_NTS;
   } else if (oRecordset->getMetadataID()) {
      cCatalogName = (SQLTCHAR*)String(_T("")).c_str();
      nCatalogLength = SQL_NTS;
   }

   if (sSchemaName.length()) {
      cSchemaName = (SQLTCHAR*)sSchemaName.c_str();
      nSchemaLength = SQL_NTS;
   } else if (oRecordset->getMetadataID()) {
      cSchemaName = (SQLTCHAR*)String(_T("")).c_str();
      nSchemaLength = SQL_NTS;
   }

   nRetcode = SQLForeignKeys(oRecordset->getHSTMT(),
         NULL, 0, /* Primary catalog */  NULL, 0, /* Primary schema */ NULL, 0, /* Primary table */
         cCatalogName, nCatalogLength, cSchemaName, nSchemaLength,
         (SQLTCHAR*)sTableName.c_str(), SQL_NTS);

   if (!RC_SUCCESS(nRetcode)) {
      ExceptionByHandle(SQL_HANDLE_STMT, oRecordset->getHSTMT(),
         _T("GetSchemaForeignKeys"),
         _T("Unable to get table foreign keys by SQLForeignKeys!"));
      delete oRecordset;
      return nRetcode;
   }

   *oRecordsetSchema = oRecordset;
   return nRetcode;
}
//---------------------------------------------------------------------------
SQLRETURN AddMetaTable(IDBMetaTable* oMetaTable)
{
   DBMetaTables* oMetaTables = NULL;

   boost::mutex::scoped_lock oLock(oMutex);
   DBConnTables::iterator iConnTables = oConnTables.find(oMetaTable->getConnection());
   if (iConnTables != oConnTables.end()) {
      oMetaTables = iConnTables->second;

      DBMetaTables::iterator iMetaTable = oMetaTables->find(oMetaTable->getTableName());
      if (iMetaTable != oMetaTables->end()) {
         TCHAR cBuffer[NORMAL_MSG] = {0};
         SgFormatMsg(cBuffer,NORMAL_MSG,_T("Tabela '%s' já adicionada ao dicionário de dados."),
                     oMetaTable->getTableName().c_str());
         SetExceptionInfo(_T("AddMetaTable"),cBuffer,
               _T("42S01"),_T("Base table or view already exists"),0);
         return SQL_ERROR;
      } else {
         (*oMetaTables)[oMetaTable->getTableName()] = dynamic_cast<DBMetaTable*>(oMetaTable);
      }
   } else {
      oMetaTables = new DBMetaTables();
      (*oMetaTables)[oMetaTable->getTableName()] = dynamic_cast<DBMetaTable*>(oMetaTable);
      oConnTables[oMetaTable->getConnection()] = oMetaTables;
   }

   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN AddMetaTableByName(IDBMetaTable** oIMetaTable, IDBConnection* oConnection, const String& sTableName)
{
   DBMetaTables* oMetaTables = NULL;
   DBMetaTable* oMetaTable = NULL;

   *oIMetaTable = NULL;
   boost::mutex::scoped_lock oLock(oMutex);
   DBConnTables::iterator iConnTables = oConnTables.find(oConnection);
   if (iConnTables != oConnTables.end()) {
      oMetaTables = iConnTables->second;

      DBMetaTables::iterator iMetaTable = oMetaTables->find(sTableName);
      if (iMetaTable != oMetaTables->end()) {
         oMetaTable = iMetaTable->second;
      } else {
         oMetaTable = new DBMetaTable(oConnection,sTableName);
         (*oMetaTables)[sTableName] = oMetaTable;
      }

   } else {
      oMetaTables = new DBMetaTables();
      oMetaTable = new DBMetaTable(oConnection,sTableName);
      (*oMetaTables)[sTableName] = oMetaTable;
      oConnTables[oMetaTable->getConnection()] = oMetaTables;
   }

   *oIMetaTable = oMetaTable;
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN GetMetaTableByName(IDBMetaTable** oIMetaTable, IDBConnection* oConnection, const String& sTableName)
{
   DBMetaTables* oMetaTables = NULL;

   *oIMetaTable = NULL;
   boost::mutex::scoped_lock oLock(oMutex);
   DBConnTables::iterator iConnTables = oConnTables.find(oConnection);
   if (iConnTables != oConnTables.end()) {
      oMetaTables = iConnTables->second;

      DBMetaTables::iterator iMetaTable = oMetaTables->find(sTableName);
      if (iMetaTable != oMetaTables->end()) {
         *oIMetaTable = iMetaTable->second;
         return SQL_SUCCESS;
      }
   }
   return SQL_ERROR;
}
//---------------------------------------------------------------------------
SQLRETURN AddMetaTableFromSchema(IDBMetaTable** oIMetaTable, IDBConnection* oConnection, const String& sTableName)
{
   SQLRETURN nRetcode = SQL_SUCCESS;
   boost::mutex::scoped_lock oLock(oMutex);

   *oIMetaTable = NULL;
   //Verificando se a tabela existe no schema do banco
   if (!ExistTable(oConnection,sTableName)) {
      TCHAR cBuffer[NORMAL_MSG] = {0};
      SgFormatMsg(cBuffer,NORMAL_MSG,_T("Tabela '%s' não encontrada no banco de dados '%s'!"),
                  sTableName.c_str(),oConnection->getDatabaseName().c_str());
      SetExceptionInfo(_T("AddMetaTableFromSchema"),cBuffer,
            _T("42S02"),_T("Base table or view not found"),0);
      return SQL_ERROR;
   }

   DBRecordsetSchema* oRecordset = NULL;
   nRetcode = GetSchemaColumns((IDBRecordsetSchema**)&oRecordset,oConnection,sTableName,_T("%"));
   if (!RC_SUCCESS(nRetcode))
      return nRetcode;

   DBFields* oFields = dynamic_cast<DBFields*>(oRecordset->getFields());
   DBMetaTable* oMetaTable = new DBMetaTable(oConnection, sTableName, false);
   DBMetaFields* oMetaFields = new DBMetaFields(oMetaTable);
   DBMetaField* oMetaField = NULL;
   DataType eDataType = TYPE_UNKNOWN;
   int nFound = -1;
   bool bUnsigned = false;
   String sTypeName = _T("");

   //Carregando os campos da tabela do schema do banco
   oRecordset->MoveFirst();
   while (!oRecordset->getEOF()) {
      //sTypeName.assign(oFields->FieldByName("TYPE_NAME")->AsString());
      sTypeName.assign(oFields->FieldByIndex(5)->AsString());
      //Transformando a string em minúsculo
      transform(sTypeName.begin(), sTypeName.end(), sTypeName.begin(), ::_totlower);
      nFound = sTypeName.find(_T("unsign"));
      bUnsigned = (nFound >= 0);
      //eDataType = GetTypeBySQL(oFields->FieldByName("DATA_TYPE")->AsInteger());
      eDataType = GetTypeBySQL(oFields->FieldByIndex(4)->AsInteger(),bUnsigned);
      //oMetaTable->Add(new BDMetaField(oMetaTable,oFields->FieldByName("ORDINAL_POSITION")->AsInteger(),oFields->FieldByName("COLUMN_NAME")->AsString(),
      //      eDataType,oFields->FieldByName("COLUMN_SIZE")->AsInteger(),oFields->FieldByName("DECIMAL_DIGITS")->AsInteger(),oFields->FieldByName("NULLABLE")->AsBoolean()));
      oMetaFields->Add(new DBMetaField(oMetaFields,oFields->FieldByIndex(16)->AsInteger(),oFields->FieldByIndex(3)->AsString(),oFields->FieldByIndex(3)->AsString(),
            MFE_MASK_EDIT,eDataType,oFields->FieldByIndex(6)->AsInteger(),oFields->FieldByIndex(8)->AsInteger(),oFields->FieldByIndex(10)->AsBoolean()));
      oRecordset->MoveNext();
   }
   oMetaTable->setMetaFields(oMetaFields);
   delete oRecordset;

   //Carregando as chaves primárias da tabela do schema do banco
   nRetcode = GetSchemaPrimaryKeys((IDBRecordsetSchema**)&oRecordset,oConnection,sTableName);
   if (oRecordset != NULL) {
      oFields = dynamic_cast<DBFields*>(oRecordset->getFields());
      oRecordset->MoveFirst();
      while (!oRecordset->getEOF()) {
         oMetaField = dynamic_cast<DBMetaField*>(oMetaTable->FieldByName(oFields->FieldByName(_T("COLUMN_NAME"))->AsString()));
         if (oMetaField != NULL)
            oMetaField->setIsPkKey(true);
         oRecordset->MoveNext();
      }
      delete oRecordset;
   }

   //Carregando as chaves estrangeiras da tabela do schema do banco
   nRetcode = GetSchemaForeignKeys((IDBRecordsetSchema**)&oRecordset,oConnection,sTableName);
   if (oRecordset != NULL) {
      oFields = dynamic_cast<DBFields*>(oRecordset->getFields());
      oRecordset->MoveFirst();
      while (!oRecordset->getEOF()) {
         oMetaField = dynamic_cast<DBMetaField*>(oMetaTable->FieldByName(oFields->FieldByName(_T("FKCOLUMN_NAME"))->AsString()));
         if (oMetaField != NULL) {
            oMetaField->setIsFkKey(true);
            oMetaField->setFkTableName(oFields->FieldByName(_T("PKTABLE_NAME"))->AsString());
            oMetaField->setFkColumnName(oFields->FieldByName(_T("PKCOLUMN_NAME"))->AsString());
         }
         oRecordset->MoveNext();
      }
      delete oRecordset;
   }

   if (!RC_SUCCESS(AddMetaTable(oMetaTable)))
      return SQL_ERROR;

   *oIMetaTable = oMetaTable;
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN GetSQLTables(IDBRecordsetSchema** oRecordsetSchema, IDBConnection* oConnection, const String& sTableName, const String& sSQL)
{
   DBRecordsetSchema* oRecordset = CreateRecordsetColumns(oConnection);
   String sCatalogName = oConnection->getCatalogName();
   String sSchemaName = oConnection->getSchemaName();
   String sSQLCmd = _T("");

   *oRecordsetSchema = NULL;
   if (sSQL.length()) {
      sSQLCmd = sSQL;
   } else {
      sSQLCmd += _T("SELECT table_catalog, table_schema, table_name, table_type, '' AS REMARKS");
      sSQLCmd += _T(" FROM information_schema.tables");
      sSQLCmd += _T(" WHERE table_name = '")+sTableName+_T("'");
      if (sCatalogName.length())
         sSQLCmd += _T(" AND table_catalog = '")+sCatalogName+_T("'");
      if (sSchemaName.length())
         sSQLCmd += _T(" AND table_schema = '")+sSchemaName+_T("'");
   }
   if (!RC_SUCCESS(oRecordset->Open(sSQLCmd))) {
      delete oRecordset;
      return SQL_ERROR;
   }

   *oRecordsetSchema = oRecordset;
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN GetSQLColumns(IDBRecordsetSchema** oRecordsetSchema, IDBConnection* oConnection, const String& sTableName, const String& sColumnName, const String& sSQL)
{
   DBRecordsetSchema* oRecordset = CreateRecordsetTables(oConnection);
   String sCatalogName = oConnection->getCatalogName();
   String sSchemaName = oConnection->getSchemaName();
   String sSQLCmd = _T("");

   *oRecordsetSchema = NULL;
   if (sSQL.length()) {
      sSQLCmd = sSQL;
   } else {
/*
SELECT table_catalog, table_schema, table_name, column_name, data_type, data_type AS TYPE_NAME,
character_maximum_length,character_maximum_length,numeric_scale,numeric_precision_radix,
CASE WHEN is_nullable = 'NO' THEN 0 ELSE 1 END NULLABLE, '' AS REMARKS,
column_default,data_type,datetime_precision,character_octet_length,ordinal_position,is_nullable

FROM information_schema.columns WHERE table_name = 'dcf990'
*/

//   oFields->Add(new DBField(oFields, 1 /*nFieldNum*/,_T("TABLE_CAT"),TYPE_TVARCHAR,oConnection->getMaxCatalogName()));
//   oFields->Add(new DBField(oFields, 2 /*nFieldNum*/,_T("TABLE_SCHEM"),TYPE_TVARCHAR,oConnection->getMaxSchemaName()));
//   oFields->Add(new DBField(oFields, 3 /*nFieldNum*/,_T("TABLE_NAME"),TYPE_TVARCHAR,oConnection->getMaxTableName(),0,true));
//   oFields->Add(new DBField(oFields, 4 /*nFieldNum*/,_T("COLUMN_NAME"),TYPE_TVARCHAR,oConnection->getMaxColumnName(),0,true));
//   oFields->Add(new DBField(oFields, 5 /*nFieldNum*/,_T("DATA_TYPE"),TYPE_SSMALLINT,5,0,true));
//   oFields->Add(new DBField(oFields, 6 /*nFieldNum*/,_T("TYPE_NAME"),TYPE_TVARCHAR,128,0,true));
//   oFields->Add(new DBField(oFields, 7 /*nFieldNum*/,_T("COLUMN_SIZE"),TYPE_SINTEGER,10));
//   oFields->Add(new DBField(oFields, 8 /*nFieldNum*/,_T("BUFFER_LENGTH"),TYPE_SINTEGER,10));
//   oFields->Add(new DBField(oFields, 9 /*nFieldNum*/,_T("DECIMAL_DIGITS"),TYPE_SSMALLINT,5));
//   oFields->Add(new DBField(oFields,10 /*nFieldNum*/,_T("NUM_PREC_RADIX"),TYPE_SSMALLINT,5));
//   oFields->Add(new DBField(oFields,11 /*nFieldNum*/,_T("NULLABLE"),TYPE_SSMALLINT,5,0,true));
//   oFields->Add(new DBField(oFields,12 /*nFieldNum*/,_T("REMARKS"),TYPE_STRING,255));
//   oFields->Add(new DBField(oFields,13 /*nFieldNum*/,_T("COLUMN_DEF"),TYPE_STRING,255));
//   oFields->Add(new DBField(oFields,14 /*nFieldNum*/,_T("SQL_DATA_TYPE"),TYPE_SSMALLINT,5,0,true));
//   oFields->Add(new DBField(oFields,15 /*nFieldNum*/,_T("SQL_DATETIME_SUB"),TYPE_SSMALLINT,5));
//   oFields->Add(new DBField(oFields,16 /*nFieldNum*/,_T("CHAR_OCTET_LENGTH"),TYPE_SINTEGER,10));
//   oFields->Add(new DBField(oFields,17 /*nFieldNum*/,_T("ORDINAL_POSITION"),TYPE_SINTEGER,10,0,true));
//   oFields->Add(new DBField(oFields,18 /*nFieldNum*/,_T("IS_NULLABLE"),TYPE_TVARCHAR,3));

      sSQLCmd += _T("SELECT table_catalog, table_schema, table_name, table_type, '' REMARKS");
      sSQLCmd += _T(" FROM information_schema.tables");
      sSQLCmd += _T(" WHERE table_name = '")+sTableName+_T("'");
      if (sCatalogName.length())
         sSQLCmd += _T(" table_catalog = '")+sCatalogName+_T("'");
      if (sSchemaName.length())
         sSQLCmd += _T(" table_schema = '")+sSchemaName+_T("'");
   }
   if (!RC_SUCCESS(oRecordset->Open(sSQLCmd))) {
      delete oRecordset;
      return SQL_ERROR;
   }

   *oRecordsetSchema = oRecordset;
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN GetSQLPrimaryKeys(IDBRecordsetSchema** oRecordsetSchema, IDBConnection* oConnection, const String& sTableName, const String& sSQL)
{
   DBRecordsetSchema* oRecordset = CreateRecordsetColumns(oConnection);
   String sCatalogName = oConnection->getCatalogName();
   String sSchemaName = oConnection->getSchemaName();
   String sSQLCmd = _T("");

   *oRecordsetSchema = NULL;
   if (sSQL.length()) {
      sSQLCmd = sSQL;
   } else {
      sSQLCmd += _T("SELECT col_pk.table_catalog, col_pk.table_schema, col_pk.table_name,");
      sSQLCmd += _T("col_pk.column_name, col_pk.ordinal_position, col_pk.constraint_name");
      sSQLCmd += _T(" FROM information_schema.table_constraints tb_pk,");
      sSQLCmd += _T(" information_schema.key_column_usage col_pk");
      sSQLCmd += _T(" WHERE tb_pk.table_name = '")+sTableName+_T("'");
      if (sCatalogName.length())
         sSQLCmd += _T(" AND tb_pk.constraint_catalog = '")+sCatalogName+_T("'");
      if (sSchemaName.length())
         sSQLCmd += _T(" AND tb_pk.constraint_schema = '")+sSchemaName+_T("'");
      sSQLCmd += _T(" AND tb_pk.constraint_type = 'PRIMARY KEY'");
      sSQLCmd += _T(" AND tb_pk.constraint_catalog = col_pk.constraint_catalog");
      sSQLCmd += _T(" AND tb_pk.constraint_schema = col_pk.constraint_schema");
      sSQLCmd += _T(" AND tb_pk.table_name = col_pk.table_name");
      sSQLCmd += _T(" AND tb_pk.constraint_name = col_pk.constraint_name");
   }
   if (!RC_SUCCESS(oRecordset->Open(sSQLCmd))) {
      delete oRecordset;
      return SQL_ERROR;
   }

   *oRecordsetSchema = oRecordset;
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN GetSQLForeignKeys(IDBRecordsetSchema** oRecordsetSchema, IDBConnection* oConnection, const String& sTableName, const String& sSQL)
{
   DBRecordsetSchema* oRecordset = CreateRecordsetColumns(oConnection);
   String sCatalogName = oConnection->getCatalogName();
   String sSchemaName = oConnection->getSchemaName();
   String sSQLCmd = _T("");

   *oRecordsetSchema = NULL;
   if (sSQL.length()) {
      sSQLCmd = sSQL;
   } else {
      sSQLCmd += _T("SELECT col_pk.table_catalog, col_pk.table_schema, col_pk.table_name,");
      sSQLCmd += _T(" col_pk.column_name, col_pk.ordinal_position, col_pk.constraint_name");
      sSQLCmd += _T(" FROM information_schema.table_constraints tb_pk,");
      sSQLCmd += _T(" information_schema.key_column_usage col_pk");
      sSQLCmd += _T(" WHERE tb_pk.table_name = '")+sTableName+_T("'");
      if (sCatalogName.length())
         sSQLCmd += _T(" AND tb_pk.constraint_catalog = '")+sCatalogName+_T("'");
      if (sSchemaName.length())
         sSQLCmd += _T(" AND tb_pk.constraint_schema = '")+sSchemaName+_T("'");
      sSQLCmd += _T(" AND tb_pk.constraint_type = 'FOREIGN KEY'");
      sSQLCmd += _T(" AND tb_pk.constraint_catalog = col_pk.constraint_catalog");
      sSQLCmd += _T(" AND tb_pk.constraint_schema = col_pk.constraint_schema");
      sSQLCmd += _T(" AND tb_pk.table_name = col_pk.table_name");
      sSQLCmd += _T(" AND tb_pk.constraint_name = col_pk.constraint_name");
   }
   if (!RC_SUCCESS(oRecordset->Open(sSQLCmd))) {
      delete oRecordset;
      return SQL_ERROR;
   }

   *oRecordsetSchema = oRecordset;
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
bool ExistTableSQL(IDBConnection* oConnection, const String& sTableName, const String& sSQL)
{
   bool bReturn = false;
   SQLRETURN nRetcode = SQL_SUCCESS;
   DBRecordsetSchema* oRecordset = CreateRecordsetTables(oConnection);
   String sCatalogName = oConnection->getCatalogName();
   String sSchemaName = oConnection->getSchemaName();
   String sSQLCmd = _T("");

   if (sSQL.length()) {
      sSQLCmd = sSQL;
   } else {
      sSQLCmd += _T("SELECT table_name");
      sSQLCmd += _T(" FROM information_schema.tables");
      sSQLCmd += _T(" WHERE table_name = '")+sTableName+_T("'");
      if (sCatalogName.length())
         sSQLCmd += _T(" AND table_catalog = '")+sCatalogName+_T("'");
      if (sSchemaName.length())
         sSQLCmd += _T(" AND table_schema = '")+sSchemaName+_T("'");
   }
   oRecordset->Open(sSQLCmd);
   nRetcode = oRecordset->MoveFirst();
   bReturn = RC_SUCCESS(nRetcode) ? true : false;
   delete oRecordset;

   return bReturn;
}
//---------------------------------------------------------------------------
bool ExistColumnSQL(IDBConnection* oConnection, const String& sTableName, const String& sColumnName, const String& sSQL)
{
   bool bReturn = false;
   SQLRETURN nRetcode = SQL_SUCCESS;
   DBRecordsetSchema* oRecordset = CreateRecordsetTables(oConnection);
   String sCatalogName = oConnection->getCatalogName();
   String sSchemaName = oConnection->getSchemaName();
   String sSQLCmd = _T("");

   if (sSQL.length()) {
      sSQLCmd = sSQL;
   } else {
      sSQLCmd += _T("SELECT column_name");
      sSQLCmd += _T(" FROM information_schema.columns");
      sSQLCmd += _T(" WHERE table_name = '")+sTableName+_T("'");
      sSQLCmd += _T(" AND column_name = '")+sColumnName+_T("'");
      if (sCatalogName.length())
         sSQLCmd += _T(" AND table_catalog = '")+sCatalogName+_T("'");
      if (sSchemaName.length())
         sSQLCmd += _T(" AND table_schema = '")+sSchemaName+_T("'");
   }
   oRecordset->Open(sSQLCmd);
   nRetcode = oRecordset->MoveFirst();
   bReturn = RC_SUCCESS(nRetcode) ? true : false;
   delete oRecordset;

   return bReturn;
}
//---------------------------------------------------------------------------
SQLRETURN AddMetaTableFromSQL(IDBMetaTable** oIMetaTable, IDBConnection* oConnection, const String& sTableName,
      const String& sSQLTable, const String& sSQLColumns,
      const String& sSQLPk, const String& sSQLFk)
{
   SQLRETURN nRetcode = SQL_SUCCESS;
   boost::mutex::scoped_lock oLock(oMutex);

   *oIMetaTable = NULL;
   //Verificando se a tabela existe no schema do banco
   if (!ExistTableSQL(oConnection,sTableName,sSQLTable)) {
      TCHAR cBuffer[NORMAL_MSG] = {0};
      SgFormatMsg(cBuffer,NORMAL_MSG,_T("Tabela '%s' não encontrada no banco de dados '%s'!"),
                  sTableName.c_str(),oConnection->getDatabaseName().c_str());
      SetExceptionInfo(_T("AddMetaTableFromSQL"),cBuffer,
            _T("42S02"),_T("Base table or view not found"),0);
      return SQL_ERROR;
   }

   DBRecordsetSchema* oRecordset = NULL;
   nRetcode = GetSQLColumns((IDBRecordsetSchema**)&oRecordset,oConnection,sTableName,_T(""),sSQLColumns);
   if (!RC_SUCCESS(nRetcode))
      return nRetcode;
   DBFields* oFields = dynamic_cast<DBFields*>(oRecordset->getFields());
   DBMetaTable* oMetaTable = new DBMetaTable(oConnection, sTableName, false);
   DBMetaFields* oMetaFields = new DBMetaFields(oMetaTable);
   DBMetaField* oMetaField = NULL;
   DataType eDataType = TYPE_UNKNOWN;
   int nFound = -1;
   bool bUnsigned = false;
   String sTypeName = _T("");

   //Carregando os campos da tabela do schema do banco
   oRecordset->MoveFirst();
   while (!oRecordset->getEOF()) {
      //sTypeName.assign(oFields->FieldByName("TYPE_NAME")->AsString());
      sTypeName.assign(oFields->FieldByIndex(5)->AsString());
      //Transformando a string em minúsculo
      transform(sTypeName.begin(), sTypeName.end(), sTypeName.begin(), ::_totlower);
      nFound = sTypeName.find(_T("unsign"));
      bUnsigned = (nFound >= 0);
      //eDataType = GetTypeBySQL(oFields->FieldByName("DATA_TYPE")->AsInteger());
      eDataType = GetTypeBySQL(oFields->FieldByIndex(4)->AsInteger(),bUnsigned);
      //oMetaTable->Add(new BDMetaField(oMetaTable,oFields->FieldByName("ORDINAL_POSITION")->AsInteger(),oFields->FieldByName("COLUMN_NAME")->AsString(),
      //      eDataType,oFields->FieldByName("COLUMN_SIZE")->AsInteger(),oFields->FieldByName("DECIMAL_DIGITS")->AsInteger(),oFields->FieldByName("NULLABLE")->AsBoolean()));
      oMetaFields->Add(new DBMetaField(oMetaFields,oFields->FieldByIndex(16)->AsInteger(),oFields->FieldByIndex(3)->AsString(),oFields->FieldByIndex(3)->AsString(),
            MFE_MASK_EDIT,eDataType,oFields->FieldByIndex(6)->AsInteger(),oFields->FieldByIndex(8)->AsInteger(),oFields->FieldByIndex(10)->AsBoolean()));
      oRecordset->MoveNext();
   }
   oMetaTable->setMetaFields(oMetaFields);
   delete oRecordset;

   //Carregando as chaves primárias da tabela do schema do banco
   nRetcode = GetSQLPrimaryKeys((IDBRecordsetSchema**)&oRecordset,oConnection,sTableName,sSQLPk);
   if (oRecordset != NULL) {
      oFields = dynamic_cast<DBFields*>(oRecordset->getFields());
      oRecordset->MoveFirst();
      while (!oRecordset->getEOF()) {
         oMetaField = dynamic_cast<DBMetaField*>(oMetaTable->FieldByName(oFields->FieldByName(_T("COLUMN_NAME"))->AsString()));
         if (oMetaField != NULL)
            oMetaField->setIsPkKey(true);
         oRecordset->MoveNext();
      }
      delete oRecordset;
   }

   //Carregando as chaves estrangeiras da tabela do schema do banco
   nRetcode = GetSQLForeignKeys((IDBRecordsetSchema**)&oRecordset,oConnection,sTableName,sSQLFk);
   if (oRecordset != NULL) {
      oFields = dynamic_cast<DBFields*>(oRecordset->getFields());
      oRecordset->MoveFirst();
      while (!oRecordset->getEOF()) {
         oMetaField = dynamic_cast<DBMetaField*>(oMetaTable->FieldByName(oFields->FieldByName(_T("FKCOLUMN_NAME"))->AsString()));
         if (oMetaField != NULL) {
            oMetaField->setIsFkKey(true);
            oMetaField->setFkTableName(oFields->FieldByName(_T("PKTABLE_NAME"))->AsString());
            oMetaField->setFkColumnName(oFields->FieldByName(_T("PKCOLUMN_NAME"))->AsString());
         }
         oRecordset->MoveNext();
      }
      delete oRecordset;
   }

   if (!RC_SUCCESS(AddMetaTable(oMetaTable)))
      return SQL_ERROR;

   *oIMetaTable = oMetaTable;
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
}; //namespace sfg
//---------------------------------------------------------------------------
