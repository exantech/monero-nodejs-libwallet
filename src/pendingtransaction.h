#pragma once

#include <node.h>
#include <node_object_wrap.h>

namespace Monero {
class PendingTransaction;
}

namespace exawallet {

class PendingTransaction : public node::ObjectWrap {
public:
    explicit PendingTransaction(Monero::PendingTransaction* tx): transaction(tx) {}
    virtual ~PendingTransaction();

    static void Init(v8::Isolate* isolate);
    static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
    static v8::Local<v8::Object> NewInstance(v8::Isolate* isolate, Monero::PendingTransaction* tx);

    static void Commit(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Amount(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Dust(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Fee(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void TransactionsIds(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void TransactionsCount(const v8::FunctionCallbackInfo<v8::Value>& args);

private:
    static v8::Persistent<v8::Function> constructor;

    Monero::PendingTransaction* transaction;
};

}
