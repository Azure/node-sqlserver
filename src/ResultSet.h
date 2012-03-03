
#pragma once

#include "Column.h"

namespace mssql
{
    using namespace std;

    class ResultSet
    {
    public:
        struct ColumnDefinition
        {
            wstring name;
            SQLULEN columnSize;
            SQLSMALLINT dataType;
            SQLSMALLINT decimalDigits;
            SQLSMALLINT nullable;
        };

        ResultSet(int columns) 
            : moreRows(false)
        {
            metadata.resize(columns);
        }
        ColumnDefinition& GetMetadata(int column)
        {
            return metadata[column];
        }
        int GetColumns() const
        {
            return metadata.size();
        }

        Handle<Value> MetaToValue();

        void SetColumn(shared_ptr<Column> column)
        {
            this->column = column;
        }
        shared_ptr<Column> GetColumn()
        {
            return column;
        }

    //private:
        vector<ColumnDefinition> metadata;
        SQLLEN rowcount;
        bool moreRows;
    private:
        shared_ptr<Column> column;
    };
}