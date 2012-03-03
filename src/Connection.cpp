
#include "stdafx.h"
#include "Connection.h"

namespace mssql
{
    using namespace v8;

    Persistent<FunctionTemplate> Connection::constructor_template;

    void Connection::Initialize(Handle<Object> target)
    {
        HandleScope scope;

        Local<FunctionTemplate> t = FunctionTemplate::New(Connection::New);
        constructor_template = Persistent<FunctionTemplate>::New(t);
        constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
        constructor_template->SetClassName(String::NewSymbol("Connection"));

        NODE_SET_PROTOTYPE_METHOD(constructor_template, "close", Connection::Close);
        NODE_SET_PROTOTYPE_METHOD(constructor_template, "open", Connection::Open);
        NODE_SET_PROTOTYPE_METHOD(constructor_template, "query", Connection::Query);
        NODE_SET_PROTOTYPE_METHOD(constructor_template, "readRow", Connection::ReadRow);
        NODE_SET_PROTOTYPE_METHOD(constructor_template, "readColumn", Connection::ReadColumn);
        NODE_SET_PROTOTYPE_METHOD(constructor_template, "commit", Connection::Commit);
        NODE_SET_PROTOTYPE_METHOD(constructor_template, "rollback", Connection::Rollback);

        target->Set(String::NewSymbol("Connection"), constructor_template->GetFunction());
    }
    
    Handle<Value> Connection::Close(const Arguments& args)
    {
        HandleScope scope;

        Local<Object> callback = args[1].As<Object>();

        Connection* connection = Unwrap<Connection>(args.This());

        return scope.Close<Value>(connection->innerConnection->Close(callback));
    }

    Handle<Value> Connection::Commit(const Arguments& args)
    {
        HandleScope scope;

        Local<Object> callback = args[1].As<Object>();

        Connection* connection = Unwrap<Connection>(args.This());

        return scope.Close<Value>(connection->innerConnection->Commit(callback));
    }

    Handle<Value> Connection::Rollback(const Arguments& args)
    {
        HandleScope scope;

        Local<Object> callback = args[1].As<Object>();

        Connection* connection = Unwrap<Connection>(args.This());

        return scope.Close<Value>(connection->innerConnection->Rollback(callback));
    }

    Handle<Value> Connection::New(const Arguments& args) {
        HandleScope scope;

        if (!args.IsConstructCall()) {
            return Undefined();
        }

        Connection *s = new Connection();
        s->Wrap(args.This());
        s->This = Persistent<Object>::New(args.This());
        return args.This();
    }
    
    Handle<Value> Connection::Query(const Arguments& args)
    {
        HandleScope scope;

        Local<String> query = args[0].As<String>();
        Local<Object> callback = args[1].As<Object>();

        Connection* connection = Unwrap<Connection>(args.This());

        return scope.Close<Value>(connection->innerConnection->Query(query, callback));
    }
    
    Handle<Value> Connection::ReadRow(const Arguments& args)
    {
        HandleScope scope;

        Local<Object> callback = args[0].As<Object>();

        Connection* connection = Unwrap<Connection>(args.This());

        return scope.Close<Value>(connection->innerConnection->ReadRow(callback));
    }

    Handle<Value> Connection::ReadColumn(const Arguments& args)
    {
        HandleScope scope;

        Local<Number> column = args[0].As<Number>();
        Local<Object> callback = args[1].As<Object>();

        Connection* connection = Unwrap<Connection>(args.This());

        return scope.Close<Value>(connection->innerConnection->ReadColumn(column, callback));
    }
    
    Handle<Value> Connection::ReadNextResult(const Arguments& args)
    {
        HandleScope scope;

        Local<Object> callback = args[0].As<Object>();

        Connection* connection = Unwrap<Connection>(args.This());

        return scope.Close<Value>(connection->innerConnection->ReadNextResult(callback));
    }

    Handle<Value> Connection::Open(const Arguments& args)
    {
        HandleScope scope;

        Local<String> connectionString = args[0].As<String>();
        Local<Object> callback = args[1].As<Object>();

        Connection* connection = Unwrap<Connection>(args.This());

        return scope.Close<Value>(connection->innerConnection->Open(connectionString, callback, args.This()));
    }

}

NODE_MODULE(sqlserver, mssql::Connection::Initialize);

