
#include "stdafx.h"
#include "ResultSet.h"

namespace mssql
{
    using namespace v8;

    Handle<Value> ResultSet::MetaToValue()
    {
        HandleScope scope;

        Local<Array> metadata = Array::New();
        for_each(this->metadata.begin(), this->metadata.end(), [&metadata](const ColumnDefinition& definition) {
            HandleScope scope;

            const wchar_t* typeName = L"";
            switch (definition.dataType)
            {
            case SQL_CHAR:
            case SQL_VARCHAR:
            case SQL_LONGVARCHAR:
            case SQL_WCHAR:
            case SQL_WVARCHAR:
            case SQL_WLONGVARCHAR:
                typeName = L"text";
                break;
            case SQL_SMALLINT:
            case SQL_BIT:
            case SQL_TINYINT:
            case SQL_INTEGER:
            case SQL_DECIMAL:
            case SQL_NUMERIC:
            case SQL_REAL:
            case SQL_FLOAT:
            case SQL_DOUBLE:
            case SQL_BIGINT:
                typeName = L"number";
                break;
            case SQL_BINARY:
            case SQL_VARBINARY:
            case SQL_LONGVARBINARY:
            case SQL_GUID:
            case SQL_TYPE_TIME:
            case SQL_TYPE_TIMESTAMP:
            case SQL_TYPE_DATE:
            default:
                typeName = L"binary";
                break;
            }

            Local<Object> entry = Object::New();
            entry->Set(New(L"name"), New(definition.name.c_str()));
            entry->Set(New(L"size"), Integer::New(definition.columnSize));
            entry->Set(New(L"nullable"), Boolean::New(definition.nullable != 0));
            entry->Set(New(L"type"), New(typeName));

            metadata->Set(metadata->Length(), entry);
        });

        return scope.Close(metadata);
    }
}