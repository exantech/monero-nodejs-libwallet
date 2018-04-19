#pragma once

#include "wallet.h"

#include <uv.h>

namespace exawallet {

// this call happens in event loop thread
template <typename T>
void makeCall(v8::Isolate* isolate, v8::Local<v8::Function> cb, const T& param);

template <>
void makeCall<uint64_t>(v8::Isolate* isolate, v8::Local<v8::Function> cb, const uint64_t& param) {
    v8::Local<v8::Value> argv[] = { v8::Integer::New(isolate, param) };
    v8::Local<v8::Function>::New(isolate, cb)->Call(isolate->GetCurrentContext()->Global(), 1, argv);
}

void deleteUvHandle(uv_handle_t* handle) {
    delete handle;
}

struct AsyncCallbackBase {
    AsyncCallbackBase() {
        async = new uv_async_t();
    }

    ~AsyncCallbackBase() {
        uv_close(reinterpret_cast<uv_handle_t*>(async), deleteUvHandle);
    }

    void Enqueue() {
        uv_async_send(async);
    }

    uv_async_t* async;
    std::string funcName;
    Wallet* wallet;
};

template<typename T>
struct AsyncCallback: AsyncCallbackBase {
    T param;

    void invoke(v8::Isolate* isolate) {
        auto maybeFunc = wallet->FindCallback(isolate, funcName);
        if (maybeFunc.IsEmpty()) {
            return;
        }

        v8::Local<v8::Function> cb = maybeFunc.ToLocalChecked();
        makeCall(isolate, cb, param);
    }
};

template <>
struct AsyncCallback<void>: AsyncCallbackBase {
    void invoke(v8::Isolate* isolate) {
        auto maybeFunc = wallet->FindCallback(isolate, funcName);
        if (maybeFunc.IsEmpty()) {
            return;
        }

        v8::Local<v8::Value> argv[] = { v8::Null(isolate) };
        maybeFunc.ToLocalChecked()->Call(isolate->GetCurrentContext()->Global(), 0, argv);
    }
};

template <typename T>
void mainThreadFunc(uv_async_t* handle) {
    AsyncCallback<T>* async = static_cast<AsyncCallback<T>*>(handle->data);

    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    {
        v8::HandleScope scope(isolate);
        async->invoke(isolate);
    }

    delete async;
}

template <typename T>
AsyncCallback<T>* constructAsyncCallback(Wallet* wallet, std::string funcName) {
    AsyncCallback<T>* cb = new AsyncCallback<T>();

    uv_async_init(uv_default_loop(), cb->async, mainThreadFunc<T>);
    cb->async->data = cb;
    cb->wallet = wallet;
    cb->funcName = funcName;

    return cb;
}

struct TransactionAmount {
    std::string tx;
    std::string amount;
};

template <>
void makeCall<TransactionAmount>(v8::Isolate* isolate, v8::Local<v8::Function> cb, const TransactionAmount& param) {
    v8::Local<v8::Value> argv[] = { v8::String::NewFromUtf8(isolate, param.tx.c_str()), v8::String::NewFromUtf8(isolate, param.amount.c_str()) };
    v8::Local<v8::Function>::New(isolate, cb)->Call(isolate->GetCurrentContext()->Global(), 2, argv);
}

} //namespace exawallet
