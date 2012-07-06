//---------------------------------------------------------------------------------------------------------------------------------
// File: OdbcConnection.h
// Contents: Async calls to ODBC done in background thread
// 
// Copyright Microsoft Corporation and contributors
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

#include "ResultSet.h"

namespace mssql
{
    using namespace std;

    class OdbcConnection
    {
    private:
        static OdbcEnvironmentHandle environment;
        OdbcConnectionHandle connection;
        OdbcStatementHandle statement;

        enum ConnectionStates
        {
            Closed,
            Opening,
            TurnOffAutoCommit,
            Open
        } connectionState;

        enum ExecutionStates
        {
            Idle,
            Executing,
            CountingColumns,
            Metadata,
            CountRows,
            FetchRow,
            FetchColumn,
            NextResults
        } executionState;

        int column;
        bool endOfResults;

    public:
        shared_ptr<ResultSet> resultset;

        OdbcConnection()
            : connectionState(Closed),
              executionState(Idle),
              column(0),
              endOfResults(true)
        {
        }

        static void InitializeEnvironment();

        bool TryBeginTran();
        bool TryClose();
        bool TryOpen(const wstring& connectionString);
        bool TryExecute(const wstring& query);
        bool TryEndTran(SQLSMALLINT completionType);
        bool TryReadRow();
        bool TryReadColumn(int column);
        bool TryReadNextResult();

        Handle<Value> GetMetaValue()
        {
            HandleScope scope;
            return scope.Close(resultset->MetaToValue());
        }

        Handle<Value> EndOfResults()
        {
            HandleScope scope;
            return scope.Close( Boolean::New( endOfResults ));
        }

        Handle<Value> EndOfRows()
        {
            HandleScope scope;
            return scope.Close(Boolean::New(resultset->EndOfRows()));
        }

        Handle<Value> GetColumnValue()
        {
            HandleScope scope;
            Local<Object> result = Object::New();
            shared_ptr<Column> column = resultset->GetColumn();
            result->Set(New(L"data"), column->ToValue());
            result->Set(New(L"more"), Boolean::New(column->More()));
            return scope.Close(result);
        }
    };

}
