//---------------------------------------------------------------------------------------------------------------------------------
// File: OdbcOperation.cpp
// Contents: Functions called by thread queue for background ODBC operations
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
