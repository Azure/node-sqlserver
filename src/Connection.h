#pragma once

#include "OdbcConnectionBridge.h"

namespace mssql
{
    using namespace std;
    using namespace v8;

    class Connection : node::ObjectWrap 
    {
    private:
        static Persistent<FunctionTemplate> constructor_template;

        unique_ptr<OdbcConnectionBridge> innerConnection;
        Persistent<Object> This;
    public:
        Connection()
            : innerConnection(new OdbcConnectionBridge())
        {
        }
        ~Connection()
        {
        }

        static void Initialize(Handle<Object> target);
        static Handle<Value> Close(const Arguments& args);
        static Handle<Value> Commit(const Arguments& args);
        static Handle<Value> Rollback(const Arguments& args);
        static Handle<Value> New(const Arguments& args);
        static Handle<Value> Open(const Arguments& args);
        static Handle<Value> Query(const Arguments& args);
        static Handle<Value> ReadRow(const Arguments& args);
        static Handle<Value> ReadColumn(const Arguments& args);
        static Handle<Value> ReadNextResult(const Arguments& args);
    };

}

