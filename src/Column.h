
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
}