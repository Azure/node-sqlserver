#pragma once

#include "Operation.h"
#include "OdbcConnection.h"
#include "OdbcOperation.h"

namespace mssql
{
    using namespace std;
    using namespace v8;

    class OdbcConnectionBridge
    {
    public:
        OdbcConnectionBridge()
        {
            connection = make_shared<OdbcConnection>();
        }

        Handle<Value> Close(Handle<Object> callback)
        {
            HandleScope scope;

            Operation* operation = new CloseOperation(connection, callback);
            Operation::Add(operation);

            return scope.Close(Undefined());
        }

        Handle<Value> Commit(Handle<Object> callback)
        {
            HandleScope scope;

            Operation* operation = new EndTranOperation(connection, SQL_COMMIT, callback);
            Operation::Add(operation);

            return scope.Close(Undefined());
        }

        Handle<Value> Rollback(Handle<Object> callback)
        {
            HandleScope scope;

            Operation* operation = new EndTranOperation(connection, SQL_ROLLBACK, callback);
            Operation::Add(operation);

            return scope.Close(Undefined());
        }

        Handle<Value> Query(Handle<String> query, Handle<Object> callback)
        {
            HandleScope scope;

            Operation* operation = new QueryOperation(connection, FromV8String(query), callback);
            Operation::Add(operation);

            return scope.Close(Undefined());
        }
        
        Handle<Value> ReadRow(Handle<Object> callback)
        {
            HandleScope scope;

            Operation* operation = new ReadRowOperation(connection, callback);
            Operation::Add(operation);

            return scope.Close(Undefined());
        }
        
        Handle<Value> ReadNextResult(Handle<Object> callback)
        {
            HandleScope scope;

            Operation* operation = new ReadNextResultOperation(connection, callback);
            Operation::Add(operation);

            return scope.Close(Undefined());
        }

        Handle<Value> ReadColumn(Handle<Number> column, Handle<Object> callback)
        {
            HandleScope scope;

            Operation* operation = new ReadColumnOperation(connection, column->Int32Value(), callback);
            Operation::Add(operation);

            return scope.Close(Undefined());
        }

        Handle<Value> Open(Handle<String> connectionString, Handle<Object> callback, Handle<Object> backpointer)
        {
            HandleScope scope;

            Operation* operation = new OpenOperation(connection, FromV8String(connectionString), callback, backpointer);
            Operation::Add(operation);

            return scope.Close(Undefined());
        }
    private:

        shared_ptr<OdbcConnection> connection;
    };

}

