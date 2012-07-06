//---------------------------------------------------------------------------------------------------------------------------------
// File: OdbcOperation.h
// Contents: ODBC Operation objects called on background thread
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

#include "Operation.h"
#include "OdbcConnection.h"

namespace mssql
{
    using namespace std;
    using namespace v8;

    class OdbcOperation : public Operation
    {
    protected:
        shared_ptr<OdbcConnection> connection;
    private:
        Persistent<Function> callback;
        bool failed;
        bool completed;
        exception failure;
    public:
        OdbcOperation(shared_ptr<OdbcConnection> connection, Handle<Object> callback)
            : connection(connection), 
              callback(Persistent<Function>::New(callback.As<Function>())),
              failed(false),
              completed(false)
        {
        }

        virtual bool TryInvokeOdbc() = 0;
        virtual Handle<Value> CreateCompletionArg() = 0;

        void InvokeBackground() override;
        void CompleteForeground() override;
    };

    class OpenOperation : public OdbcOperation
    {
    private:
        wstring connectionString;
        Persistent<Object> backpointer;
    public:
        OpenOperation(shared_ptr<OdbcConnection> connection, const wstring& connectionString, Handle<Object> callback, Handle<Object> backpointer)
            : OdbcOperation(connection, callback), 
              connectionString(connectionString), 
              backpointer(Persistent<Object>::New(backpointer))
        {
        }

        bool TryInvokeOdbc() override
        {
            return connection->TryOpen(connectionString);
        }

        Handle<Value> CreateCompletionArg() override
        {
            HandleScope scope;
            return scope.Close(backpointer);
        }

    };
    
    class QueryOperation : public OdbcOperation
    {
    private:
        wstring query;
    public:
        QueryOperation(shared_ptr<OdbcConnection> connection, const wstring& query, Handle<Object> callback)
            : OdbcOperation(connection, callback), 
              query(query)
        {
        }

        bool TryInvokeOdbc() override
        {
            return connection->TryExecute(query);
        }

        Handle<Value> CreateCompletionArg() override
        {
            HandleScope scope;
            return scope.Close( connection->GetMetaValue() );
        }

    };
    
    class ReadRowOperation : public OdbcOperation
    {
    public:
        ReadRowOperation(shared_ptr<OdbcConnection> connection, Handle<Object> callback)
            : OdbcOperation(connection, callback)
        {
        }

        bool TryInvokeOdbc() override
        {
            return connection->TryReadRow();
        }

        Handle<Value> CreateCompletionArg() override
        {
            HandleScope scope;
            return scope.Close(connection->EndOfRows());
        }

    };
    
    class ReadColumnOperation : public OdbcOperation
    {
    private:
        int column;
    public:
        ReadColumnOperation(shared_ptr<OdbcConnection> connection, int column, Handle<Object> callback)
            : OdbcOperation(connection, callback),
              column(column)
        {
        }

        bool TryInvokeOdbc() override
        {
            return connection->TryReadColumn(column);
        }

        Handle<Value> CreateCompletionArg() override
        {
            HandleScope scope;
            return scope.Close(connection->GetColumnValue());
        }

    };
    
    class ReadNextResultOperation : public OdbcOperation
    {
    public:
        ReadNextResultOperation(shared_ptr<OdbcConnection> connection, Handle<Object> callback)
            : OdbcOperation(connection, callback)
        {
        }

        bool TryInvokeOdbc() override
        {
            return connection->TryReadNextResult();
        }

        Handle<Value> CreateCompletionArg() override
        {
            HandleScope scope;

            Local<Object> more_meta = Object::New();

            more_meta->Set( String::NewSymbol( "endOfResults" ), connection->EndOfResults() );
            more_meta->Set( String::NewSymbol( "meta" ), connection->GetMetaValue() );

            return scope.Close( more_meta );
        }

    };

    class CloseOperation : public OdbcOperation
    {
    public:
        CloseOperation(shared_ptr<OdbcConnection> connection, Handle<Object> callback)
            : OdbcOperation(connection, callback)
        {
        }

        bool TryInvokeOdbc() override
        {
            return connection->TryClose();
        }

        Handle<Value> CreateCompletionArg() override
        {
            HandleScope scope;
            return scope.Close( Undefined() );
        }

    };

    class BeginTranOperation : public OdbcOperation
    {
    public:
        BeginTranOperation(shared_ptr<OdbcConnection> connection, Handle<Object> callback )
            : OdbcOperation(connection, callback)
        {
        }

        bool TryInvokeOdbc() override
        {
            return connection->TryBeginTran();
        }

        Handle<Value> CreateCompletionArg() override
        {
            HandleScope scope;
            return scope.Close( Undefined() );
        }
    };

    class EndTranOperation : public OdbcOperation
    {
    private:

        SQLSMALLINT completionType;

    public:
        EndTranOperation(shared_ptr<OdbcConnection> connection, SQLSMALLINT completionType, Handle<Object> callback)
            : OdbcOperation(connection, callback), 
              completionType(completionType)
        {
        }

        bool TryInvokeOdbc() override
        {
            return connection->TryEndTran(completionType);
        }

        Handle<Value> CreateCompletionArg() override
        {
            HandleScope scope;
            return scope.Close( Undefined() );
        }

    };
}

