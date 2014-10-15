//---------------------------------------------------------------------------
#ifndef DBMetaDataH
#define DBMetaDataH
//---------------------------------------------------------------------------
#include "DBDatabase.hpp"

#ifdef _MSC_VER
   #pragma warning(disable:4800)
#endif

namespace sfg {

class DBMetaTable;
class DBMetaField;
class DBMetaFields;

//---------------------------------------------------------------------------
class DBMetaTable : public DBObject, public virtual IDBMetaTable
{
private:
   friend class DBMetaField;
   friend class DBMetaFields;
private: //Variáveis
   String        sTableName;
   DBConnection* oConnection;
   DBMetaFields* oMetaFields;
public: //Properties
   IDBConnection* getConnection();
   IDBMetaFields* getMetaFields();
   void setMetaFields(IDBMetaFields* oMetaFields);
   const String& getTableName();
   SQLRETURN setTableName(const String& Value);
public: //Methods
   DBMetaTable(IDBConnection* oConnection);
   DBMetaTable(IDBConnection* oConnection, const String& sTableName, bool bCreateFields = true);
   ~DBMetaTable();
   IDBMetaField* FieldByIndex(int nIndex);
   IDBMetaField* FieldByName(const String& sName);
};
//---------------------------------------------------------------------------
class DBMetaField : public DBObject, public virtual IDBMetaField
{
private:
   friend class DBMetaTable;
   friend class DBMetaFields;
private: //Variáveis
   DBMetaFields* oMetaFields;
   DataType      eDataType;
   int           nFieldNum;
   String        sFieldName;
   int           nDataSize;
   int           nDecimalDigits;
   int           nProperties;
   MetaFieldEdit eEditCmp;
   String        sEditMask;
   String        sFieldLabel;
   String        sCheckValues;
   String        sComboOptions;
   String        sFkTableName;
   String        sFkColumnName;
   bool          bAutoGenerate;
   bool          bIsVirtual;
public: //Properties
   IDBMetaFields* getMetaFields();
   int getFieldNum() {return nFieldNum;};
   void setFieldNum(const int Value) {nFieldNum = Value;};
   const String& getFieldName() {return sFieldName;};
   void setFieldName(const String &Value) {sFieldName = Value;};
   DataType getDataType() {return eDataType;};
   void setDataType(const DataType Value) {eDataType = Value;};
   int getDataSize() {return nDataSize;};
   void setDataSize(const int Value) {nDataSize = Value;};
   int getDecimalDigits() {return nDecimalDigits;};
   void setDecimalDigits(const int Value) {nDecimalDigits = Value;};
   int getProperties() {return nProperties;};
   void setProperties(const int Value) {nProperties = Value;};
   bool getIsPkKey() {return (bool)(nProperties & MFA_PKKEY);};
   void setIsPkKey(const bool Value);
   bool getNotNull() {return (bool)(nProperties & MFA_NOTNULL);};
   void setNotNull(const bool Value);
   bool getAutoIncrement() {return (bool)(nProperties & MFA_AUTOINC);};
   void setAutoIncrement(const bool Value);   
   bool getIsFkKey() {return (bool)(nProperties & MFA_FKKEY);};
   void setIsFkKey(const bool Value);
   bool getIsIndex() {return (bool)(nProperties & MFA_INDEX);};
   void setIsIndex(const bool Value);
   bool getIsFkList() {return (bool)(nProperties & MFA_FKLIST);};
   void setIsFkList(const bool Value);   
   bool getIsBrowse() {return (bool)(nProperties & MFA_BROWSE);};
   void setIsBrowse(const bool Value);
   bool getIsFilter() {return (bool)(nProperties & MFA_FILTER);};
   void setIsFilter(const bool Value);
   bool getIsSearch() {return (bool)(nProperties & MFA_SEARCH);};
   void setIsSearch(const bool Value);
   bool getIsOrder() {return (bool)(nProperties & MFA_ORDER);};
   void setIsOrder(const bool Value);   
   bool getIsMoney() {return (bool)(nProperties & MFA_MONEY);};
   void setIsMoney(const bool Value);
   MetaFieldEdit getEditCmp() {return eEditCmp;};
   void setEditCmp(const MetaFieldEdit Value) {eEditCmp = Value;};
   const String& getEditMask() {return sEditMask;};
   void setEditMask(const String& Value) {sEditMask = Value;};
   const String& getFieldLabel() {return sFieldLabel;};
   void setFieldLabel(const String& Value) {sFieldLabel = Value;};
   const String& getCheckValues() {return sCheckValues;};
   void setCheckValues(const String& Value) {sCheckValues = Value;};
   const String& getComboOptions() {return sComboOptions;};
   void setComboOptions(const String& Value) {sComboOptions = Value;};
   const String& getFkTableName() {return sFkTableName;};
   void setFkTableName(const String& Value) {sFkTableName = Value;};
   const String& getFkColumnName() {return sFkColumnName;};
   void setFkColumnName(const String& Value) {sFkColumnName = Value;};
   bool getIsVirtual() {return bIsVirtual;};
   void setIsVirtual(const bool Value) {bIsVirtual = Value;};
public: //Methods
   DBMetaField(IDBMetaFields* oMetaFields);
   DBMetaField(IDBMetaFields* oMetaFields, int nFieldNum, const String& sFieldName, const String& sFieldLabel,
               MetaFieldEdit eEditCmp, DataType eDataType, int nDataSize, int nDecimalDigits = 0, 
               bool bNotNull = false, bool bIsPkKey = false, bool bAutoIncrement = false, int nProperties = 0,
               const String& sEditMask = _T(""), const String& sCheckValues = _T("1;0"), 
               const String& sFkTableName = _T(""), const String& sFkColumnName = _T(""));
   ~DBMetaField();
};
//---------------------------------------------------------------------------
typedef std::vector<DBMetaField*> DBMetaFieldList;
class DBMetaFields : public DBObject, public virtual IDBMetaFields 
{
private:
   friend class DBMetaTable;
   friend class DBMetaField;
private: //Variáveis
   DBMetaTable*     oMetaTable;
   DBMetaFieldList* oListFields;
   bool             bAutoGenerate;
public: //Properties
   IDBMetaTable* getMetaTable();
public: //Methods
   DBMetaFields(IDBMetaTable* oMetaTable);
   ~DBMetaFields();
   int Count();
   IDBMetaField* FieldByIndex(int nIndex);
   IDBMetaField* FieldByName(const String& sName);
   IDBMetaField* Add(IDBMetaField* oMetaField);
   IDBMetaField* Insert(int nIndex, IDBMetaField* oMetaField);
   IDBMetaField* Remove(int nIndex);
};
//---------------------------------------------------------------------------
class DBRecordsetSchema : public DBRecordset, public virtual IDBRecordsetSchema
{
protected: //Variáveis
   bool bMetadataID;
public: //Proprieadades
   bool getMetadataID() {return bMetadataID;};
public: //Methods
   DBRecordsetSchema(IDBConnection* oConnection); 
   ~DBRecordsetSchema(){};
   SQLRETURN Initialize();
};
//---------------------------------------------------------------------------
}; //end namespace sfg
//---------------------------------------------------------------------------
#endif
