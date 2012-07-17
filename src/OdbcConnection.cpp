//---------------------------------------------------------------------------------------------------------------------------------
// File: OdbcConnection.cpp
// Contents: Async calls to ODBC done in background thread
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
#include "OdbcConnection.h"

namespace mssql
{
    OdbcEnvironmentHandle OdbcConnection::environment;

    void OdbcConnection::InitializeEnvironment()
    {
        SQLRETURN ret = SQLSetEnvAttr(NULL, SQL_ATTR_CONNECTION_POOLING, (SQLPOINTER)SQL_CP_ONE_PER_HENV, 0);
        if (!SQL_SUCCEEDED(ret)) { throw OdbcException("Unable to initialize ODBC connection pooling"); }

        environment.Alloc();

        ret = SQLSetEnvAttr(environment, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
        if (!SQL_SUCCEEDED(ret)) { throw OdbcException::Create(SQL_HANDLE_ENV, environment); }
        ret = SQLSetEnvAttr(environment, SQL_ATTR_CP_MATCH, (SQLPOINTER)SQL_CP_RELAXED_MATCH, 0);
        if (!SQL_SUCCEEDED(ret)) { throw OdbcException::Create(SQL_HANDLE_ENV, environment); }
    }

    bool OdbcConnection::TryClose()
    {
        if (connectionState != Closed)
        {
            SQLRETURN ret = SQLDisconnect(connection);
            if (ret == SQL_STILL_EXECUTING) 
            { 
                return false; 
            }
            if (!SQL_SUCCEEDED(ret)) 
            { 
                connection.Throw();  
            }

            statement.Free();
            connection.Free();
            connectionState = Closed;
        }

        return true;
    }

    bool OdbcConnection::TryOpen(const wstring& connectionString)
    {
        SQLRETURN ret;

        if (connectionState == Closed)
        {
            OdbcConnectionHandle localConnection;

            localConnection.Alloc(environment);

            this->connection = std::move(localConnection);

            // TODO: determine async open support correctly
            // SQLSetConnectAttr(connection, SQL_ATTR_ASYNC_DBC_FUNCTIONS_ENABLE, (SQLPOINTER)SQL_ASYNC_DBC_ENABLE_ON, 0);

            connectionState = Opening;
        }

        if (connectionState == Opening)
        {
            ret = SQLDriverConnect(connection, NULL, const_cast<wchar_t*>(connectionString.c_str()), connectionString.length(), NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
            if (ret == SQL_STILL_EXECUTING) 
            { 
                return false; 
            }
            if (!SQL_SUCCEEDED(ret)) 
            { 
                connection.Throw();  
            }

            connectionState = Open;
            return true;
        }

        throw OdbcException("Attempt to open a connection that is not closed");
    }

    bool OdbcConnection::TryExecute(const wstring& query)
    {
        if (connectionState != Open)
        {
            throw OdbcException("Unable to execute a query on a connection that is not open");
        }

        if (executionState == Idle)
        {
            statement.Alloc(connection);
            
            // ignore failure - optional attribute
            SQLSetStmtAttr(statement, SQL_ATTR_ASYNC_ENABLE, (SQLPOINTER)SQL_ASYNC_ENABLE_ON, 0);

            executionState = Executing;
        }

        if (executionState == Executing)
        {
            endOfResults = true;     // reset 
            SQLRETURN ret = SQLExecDirect(statement, const_cast<wchar_t*>(query.c_str()), query.length());
            if (ret == SQL_STILL_EXECUTING) 
            { 
                return false; 
            }
            if (ret != SQL_NO_DATA && !SQL_SUCCEEDED(ret)) 
            { 
                statement.Throw();  
            }

            executionState = CountingColumns;
        }

        if (executionState == CountingColumns)
        {
            SQLSMALLINT columns;
            SQLRETURN ret = SQLNumResultCols(statement, &columns);
            if (ret == SQL_STILL_EXECUTING) 
            { 
                return false; 
            }
            if (!SQL_SUCCEEDED(ret)) 
            { 
                statement.Throw();  
            }        

            executionState = Metadata;
            column = 0;
            resultset = make_shared<ResultSet>(columns);

        }

        if (executionState == Metadata)
        {
            while (column < resultset->GetColumns())
            {
                SQLSMALLINT nameLength;
                SQLRETURN ret = SQLDescribeCol(statement, column + 1, nullptr, 0, &nameLength, nullptr, nullptr, nullptr, nullptr);
                if (ret == SQL_STILL_EXECUTING) 
                { 
                    return false; 
                }
                if (!SQL_SUCCEEDED(ret)) 
                { 
                    statement.Throw();  
                }
                ResultSet::ColumnDefinition& current = resultset->GetMetadata(column);
                vector<wchar_t> buffer(nameLength+1);
                ret = SQLDescribeCol(statement, column + 1, buffer.data(), nameLength+1, &nameLength, &current.dataType, &current.columnSize, &current.decimalDigits, &current.nullable);
                if (ret == SQL_STILL_EXECUTING) 
                { 
                    return false; 
                }
                if (!SQL_SUCCEEDED(ret)) 
                { 
                    statement.Throw();  
                }
                current.name = wstring(buffer.data(), nameLength);

                column++;
            }

            executionState = CountRows;
        }

        if (executionState == CountRows)
        {
            SQLLEN rowcount;
            SQLRETURN ret = SQLRowCount(statement, &rowcount);
            if (!SQL_SUCCEEDED(ret)) 
            { 
                statement.Throw();  
            }
            resultset->rowcount = rowcount;

            if (resultset->GetColumns() > 0)
            {
                executionState = FetchRow;
            }
            else 
            {
                executionState = NextResults;
            }
            
            return true;
        }

        throw OdbcException("The connection is in an invalid state");
    }

    bool OdbcConnection::TryReadRow()
    {
        if (executionState != FetchRow)
        {
            throw OdbcException("The connection is in an invalid state");
        }

        SQLRETURN ret = SQLFetch(statement);
        if (ret == SQL_STILL_EXECUTING) 
        { 
            return false; 
        }
        if (ret == SQL_NO_DATA) 
        { 
            resultset->endOfRows = true;
            executionState = NextResults;
            return true;
        }
        else 
        {
            resultset->endOfRows = false;
        }
        if (!SQL_SUCCEEDED(ret)) 
        { 
            statement.Throw();
        }

        return true;
    }

    bool OdbcConnection::TryReadColumn(int column)
    {
        if (executionState != FetchRow)
        {
            throw OdbcException("The connection is in an invalid state");
        }

        if (column < 0 || column >= resultset->GetColumns())
        {
            // TODO report an error
            return true;
        }

        SQLLEN strLen_or_IndPtr;
        const ResultSet::ColumnDefinition& definition = resultset->GetMetadata(column);
        switch (definition.dataType)
        {
        case SQL_CHAR:
        case SQL_VARCHAR:
        case SQL_LONGVARCHAR:
        case SQL_WCHAR:
        case SQL_WVARCHAR:
        case SQL_WLONGVARCHAR:
            {
                bool more = false;
                wchar_t buffer[2048+1] = {0};
                SQLRETURN ret = SQLGetData(statement, column + 1, SQL_C_WCHAR, buffer, sizeof(buffer)-sizeof(wchar_t), &strLen_or_IndPtr);
                if (ret == SQL_STILL_EXECUTING) 
                { 
                    return false; 
                }
                if (!SQL_SUCCEEDED(ret)) 
                { 
                    statement.Throw();  
                }
                if (strLen_or_IndPtr == SQL_NULL_DATA) 
                {
                    resultset->SetColumn(make_shared<NullColumn>());
                }
                else 
                {
                    SQLWCHAR SQLState[6];
                    SQLINTEGER nativeError;
                    SQLSMALLINT textLength;
                    if (ret == SQL_SUCCESS_WITH_INFO)
                    {
                        ret = SQLGetDiagRec(SQL_HANDLE_STMT, statement, 1, SQLState, &nativeError, NULL, 0, &textLength);
                        if (!SQL_SUCCEEDED(ret)) 
                        { 
                            statement.Throw();  
                        }
                        more = wcsncmp(SQLState, L"01004", 6) == 0;
                    }

                    resultset->SetColumn(make_shared<StringColumn>(buffer, more));
                }
            }
            break;
        case SQL_BIT:
            {
                long val;
                SQLRETURN ret = SQLGetData(statement, column + 1, SQL_C_SLONG, &val, sizeof(val), &strLen_or_IndPtr);
                if (ret == SQL_STILL_EXECUTING) 
                { 
                    return false; 
                }
                if (!SQL_SUCCEEDED(ret)) 
                { 
                    statement.Throw();  
                }
                if (strLen_or_IndPtr == SQL_NULL_DATA) 
                {
                    resultset->SetColumn(make_shared<NullColumn>());
                }
                else 
                {
                    resultset->SetColumn(make_shared<BoolColumn>((val != 0) ? true : false));
                }
            }
            break;
        case SQL_SMALLINT:
        case SQL_TINYINT:
        case SQL_INTEGER:
            {
                long val;
                SQLRETURN ret = SQLGetData(statement, column + 1, SQL_C_SLONG, &val, sizeof(val), &strLen_or_IndPtr);
                if (ret == SQL_STILL_EXECUTING) 
                { 
                    return false; 
                }
                if (!SQL_SUCCEEDED(ret)) 
                { 
                    statement.Throw();  
                }
                if (strLen_or_IndPtr == SQL_NULL_DATA) 
                {
                    resultset->SetColumn(make_shared<NullColumn>());
                }
                else 
                {
                    resultset->SetColumn(make_shared<IntColumn>(val));
                }
            }
            break;
        case SQL_DECIMAL:
        case SQL_NUMERIC:
        case SQL_REAL:
        case SQL_FLOAT:
        case SQL_DOUBLE:
        case SQL_BIGINT:
            {
                double val;
                SQLRETURN ret = SQLGetData(statement, column + 1, SQL_C_DOUBLE, &val, sizeof(val), &strLen_or_IndPtr);
                if (ret == SQL_STILL_EXECUTING) 
                { 
                    return false; 
                }
                if (!SQL_SUCCEEDED(ret)) 
                { 
                    statement.Throw();  
                }
                if (strLen_or_IndPtr == SQL_NULL_DATA) 
                {
                    resultset->SetColumn(make_shared<NullColumn>());
                }
                else 
                {
                    resultset->SetColumn(make_shared<NumberColumn>(val));
                }
            }
            break;
        case SQL_BINARY:
        case SQL_VARBINARY:
        case SQL_LONGVARBINARY:
            {
                bool more = false;
                vector<char> buffer(2048);
                SQLRETURN ret = SQLGetData(statement, column + 1, SQL_C_BINARY, buffer.data(), buffer.size(), &strLen_or_IndPtr);
                if (ret == SQL_STILL_EXECUTING) 
                { 
                    return false; 
                }
                if (!SQL_SUCCEEDED(ret)) 
                { 
                    statement.Throw();  
                }
                if (strLen_or_IndPtr == SQL_NULL_DATA) 
                {
                    resultset->SetColumn(make_shared<NullColumn>());
                }
                else 
                {
                    assert(strLen_or_IndPtr != SQL_NO_TOTAL); // per http://msdn.microsoft.com/en-us/library/windows/desktop/ms715441(v=vs.85).aspx

                    SQLWCHAR SQLState[6];
                    SQLINTEGER nativeError;
                    SQLSMALLINT textLength;
                    if (ret == SQL_SUCCESS_WITH_INFO)
                    {
                        ret = SQLGetDiagRec(SQL_HANDLE_STMT, statement, 1, SQLState, &nativeError, NULL, 0, &textLength);
                        if (!SQL_SUCCEEDED(ret)) 
                        { 
                            statement.Throw();  
                        }
                        more = wcsncmp(SQLState, L"01004", 6) == 0;
                    }

					int amount = strLen_or_IndPtr;
					if (more) {
						amount = buffer.size();
					}

                    vector<char> trimmed(amount);
                    memcpy(trimmed.data(), buffer.data(), amount);
                    resultset->SetColumn(make_shared<BinaryColumn>(trimmed, more));
                }
            }
            break;
        // use text format form time/date/etc.. for now
        // INTERVAL TYPES? 
        case SQL_GUID:
        case SQL_TYPE_TIME:
        case SQL_TYPE_TIMESTAMP:
        case SQL_TYPE_DATE:
        default:
            {
                // TODO: how to figure out the size?
                vector<wchar_t> buffer(8192+1);
                SQLRETURN ret = SQLGetData(statement, column + 1, SQL_C_WCHAR, buffer.data(), 8192*sizeof(wchar_t), &strLen_or_IndPtr);
                if (ret == SQL_STILL_EXECUTING) 
                { 
                    return false; 
                }
                if (!SQL_SUCCEEDED(ret)) 
                { 
                    statement.Throw();  
                }
                                
                resultset->SetColumn(make_shared<StringColumn>(buffer.data(), false));
            }
            break;
        }

        return true;
    }

    bool OdbcConnection::TryReadNextResult()
    {
        if (executionState != NextResults)
        {            
            throw OdbcException("The connection is in an invalid state");
        }

        SQLRETURN ret = SQLMoreResults(statement);
        if (ret == SQL_STILL_EXECUTING) 
        { 
            return false; 
        }
        if (ret == SQL_NO_DATA) 
        { 
            endOfResults = true;
            statement.Free();
            executionState = Idle;
            return true;
        }
        if (!SQL_SUCCEEDED(ret)) 
        { 
            statement.Throw();
        }

        endOfResults = false;
        executionState = CountingColumns;

        return TryExecute(L"");
    }

    bool OdbcConnection::TryBeginTran( void )
    {
        // turn off autocommit
        SQLRETURN ret = SQLSetConnectAttr( connection, SQL_ATTR_AUTOCOMMIT, reinterpret_cast<SQLPOINTER>( SQL_AUTOCOMMIT_OFF ),
                                           SQL_IS_UINTEGER );
        if (ret == SQL_STILL_EXECUTING) 
        { 
            return false; 
        }
        if (!SQL_SUCCEEDED(ret)) 
        { 
            statement.Throw();  
        }
        return true;
    }

    bool OdbcConnection::TryEndTran(SQLSMALLINT completionType)
    {
        SQLRETURN ret = SQLEndTran(SQL_HANDLE_DBC, connection, completionType);
        if (ret == SQL_STILL_EXECUTING) 
        { 
            return false; 
        }
        if (!SQL_SUCCEEDED(ret)) 
        { 
            statement.Throw();  
        }

        // put the connection back into auto commit mode
        ret = SQLSetConnectAttr( connection, SQL_ATTR_AUTOCOMMIT, reinterpret_cast<SQLPOINTER>( SQL_AUTOCOMMIT_ON ),
                                           SQL_IS_UINTEGER );
        // TODO: This will not work because calling into TryEndTran again from the callback will fail
        // when the completion has already finished.
        if (ret == SQL_STILL_EXECUTING) 
        { 
            return false; 
        }
        if (!SQL_SUCCEEDED(ret)) 
        { 
            statement.Throw();  
        }

        return true;
    }

}
