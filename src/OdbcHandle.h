
#pragma once

namespace mssql
{
    using namespace std;

    template<SQLSMALLINT Type>
    class OdbcHandle
    {
    public:
        OdbcHandle() : handle(nullptr) {}
        OdbcHandle(OdbcHandle&& orig) : handle(orig.handle) 
        { 
            orig.handle = nullptr; 
        }
        ~OdbcHandle()
        {
            Free();
        }
        OdbcHandle& operator=(OdbcHandle&& orig)
        {
            handle = orig.handle;
            orig.handle = nullptr;
            return *this;
        }

        OdbcHandle& Alloc()
        {
            assert(handle == NULL);
            SQLRETURN ret = SQLAllocHandle(Type, nullptr, &handle);
            if (!SQL_SUCCEEDED(ret))
            {
                throw OdbcException(L"Unable to allocate ODBC handle");
            }
            return *this;
        }

        template<SQLSMALLINT ParentType>
        OdbcHandle& Alloc(const OdbcHandle<ParentType>& parent)
        {
            assert(handle == NULL);
            SQLRETURN ret = SQLAllocHandle(Type, parent, &handle);
            if (!SQL_SUCCEEDED(ret))
            {
                throw OdbcException(L"Unable to allocate ODBC handle");
            }
            return *this;
        }

        void Free()
        {
            if (handle != nullptr)
            {
                SQLFreeHandle(Type, handle);
                handle = nullptr;
            }
        }

        void Throw()
        {
            throw OdbcException::Create(Type, handle);
        }

        operator SQLHENV() const { return handle; }
    private:
        void operator=(const OdbcHandle& orig) 
        {
            assert(false);
        }

        SQLHENV handle;
    };

    typedef OdbcHandle<SQL_HANDLE_ENV> OdbcEnvironmentHandle;
    typedef OdbcHandle<SQL_HANDLE_DBC> OdbcConnectionHandle;
    typedef OdbcHandle<SQL_HANDLE_STMT> OdbcStatementHandle;
}