
#include "stdafx.h"
#include "OdbcOperation.h"

namespace mssql
{
    void OdbcOperation::InvokeBackground()
    {
        try
        {
            completed = TryInvokeOdbc();
        }
        catch (exception& error)
        {
            completed = true;
            failed = true;
            failure = error;
        }
    }

    void OdbcOperation::CompleteForeground()
    {
        HandleScope scope;

        if (!callback->IsUndefined())
        {
            int argc;
            Local<Value> args[3];
            if (!completed) {
                args[0] = Local<Value>::New(Boolean::New(false));
                argc = 1;
            }
            else if (failed)
            {
                args[0] = Local<Value>::New(Boolean::New(true));
                args[1] = Exception::Error(String::New(failure.what()));
                argc = 2;
            }
            else
            {
                args[0] = Local<Value>::New(Boolean::New(true));
                args[1] = Local<Value>::New(Undefined());
                args[2] = Local<Value>::New(CreateCompletionArg());
                argc = 3;
            }

            callback->Call(Undefined().As<Object>(), argc, args);
        }

        callback.Dispose();
    }

}