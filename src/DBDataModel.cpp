#include <vector>
#include "DBDataModel.hpp"
//---------------------------------------------------------------------------
namespace sfg {
//---------------------------------------------------------------------------
class DBFieldHack : public DBField {
public:
   void setReadOnly(bool Value) {bReadOnly = Value;};
   void setRetLength(SQLLEN Value) {nRetLength = Value;};
};
//---------------------------------------------------------------------------
SQLRETURN BindFields(DBFields *oFields) {
   std::vector<DBField*> oKeyFds; 
   DBField *oField;
   //Retira os campos que são PK
   for (int i = oFields->Count() - 1; i >= 0; i--) {
      oField = dynamic_cast<DBField*>(oFields->FieldByIndex(i));
      if (oField->getIsPkKey()) {
         oKeyFds.push_back(dynamic_cast<DBField*>(oFields->Remove(i)));
      }
   }
   //Associando os campos ao SELECT
   if (!RC_SUCCESS(oFields->BindFields()))
      return SQL_ERROR;
   //Adiciona novamente os campos que são PK, no final da lista
   std::vector<DBField*>::iterator iKeyFds = oKeyFds.begin();
   for (;iKeyFds != oKeyFds.end(); ++iKeyFds)
      oFields->Add(*iKeyFds);

   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
IDBDataModel* GetDBDataModel1(
      IDBMetaTable *oMetaTable, DataModelType eDmType, bool bCreateDBObj) {
   return new DBDataModel(oMetaTable,eDmType,bCreateDBObj);
}
//---------------------------------------------------------------------------
DBDataModel::DBDataModel(IDBMetaTable *oMetaTable, DataModelType eDmType, bool bCreateDBObj) :
      DBObject(), IDBDataModel()
{
   this->eDmState = DMS_NONE;
   this->eDmType = eDmType;
   this->oMetaTable = dynamic_cast<DBMetaTable*>(oMetaTable);
   this->oMetaFields = dynamic_cast<DBMetaFields*>(oMetaTable->getMetaFields());
   this->bRecordset = false;
   this->bSelectStmt = false;
   this->bInsertStmt = false;
   this->bUpdateStmt = false;
   this->bDeleteStmt = false;
   this->oFields = NULL;
   this->oRecordset = NULL;
   this->oSelectStmt = NULL;
   this->oInsertStmt = NULL;
   this->oUpdateStmt = NULL;
   this->oDeleteStmt = NULL;
   this->AfterRead = NULL;
   this->AfterPost =  NULL;
   this->AfterCancel = NULL;
   this->OnValid = NULL;
   if (eDmType == DMT_STMT) {
      if (bCreateDBObj)
         CreateStmtSQL();
   } else {
      if (bCreateDBObj)
         CreateRecordset();
   }
}
//---------------------------------------------------------------------------
DBDataModel::~DBDataModel()
{
   if (bRecordset) {
      delete oRecordset;
   } else {
      if (oFields != NULL)
         delete oFields;
   }
   if (bSelectStmt)
      delete oSelectStmt;
   if (bInsertStmt)
      delete oInsertStmt;
   if (bUpdateStmt)
      delete oUpdateStmt;
   if (bDeleteStmt)
      delete oDeleteStmt;
}
//---------------------------------------------------------------------------
BYTE DBDataModel::getDmState() {
   return eDmState;
}
//---------------------------------------------------------------------------
DataModelType DBDataModel::getDmType() {
   return eDmType;
}
//---------------------------------------------------------------------------
IDBMetaTable* DBDataModel::getMetaTable() {
   return oMetaTable;
}
//---------------------------------------------------------------------------
IDBMetaFields* DBDataModel::getMetaFields() {
   return oMetaFields;
}
//---------------------------------------------------------------------------
IDBFields* DBDataModel::getFields() {
   return oFields;
}
//---------------------------------------------------------------------------
IDBRecordset* DBDataModel::getRecordset() {
   return oRecordset;
}
//---------------------------------------------------------------------------
IDBStmt* DBDataModel::getStmt(DataModelStmt eStmtType)
{
   switch(eStmtType) {
      case DMM_SELECT:
         return oSelectStmt;
      case DMM_INSERT:
         return oInsertStmt;
      case DMM_UPDATE:
         return oUpdateStmt;
      case DMM_DELETE:
         return oDeleteStmt;
      default:
         return NULL;
   }
}
//---------------------------------------------------------------------------
void DBDataModel::setRecordset(IDBRecordset *oRecordset)
{
   if (bRecordset)
      delete this->oRecordset;
   this->oRecordset = dynamic_cast<DBRecordset*>(oRecordset);
   this->oFields = dynamic_cast<DBFields*>(oRecordset->getFields());
   bRecordset = false;
   for (int i=0; i < oFields->Count();i++) {
      oFields->FieldByIndex(i)->SetAfterSetValue(std::bind(&DBDataModel::SetChanged,this,std::placeholders::_1));
   }
}
//---------------------------------------------------------------------------
void DBDataModel::setStmt(IDBStmt* oStmt, DataModelStmt eStmtType)
{
   switch(eStmtType) {
      case DMM_SELECT:
         if (bSelectStmt)
            delete oSelectStmt;
         oSelectStmt = dynamic_cast<DBStmt*>(oStmt);
         bSelectStmt = false;
         break;
      case DMM_INSERT:
         if (bInsertStmt)
            delete oInsertStmt;
         oInsertStmt = dynamic_cast<DBStmt*>(oStmt);
         bInsertStmt = false;
         break;
      case DMM_UPDATE:
         if (bUpdateStmt)
            delete oUpdateStmt;
         oUpdateStmt = dynamic_cast<DBStmt*>(oStmt);
         bUpdateStmt = false;
         break;
      case DMM_DELETE:
         if (bDeleteStmt)
            delete oDeleteStmt;
         oDeleteStmt = dynamic_cast<DBStmt*>(oStmt);
         bDeleteStmt = false;
         break;
      default:; //Nothing
   }
}
//---------------------------------------------------------------------------
void DBDataModel::SetAfterRead(DataModelNotify pValue) {
   this->AfterRead = pValue;
}
//---------------------------------------------------------------------------
void DBDataModel::SetAfterPost(DataModelNotify pValue) {
   this->AfterPost = pValue;
}
//---------------------------------------------------------------------------
void DBDataModel::SetAfterCancel(DataModelNotify pValue) {
   this->AfterCancel = pValue;
}
//---------------------------------------------------------------------------
void DBDataModel::SetOnCalcFields(DataModelNotify pValue) {
   this->OnCalcFields = pValue;
}
//---------------------------------------------------------------------------
void DBDataModel::SetOnValid(DataModelValid pValue) {
   this->OnValid = pValue;
}
//---------------------------------------------------------------------------
IDBMetaField* DBDataModel::MetaFieldByName(const String &sName)
{
   if (oMetaFields != NULL)
      return oMetaFields->FieldByName(sName);
   else
      return NULL;
}
//---------------------------------------------------------------------------
IDBMetaField* DBDataModel::MetaFieldByIndex(int nIndex)
{
   if (oMetaFields != NULL)
      return oMetaFields->FieldByIndex(nIndex);
   else
      return NULL;
}
//---------------------------------------------------------------------------
IDBField* DBDataModel::FieldByName(const String &sName)
{
   if (oFields != NULL)
      return oFields->FieldByName(sName);
   else
      return NULL;
}
//---------------------------------------------------------------------------
IDBField* DBDataModel::FieldByIndex(int nIndex)
{
   if (oFields != NULL)
      return oFields->FieldByIndex(nIndex);
   else
      return NULL;
}
//---------------------------------------------------------------------------
bool DBDataModel::Valid()
{
   bool bValid = true;
   if (OnValid != NULL)
      OnValid(this,bValid);

   return bValid;
}
//---------------------------------------------------------------------------
void DBDataModel::Add()
{
   if (eDmType == DMT_RECORDSET) {
      if (oRecordset != NULL)
         oRecordset->Add();
   } else {
      DBFieldHack* oField = NULL;
      int nCount = oFields->Count();
      for(int i = 0; i < nCount; i++)
      {         
         oField = dynamic_cast<DBFieldHack*>(oFields->FieldByIndex(i));
         oField->setReadOnly(false);
         if(!oField->getNotNull())
            oField->setRetLength(SQL_NULL_DATA);
         if(oField->getAutoIncrement())
         {
            oField->setReadOnly(true);
            oField->setRetLength(SQL_COLUMN_IGNORE);
         }
      }
   }
   eDmState = eDmState|DMS_ADDED;
   eDmState = eDmState &~ (DMS_CHANGED|DMS_COMMITTED);
}
//---------------------------------------------------------------------------
bool DBDataModel::Delete(bool bValid)
{
   if (bValid)
      if (!Valid())
         return false;

   eDmState = eDmState|DMS_DELETED;
   eDmState = eDmState &~ DMS_COMMITTED;

   return true;
}
//---------------------------------------------------------------------------
bool DBDataModel::Read()
{
   if (!ReadFromDB())
      return false;

   eDmState = eDmState|DMS_READ;
   eDmState = eDmState &~ (DMS_CHANGED|DMS_COMMITTED);

   if (AfterRead != NULL)
      AfterRead(this);

   return true;
}
//---------------------------------------------------------------------------
bool DBDataModel::Post(bool bValid)
{
   //Se não foi feito nada com o registro, não escreve o mesmo
   if (eDmState == DMS_NONE)
      return true;

   //Se já foi escrito o registro, não reescreve o mesmo
   if (eDmState & DMS_COMMITTED)
      return true;

   if (bValid)
      if (!Valid())
         return false;

   if (eDmState & DMS_ADDED) {
      //Se foi adicionado o registro mas foi deletado, não faz nada
      if (eDmState & DMS_DELETED) {
         return true;
      } else {
         //Se foi adicionado o registro deve propagar a inclusão pro banco de dados
         return InsertToDB();
      }
   }
   else
   //Se foi deletado o registro deve propagar a deleção pro banco de dados
   if (eDmState & DMS_DELETED) {
      return DeleteToDB();
   }
   else
   //Se foi lido o registro e alterado deve propagar a alteração pro banco de dados
   if ((eDmState & DMS_READ) && (eDmState & DMS_CHANGED)) {
      return UpdateToDB();
   }
   eDmState = eDmState|DMS_COMMITTED;
   eDmState = eDmState &~ DMS_CHANGED;

   if (AfterPost != NULL)
      AfterPost(this);

   return true;
}
//---------------------------------------------------------------------------
bool DBDataModel::Cancel()
{
   //Se não foi feito nada com o registro, não faz nada
   if (eDmState == DMS_NONE)
      return true;
   //Retira a flag de deletado caso exista
   eDmState = eDmState &~ DMS_DELETED;
   //Retira a flag de comitado caso exista
   eDmState = eDmState &~ DMS_COMMITTED;
   //Se foi lido o registro e alterado deve refazer a leitura
   if ((eDmState & DMS_READ) && (eDmState & DMS_CHANGED)) {
      eDmState = eDmState &~ DMS_CHANGED;
      if (!ReadFromDB())
         return false;

      eDmState = eDmState|DMS_READ;
      return true;
   }

   if (AfterCancel != NULL)
      AfterCancel(this);

   return true;
}
//---------------------------------------------------------------------------
SQLRETURN DBDataModel::CreateStmtSQL()
{
   String sKeyCols = _T("");
   String sSelectCols = _T("");
   String sInsertCols = _T("");
   String sInsertPars = _T("");
   String sUpdateCols = _T("");
   String sInsertParams = _T("");
   String sSelectSQL = _T("SELECT ");
   String sInsertSQL = _T("INSERT INTO ");
   String sUpdateSQL = _T("UPDATE ");
   String sDeleteSQL = _T("DELETE FROM ");

   int nCount = oMetaFields->Count();
   DBMetaField *oMetaField = NULL;

   //Gerando o SQL de SELECT da tabela
   oSelectStmt = new DBStmt(oMetaTable->getConnection());
   bSelectStmt = true;
   DBParams* oSelectParams = new DBParams(oSelectStmt, false);
   oFields = new DBFields((DBRecordset*)oSelectStmt, false);
   DBField *oField = NULL;
   for (int i = 0; i < nCount; i++) {
      oMetaField = dynamic_cast<DBMetaField*>(oMetaFields->FieldByIndex(i));

      //Não precisa testar se é virtual, pois virtual nunca pode ser PK
      if (oMetaField->getIsPkKey()) { 
         if (sKeyCols.length() <= 0)
            sKeyCols = oMetaField->getFieldName() + _T(" = ?");
         else
            sKeyCols += _T(" AND ") + oMetaField->getFieldName() + _T(" = ?");
         oSelectParams->Add(new DBParam(oSelectParams, 0 /*ParamNum*/, oMetaField->getFieldName(),
               oMetaField->getDataType(), oMetaField->getDataSize(), oMetaField->getDecimalDigits()));
      } else {
         //Se é virtual não inclui o campo no SELECT
         if (!oMetaField->getIsVirtual()) {
            if (sSelectCols.length() <= 0)
               sSelectCols = oMetaField->getFieldName();
            else
               sSelectCols += _T(",") + oMetaField->getFieldName();
         }
      }
      //Criando a listagem de campos do SQL
      oField = CreateField(oFields,oMetaField);
      oFields->Add(oField);
   }

   if (sKeyCols.length() <= 0) {
      TCHAR cBuffer[SHORT_MSG] = {0};
      SgFormatMsg(cBuffer,SHORT_MSG,
                  _T("Tabela %s sem definição de chave primária para instruções de comando SQL."),
                  oMetaTable->getTableName().c_str());
      SetExceptionInfo(_T("DBDataModel::CreateStmtSQL"),cBuffer,_T("HY000"),_T("General error"),0);
      return SQL_ERROR;
   }

   sSelectSQL += sSelectCols + _T("\n") +\
                 _T("  FROM ") + oMetaTable->getTableName() + _T("\n") +\
                 _T(" WHERE ") + sKeyCols;

   oSelectStmt->setSQLCmd(sSelectSQL);
   if (!RC_SUCCESS(oSelectStmt->Prepare())) 
      return SQL_ERROR;
   //Associando os campos ao SELECT
   if (!RC_SUCCESS(BindFields(oFields)))
      return SQL_ERROR;

   //Gerando o SQL de INSERT da tabela
   oInsertStmt = new DBStmt(oMetaTable->getConnection());
   bInsertStmt = true;
   DBParams* oInsertParams = new DBParams(oInsertStmt, false);
   for (int i = 0; i < nCount; i++) {
      oMetaField = dynamic_cast<DBMetaField*>(oMetaFields->FieldByIndex(i));
      //Se é virtual não inclui o campo
      if (oMetaField->getIsVirtual())
         continue;
      //Se é auto-incremento não inclui o campo
      if (oMetaField->getAutoIncrement())
         continue;

      if (sInsertCols.length() <= 0) {
         sInsertCols = _T(" (") + oMetaField->getFieldName();
         sInsertParams = _T(" (?");
      } else {
         sInsertCols += _T(",") + oMetaField->getFieldName();
         sInsertParams += _T(",?");
      }
      oInsertParams->Add(new DBParam(oInsertParams, 0 /*ParamNum*/, oMetaField->getFieldName(),
            oMetaField->getDataType(), oMetaField->getDataSize(), oMetaField->getDecimalDigits()));
   }
   sInsertCols += _T(")");
   sInsertParams += _T(")");
   sInsertSQL += oMetaTable->getTableName() + _T("\n") +\
                 sInsertCols + _T("\n") +\
                 _T(" VALUES ") + sInsertParams;
   oInsertStmt->setSQLCmd(sInsertSQL);
   if (!RC_SUCCESS(oInsertStmt->Prepare())) 
      return SQL_ERROR;

   //Gerando o SQL de UPDATE da tabela
   oUpdateStmt = new DBStmt(oMetaTable->getConnection());
   bUpdateStmt = true;
   DBParams* oUpdateParams = new DBParams(oUpdateStmt, false);
   for (int i = 0; i < nCount; i++) {
      oMetaField = dynamic_cast<DBMetaField*>(oMetaFields->FieldByIndex(i));
      //Se é virtual não inclui o campo
      if (oMetaField->getIsVirtual())
         continue;
      //Se o campo é chave primária não atualiza o mesmo
      if (oMetaField->getIsPkKey())
         continue;

      if (sUpdateCols.length() <= 0)
         sUpdateCols = oMetaField->getFieldName() + _T(" = ?");
      else
         sUpdateCols += _T(",") + oMetaField->getFieldName() + _T(" = ?");
      oUpdateParams->Add(new DBParam(oUpdateParams, 0 /*ParamNum*/, oMetaField->getFieldName(),
            oMetaField->getDataType(), oMetaField->getDataSize(), oMetaField->getDecimalDigits()));
   }
   DBParam *oUpdateParam = NULL;
   for(int i = 0; i < oSelectParams->Count(); i++) {
      oUpdateParam = dynamic_cast<DBParam*>(oSelectParams->ParamByIndex(i));
      oUpdateParams->Add(new DBParam(oUpdateParams, 0 /*ParamNum*/, oUpdateParam->getParamName(),
               oUpdateParam->getDataType(), oUpdateParam->getDataSize(), oUpdateParam->getDecimalDigits()));
   }
   sUpdateSQL += oMetaTable->getTableName() + _T("\n") +\
                 _T("   SET ") + sUpdateCols + _T("\n") +\
                 _T(" WHERE ") + sKeyCols;
   oUpdateStmt->setSQLCmd(sUpdateSQL);
   if (!RC_SUCCESS(oUpdateStmt->Prepare())) 
      return SQL_ERROR;

   //Gerando o SQL de DELETE da tabela
   oDeleteStmt = new DBStmt(oMetaTable->getConnection());
   bDeleteStmt = true;
   DBParams* oDeleteParams = new DBParams(oDeleteStmt, false);
   sDeleteSQL += oMetaTable->getTableName() + _T("\n") +\
                 _T(" WHERE ") + sKeyCols;
   DBParam *oDeleteParam = NULL;
   for(int i = 0; i < oSelectParams->Count(); i++) {
      oDeleteParam = dynamic_cast<DBParam*>(oSelectParams->ParamByIndex(i));
      oDeleteParams->Add(new DBParam(oDeleteParams, 0 /*ParamNum*/, oDeleteParam->getParamName(),
               oDeleteParam->getDataType(), oDeleteParam->getDataSize(), oDeleteParam->getDecimalDigits()));
   }
   oDeleteStmt->setSQLCmd(sDeleteSQL);
   if (!RC_SUCCESS(oDeleteStmt->Prepare())) 
      return SQL_ERROR;

   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
SQLRETURN DBDataModel::CreateRecordset()
{
   String sKeyCols = _T("");
   String sSelectCols = _T("");
   String sSelectSQL = _T("SELECT ");

   int nCount = oMetaFields->Count();
   DBMetaField *oMetaField = NULL;

   //Gerando o SQL de SELECT da tabela
   oRecordset = new DBRecordset(oMetaTable->getConnection());
   bRecordset = true;
   oFields = new DBFields(oRecordset, false);
   DBField *oField = NULL;
   for (int i = 0; i < nCount; i++) {
      oMetaField = dynamic_cast<DBMetaField*>(oMetaFields->FieldByIndex(i));
      //Não precisa testar se é virtual, pois virtual nunca pode ser PK
      if (oMetaField->getIsPkKey()) {
         if (sKeyCols.length() <= 0)
            sKeyCols = oMetaField->getFieldName();
         else
            sKeyCols += _T(",") + oMetaField->getFieldName();
      }
      //Se é virtual não inclui o campo no SELECT
      if (!oMetaField->getIsVirtual()) {
         if (sSelectCols.length() <= 0)
            sSelectCols = oMetaField->getFieldName();
         else
            sSelectCols += _T(",") + oMetaField->getFieldName();
      }
      //Criando a listagem de campos do SQL
      oField = CreateField(oFields,oMetaField);
      oFields->Add(oField);
   }

   /*if (sKeyCols.length() <= 0) {
      SgOStringStream oErrMsg;
      oErrMsg << _T("Tabela ") << oMetaTable->getTableName() << _T(" sem definição de chave primária para instruções de comando SQL.");
      SetExceptionInfo(_T("DBDataModel::setTableName"),oErrMsg.str());
   }*/

   sSelectSQL += sSelectCols + _T("\n") +\
                 _T("  FROM ") + oMetaTable->getTableName() + _T("\n") +\
                 _T(" ORDER BY ") + sKeyCols;
   
   if (!RC_SUCCESS(oRecordset->Open(sSelectSQL, SQL_CURSOR_DYNAMIC)))
      return SQL_ERROR;

   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
DBField* DBDataModel::CreateField(DBFields *oFields, DBMetaField *oMetaField)
{
   DBField* oField = new DBField(oFields, 0 /*FieldNum*/, oMetaField->getFieldName(),
               oMetaField->getDataType(), oMetaField->getDataSize(), oMetaField->getDecimalDigits(),
               oMetaField->getNotNull(),oMetaField->getIsPkKey(),oMetaField->getIsVirtual());
   oField->setAutoIncrement(oMetaField->getAutoIncrement());
   oField->SetAfterSetValue(std::bind(&DBDataModel::SetChanged,this,std::placeholders::_1));
   oField->setTag((void*)oMetaField);
   oMetaField->setTag((void*)oField);
   return oField;
}
//---------------------------------------------------------------------------
void DBDataModel::SetChanged(IDBBind* oBind) {
   eDmState = eDmState|DMS_CHANGED;
   eDmState = eDmState &~ DMS_COMMITTED;
}
//---------------------------------------------------------------------------
bool DBDataModel::ReadFromDB()
{
   if (eDmType == DMT_STMT) {
      if (oSelectStmt != NULL) {
         if (oSelectStmt->getParams() != NULL) {
            if (!RC_SUCCESS(CopyFieldToParam(dynamic_cast<DBParams*>(oSelectStmt->getParams()))))
               return false;
            if (oSelectStmt->IsExecuted())
               SQLCloseCursor(oSelectStmt->getHSTMT());
            if (!RC_SUCCESS(oSelectStmt->Execute()))
               return false;
            SQLFetchScroll(oSelectStmt->getHSTMT(), SQL_FETCH_NEXT, 0);
         } else 
            SetExceptionInfo(_T("DBDataModel::ReadFromDB"),
                              _T("Can't read from DB because a Statement object don't have parameters."),
                              _T("07001"),_T("Wrong number of parameters"),0);
            return false;
      } else
         SetExceptionInfo(_T("DBDataModel::ReadFromDB"),
                           _T("Can't read from DB because don't have a Statement object."),
                           _T("HY000"),_T("General error"),0);
         return false;
   } else {
      if(oRecordset != NULL) {
         oRecordset->RefreshRow();
      } else
         SetExceptionInfo(_T("DBDataModel::ReadFromDB"),
                           _T("Can't read from DB because don't have a Recordset object."),
                           _T("HY000"),_T("General error"),0);
         return false;
   }
   
   bool bHasVirtual = false;
   for(int i = 0; i < oFields->Count() && !bHasVirtual; i++) {
      bHasVirtual = oFields->FieldByIndex(i)->getIsVirtual();
   }
   if (bHasVirtual && OnCalcFields != NULL)
      OnCalcFields(this);
      
   return true;
}
//---------------------------------------------------------------------------
SQLRETURN DBDataModel::CopyFieldToParam(DBParams *oParams)
{
   int nIndex = 0;
   int nCount = oParams->Count();
   DBParam *oParam = NULL;
   DBField *oField = NULL;

   if (oFields == NULL)
      return SQL_ERROR;

   for(;nIndex < nCount; nIndex++) {
      oParam = dynamic_cast<DBParam*>(oParams->ParamByIndex(nIndex));
      oField = dynamic_cast<DBField*>(oFields->FieldByName(oParam->getParamName()));
      if (oField != NULL)
         if (!RC_SUCCESS(oField->CopyValue(oParam)))
            return SQL_ERROR;
   }
   return SQL_SUCCESS;
}
//---------------------------------------------------------------------------
bool DBDataModel::InsertToDB()
{
   if (eDmType == DMT_STMT) {
      if (oInsertStmt != NULL) {
         if (oInsertStmt->getParams() != NULL)
            if (!RC_SUCCESS(CopyFieldToParam(dynamic_cast<DBParams*>(oInsertStmt->getParams()))))
               return false;
         if (!RC_SUCCESS(oInsertStmt->Execute()))
            return false;
      } else
         SetExceptionInfo(_T("DBDataModel::InsertToDB"),
                           _T("Can't insert to DB because don't have a Statement object."),
                           _T("HY000"),_T("General error"),0);
         return false;
   } else {
      if(oRecordset != NULL) {
         if (!RC_SUCCESS(oRecordset->Post()))
            return false;
      } else
         SetExceptionInfo(_T("DBDataModel::InsertToDB"),
                           _T("Can't insert to DB because don't have a Recordset object."),
                           _T("HY000"),_T("General error"),0);
         return false;
   }
   return true;
}
//---------------------------------------------------------------------------
bool DBDataModel::UpdateToDB()
{
   if (eDmType == DMT_STMT) {
      if (oUpdateStmt != NULL) {
         if (oUpdateStmt->getParams() != NULL)
            if (!RC_SUCCESS(CopyFieldToParam(dynamic_cast<DBParams*>(oUpdateStmt->getParams()))))
               return false;
         if (!RC_SUCCESS(oUpdateStmt->Execute()))
            return false;
      } else
         SetExceptionInfo(_T("DBDataModel::UpdateToDB"),
                           _T("Can't update to DB because don't have a Statement object."),
                           _T("HY000"),_T("General error"),0);
         return false;
   } else {
      if(oRecordset != NULL) {
         if (!RC_SUCCESS(oRecordset->Post()))
            return false;
      } else
         SetExceptionInfo(_T("DBDataModel::UpdateToDB"),
                           _T("Can't update to DB because don't have a Recordset object."),
                           _T("HY000"),_T("General error"),0);
         return false;
   }
   return true;
}
//---------------------------------------------------------------------------
bool DBDataModel::DeleteToDB()
{
   if (eDmType == DMT_STMT) {
      if (oDeleteStmt != NULL) {
         if (oDeleteStmt->getParams() != NULL)
            if (!RC_SUCCESS(CopyFieldToParam(dynamic_cast<DBParams*>(oDeleteStmt->getParams()))))
               return false;
         if (!RC_SUCCESS(oDeleteStmt->Execute()))
            return false;
      } else
         SetExceptionInfo(_T("DBDataModel::DeleteToDB"),
                           _T("Can't delete from DB because don't have a Statement object."),
                           _T("HY000"),_T("General error"),0);
         return false;
   } else {
      if(oRecordset != NULL) {
         if (!RC_SUCCESS(oRecordset->Delete()))
            return false;
      } else
         SetExceptionInfo(_T("DBDataModel::DeleteToDB"),
                           _T("Can't delete from DB because don't have a Recordeset object."),
                           _T("HY000"),_T("General error"),0);
         return false;
   }
   return true;
}
//---------------------------------------------------------------------------
}; //namespce sfg
