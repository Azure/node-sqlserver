
#pragma once
#include <v8.h>

namespace mssql
{
    using namespace std;
    using namespace v8;

    inline Local<String> New(const wchar_t* text)
    {
        HandleScope scope;
        return scope.Close(String::New(reinterpret_cast<const uint16_t*>(text)));
    }

    wstring FromV8String(Handle<String> input);

    string w2a(const wchar_t* input);

}