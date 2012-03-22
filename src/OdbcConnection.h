
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
            FetchColumn
        } executionState;

        int column;

    public:
        shared_ptr<ResultSet> resultset;

        OdbcConnection()
            : connectionState(Closed),
              executionState(Idle)
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

        Handle<Value> MoreRows()
        {
            HandleScope scope;
            return scope.Close(Boolean::New(resultset->moreRows));
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
