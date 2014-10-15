//---------------------------------------------------------------------------
#ifndef DBDataModelH
#define DBDataModelH
//---------------------------------------------------------------------------
#include "DBMetaData.hpp"

namespace sfg {

class DBDataModel;
class DBVirtualField;
class DBVirtualFields;

//---------------------------------------------------------------------------
class DBDataModel : public DBObject, public virtual IDBDataModel
{
private: //Variáveis
   bool bRecordset;
   bool bSelectStmt;
   bool bInsertStmt;
   bool bUpdateStmt;
   bool bDeleteStmt;
protected://Events
   DataModelNotify AfterRead;
   DataModelNotify AfterPost;
   DataModelNotify AfterCancel;
   DataModelNotify OnCalcFields;
   DataModelValid OnValid;
protected: //Variáveis
   BYTE eDmState;
   DataModelType eDmType;
   DBMetaTable* oMetaTable;
   DBMetaFields* oMetaFields;
   DBFields* oFields;
   DBRecordset* oRecordset;
   DBStmt* oSelectStmt;
   DBStmt* oInsertStmt;
   DBStmt* oUpdateStmt;
   DBStmt* oDeleteStmt;
private: //Métodos
   SQLRETURN CreateStmtSQL();
   SQLRETURN CreateRecordset();
   SQLRETURN CopyFieldToParam(DBParams *oParams);
   void SetChanged(IDBBind* oBind);
protected: //Métodos
   virtual bool ReadFromDB();
   virtual bool InsertToDB();
   virtual bool UpdateToDB();
   virtual bool DeleteToDB();
   virtual DBField* CreateField(DBFields *oFields, DBMetaField *oMetaField);
public: //Propriedades
   BYTE getDmState();
   DataModelType getDmType();
   IDBMetaTable* getMetaTable();
   IDBMetaFields* getMetaFields();
   IDBFields* getFields();
   IDBRecordset* getRecordset();
   IDBStmt* getStmt(DataModelStmt eStmtType);
   void setRecordset(IDBRecordset *oRecordset);
   void setStmt(IDBStmt* oStmt, DataModelStmt eStmtType);
public: //Métodos
   DBDataModel(IDBMetaTable *oMetaTable, DataModelType eDmType = DMT_STMT, bool bCreateDBObj = true);
   ~DBDataModel();
   virtual void Add();
   virtual bool Delete(bool bValid = true);
   virtual bool Read();
   virtual bool Post(bool bValid = true);
   virtual bool Cancel();
   virtual bool Valid();
   IDBMetaField* MetaFieldByName(const String &sName);
   IDBMetaField* MetaFieldByIndex(int nIndex);
   IDBField* FieldByName(const String &sName);
   IDBField* FieldByIndex(int nIndex);
public: //Eventos
   void SetAfterRead(DataModelNotify pValue);
   void SetAfterPost(DataModelNotify pValue);
   void SetAfterCancel(DataModelNotify pValue);
   void SetOnCalcFields(DataModelNotify pValue);
   void SetOnValid(DataModelValid pValue);
};
//---------------------------------------------------------------------------
}; //end namespace sfg
#endif





