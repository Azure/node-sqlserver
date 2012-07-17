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
