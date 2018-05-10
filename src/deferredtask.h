#pragma once

#include <node.h>
#include <uv.h>
#include <string>
namespace exawallet {

// only dynamic allocated
class DeferredTask {
public:
    DeferredTask(v8::Isolate* isolate);
    virtual ~DeferredTask();

    void Resolve(v8::Isolate* isolate, v8::Local<v8::Value> value);
    void Reject(v8::Isolate* isolate);
    v8::Local<v8::Promise> Enqueue(v8::Isolate* isolate);

    static void AsyncWork(uv_work_t* work);
    static void AsyncAfterWork(uv_work_t* work, int status);
    static void Async(const v8::FunctionCallbackInfo<v8::Value>& args);

    // for long time operations in separate thread
    virtual std::string doWork() = 0;

    // for operations to be made in event loop thread like js object creation
    virtual v8::Local<v8::Value> afterWork(v8::Isolate* isolate, std::string&) = 0;

private:
    v8::Persistent<v8::Promise::Resolver> resolver;
    std::string errorString;
    uv_work_t work;
};

} // namespace exawallet
