#include "deferredtask.h"

namespace exawallet {

DeferredTask::DeferredTask(v8::Isolate* isolate) {
    resolver.Reset(isolate, v8::Promise::Resolver::New(isolate->GetCurrentContext()).ToLocalChecked());
}

DeferredTask::~DeferredTask() {
    resolver.Reset();
}

void DeferredTask::Resolve(v8::Isolate* isolate, v8::Local<v8::Value> value) {
    resolver.Get(isolate)->Resolve(isolate->GetCurrentContext(), value);
}

void DeferredTask::Reject(v8::Isolate* isolate) {
    resolver.Get(isolate)->Reject(isolate->GetCurrentContext(), v8::String::NewFromUtf8(isolate, errorString.c_str()));
}

v8::Local<v8::Promise> DeferredTask::Enqueue(v8::Isolate* isolate) {
    work.data = this;
    uv_queue_work(uv_default_loop(), &work, DeferredTask::AsyncWork, DeferredTask::AsyncAfterWork);
    return resolver.Get(isolate)->GetPromise();
}

void DeferredTask::AsyncWork(uv_work_t* work) {
    DeferredTask* deferred = reinterpret_cast<DeferredTask*>(work->data);
    deferred->errorString = deferred->doWork();
}

void DeferredTask::AsyncAfterWork(uv_work_t* work, int status) {
    //TODO: check if work is cancelled
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    DeferredTask* deferred = reinterpret_cast<DeferredTask*>(work->data);

    if (!deferred->errorString.empty()) {
        deferred->Reject(isolate);
        return;
    }

    v8::Local<v8::Value> value = deferred->afterWork(isolate, deferred->errorString);
    if (!deferred->errorString.empty()) {
        deferred->Reject(isolate);
        return;
    }

    deferred->Resolve(isolate, value);

    delete deferred;
}

}
