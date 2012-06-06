//---------------------------------------------------------------------------------------------------------------------------------
// File: OdbcException.cpp
// Contents: Create exception from ODBC error(s)
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

#include "stdafx.h"
#include "OdbcException.h"

namespace mssql
{
    OdbcException OdbcException::Create(SQLSMALLINT handleType, SQLHANDLE handle)
    {
        vector<wchar_t> buffer;

        SQLWCHAR wszSqlState[6];
        SQLINTEGER nativeError;
        SQLSMALLINT actual;
        if (!SQL_SUCCEEDED(SQLGetDiagRec(handleType, handle, 1, wszSqlState, &nativeError, NULL, 0, &actual)))
        {
            assert(false);
        }
        buffer.resize(actual+1);
        if (!SQL_SUCCEEDED(SQLGetDiagRec(handleType, handle, 1, wszSqlState, &nativeError, buffer.data(), actual+1, &actual)))
        {
            assert(false);
        }

        string code = w2a(wszSqlState);
        string message = code + ": " + w2a(buffer.data());
        return OdbcException(message.c_str(), code);
    }
}