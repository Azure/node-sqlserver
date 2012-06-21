//---------------------------------------------------------------------------------------------------------------------------------
// File: OdbcHandle.h
// Contents: Object to manage ODBC handles
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
