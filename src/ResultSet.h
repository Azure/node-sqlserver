//---------------------------------------------------------------------------------------------------------------------------------
// File: ResultSet.h
// Contents: ResultSet object that holds metadata and current column to return to Javascript
// 
// Copyright Microsoft Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// You may obtain a copy of the License at:
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//---------------------------------------------------------------------------------------------------------------------------------

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