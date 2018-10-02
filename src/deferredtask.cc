#include "deferredtask.h"

namespace exawallet {

DeferredTask::DeferredTask() {
    v8::Local<v8::Promise::Resolver> value =v8::Promise::Resolver::New(Nan::GetCurrentContext()).ToLocalChecked();
    resolver.Reset(value);
}

DeferredTask::~DeferredTask() {
    resolver.Reset();
}

void DeferredTask::Resolve(v8::Local<v8::Value> value) {
    Nan::New(resolver)->Resolve(Nan::GetCurrentContext(), value);
}

void DeferredTask::Reject() {
    v8::Local<v8::Context> context = Nan::GetCurrentContext();
    Nan::New(resolver)->Reject(context, Nan::New(errorString.c_str()).ToLocalChecked());
}

v8::Local<v8::Promise> DeferredTask::Enqueue() {
    work.data = this;
    uv_queue_work(uv_default_loop(), &work, DeferredTask::AsyncWork, DeferredTask::AsyncAfterWork);
    return Nan::New(resolver)->GetPromise();
}

void DeferredTask::AsyncWork(uv_work_t* work) {
    DeferredTask* deferred = reinterpret_cast<DeferredTask*>(work->data);
    deferred->errorString = deferred->doWork();
}

void DeferredTask::AsyncAfterWork(uv_work_t* work, int status) {
    //TODO: check if work is cancelled

    Nan::HandleScope scope;

    DeferredTask* deferred = reinterpret_cast<DeferredTask*>(work->data);

    if (!deferred->errorString.empty()) {
        deferred->Reject();
        return;
    }

    v8::Local<v8::Value> value = deferred->afterWork(deferred->errorString);
    if (!deferred->errorString.empty()) {
        deferred->Reject();
        return;
    }

    deferred->Resolve(value);

    delete deferred;
}

}
