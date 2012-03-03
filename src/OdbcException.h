
#pragma once

namespace mssql
{
    using namespace std;

    class OdbcException : public exception
    {
    public:
        OdbcException(const char* message)
            : exception(message)
        {
        }
        OdbcException(const char* message, const string& code)
            : exception(message), code(code)
        {
        }
        OdbcException(const wchar_t* message)
            : exception(w2a(message).c_str())
        {
        }

        const string& GetCode() const { return code; }

        static OdbcException Create(SQLSMALLINT handleType, SQLHANDLE handle);
    private:
        string code;
    };
}