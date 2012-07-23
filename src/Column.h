//---------------------------------------------------------------------------------------------------------------------------------
// File: Column.h
// Contents: Column objects from SQL Server to return as Javascript types
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

    class Column
    {
    public:
        virtual Handle<Value> ToValue() = 0;
        virtual bool More() const { return false; }
    };

    class StringColumn : public Column
    {
    public:
        StringColumn(const wchar_t* text, bool more) : text(text), more(more) {}
        Handle<Value> ToValue()
        {
            HandleScope scope;
            return scope.Close(String::New(reinterpret_cast<const uint16_t*>(text.c_str()), text.length()));
        }
        void Add(const wchar_t* additionalText)
        {
            text.append(additionalText);
        }
        bool More() const { return more; }
    private:
        wstring text;
        bool more;
    };
    
    class BinaryColumn : public Column
    {
    public:
        BinaryColumn(vector<char>& src, bool more)
            : more(more)
        {
            buffer = move(src);
        }
        Handle<Value> ToValue()
        {
            HandleScope scope;
            
            int length = buffer.size();

            char* destination = new char[length];

            memcpy(destination, buffer.data(), length);

            return scope.Close(node::Buffer::New(destination, length, deleteBuffer, nullptr)->handle_);
        }
        bool More() const { return more; }
        
        static void deleteBuffer(char* ptr, void* hint)
        {
            delete[] ptr;
        }

    private:
        vector<char> buffer;
        bool more;
    };

    class IntColumn : public Column
    {
    public:
        IntColumn(long value) : value(value) {}
        Handle<Value> ToValue()
        {
            HandleScope scope;
            return scope.Close(Integer::New(value));
        }

    private:
       int value;
    };

    class NullColumn : public Column
    {
    public:
       Handle<Value> ToValue()
       {
           HandleScope scope;
           return scope.Close(Null());
       }
    };

    class NumberColumn : public Column
    {
    public:
        NumberColumn(double value) : value(value) {}
        Handle<Value> ToValue()

        {
           HandleScope scope;
           return scope.Close(Number::New(value));
        }

    private:
        double value;
    };

    // Timestamps return dates in UTC timezone
    class TimestampColumn : public Column 
    {
    public:

        TimestampColumn( SQL_SS_TIMESTAMPOFFSET_STRUCT const& timeStruct )
        {
            MillisecondsFromDate( timeStruct );
        }
        Handle<Value> ToValue()
        {
            HandleScope scope;

            Local<Date> date = Local<Date>::Cast( Date::New( milliseconds ));

            // include the properties for items in a DATETIMEOFFSET that are not included in a JS Date object
            date->Set( String::NewSymbol( "nanosecondsDelta" ), 
                       Number::New( nanoseconds_delta / ( NANOSECONDS_PER_MS * 1000.0 )));

            return scope.Close( date );
        }

    private:

        static const int64_t NANOSECONDS_PER_MS = 1000000;                  // nanoseconds per millisecond

        double milliseconds;
        int32_t nanoseconds_delta;    // just the fractional part of the time passed in, not since epoch time

        // return the number of days since Jan 1, 1970
        double DaysSinceEpoch( SQLSMALLINT y, SQLUSMALLINT m, SQLUSMALLINT d )
        {
            // table derived from ECMA 262 15.9.1.4
            static const double days_in_months[] = { 0.0, 31.0, 59.0, 90.0, 120.0, 151.0, 181.0, 212.0, 243.0, 273.0, 304.0, 334.0 };

            double days;

            // calculate the number of days to the start of the year
            days = 365.0 * (y-1970.0) + floor((y-1969.0)/4.0) - floor((y-1901.0)/100.0) + floor((y-1601.0)/400.0);

            // add in the number of days from the month
            days += days_in_months[ m - 1 ];

            // and finally add in the day from the date to the number of days elapsed
            days += d - 1.0;

            // account for leap year this year (affects days after Feb. 29)
            if((( y % 4 == 0 && y % 100 != 0 ) || y % 400 == 0 ) && m > 2 ) {
                days += 1.0;
            }

            return (double) floor( days );
        }

        // derived from ECMA 262 15.9
        void MillisecondsFromDate( SQL_SS_TIMESTAMPOFFSET_STRUCT const& timeStruct )
        {
            const double MS_PER_SECOND      = 1000.0;
            const double MS_PER_MINUTE      = 60.0 * MS_PER_SECOND;
            const double MS_PER_HOUR        = 60.0 * MS_PER_MINUTE;
            const double MS_PER_DAY         = 24.0 * MS_PER_HOUR;

            double ms = DaysSinceEpoch( timeStruct.year, timeStruct.month, timeStruct.day );
            ms *= MS_PER_DAY;

            // add in the hour, day minute, second and millisecond
            // TODO: How to handle the loss of precision from the datetimeoffset fields?
            ms += timeStruct.hour * MS_PER_HOUR + timeStruct.minute * MS_PER_MINUTE + timeStruct.second * MS_PER_SECOND;
            ms += timeStruct.fraction / NANOSECONDS_PER_MS;    // fraction is in nanoseconds

            // handle timezone adjustment to UTC
            ms += timeStruct.timezone_hour * MS_PER_HOUR;
            ms += timeStruct.timezone_minute * MS_PER_MINUTE;

            milliseconds = ms;

            nanoseconds_delta = timeStruct.fraction % NANOSECONDS_PER_MS;
        }
    };

    class BoolColumn : public Column
    {
    public:
        BoolColumn( bool value) : value(value) {}
        Handle<Value> ToValue()
        {
            HandleScope scope;
            return scope.Close(Boolean::New(value));
        }
    private:
        bool value;
    };
}
