
#pragma once

namespace mssql {

using namespace std;

class Operation
{
public:
    virtual void InvokeBackground() = 0;
    virtual void CompleteForeground() = 0;
    virtual ~Operation() {};

    static void Add(Operation* operation)
    {
        operation->work.data = operation;
        int result = uv_queue_work(uv_default_loop(), &operation->work, OnBackground, OnForeground);
        if (result != 0)
        {
            uv_err_t error = uv_last_error(uv_default_loop());
            throw OdbcException(uv_strerror(error));
        }
    }
private:
    uv_work_t work;

    static void OnBackground(uv_work_t* work)
    {
        Operation* operation = (Operation*)work->data;
        operation->InvokeBackground();
    }

    static void OnForeground(uv_work_t* work)
    {
        Operation* operation = (Operation*)work->data;
        operation->CompleteForeground();
        delete operation;
    }

};

}