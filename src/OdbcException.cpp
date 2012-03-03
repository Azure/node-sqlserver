
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