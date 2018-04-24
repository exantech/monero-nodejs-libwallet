#include "pendingtransaction.h"

#include <cmath>

#include <wallet2_api.h>
#include "wallettasks.h"

using namespace v8;

namespace exawallet {

Persistent<Function> PendingTransaction::constructor;

PendingTransaction::~PendingTransaction() {
    if (transaction) {
        delete transaction;
    }
}

void PendingTransaction::Init(Isolate* isolate) {
    struct FunctionRegisterInfo {
        const char* name;
        FunctionCallback func;
    };

    static std::vector<FunctionRegisterInfo> functions = {
        {"commit", Commit},
        {"amount", Amount},
        {"dust", Dust},
        {"fee", Fee},
        {"transactionsIds", TransactionsIds},
        {"transactionsCount", TransactionsCount},
        {"multisigSignData", MultisigSignData},
        {"signersKeys", SignersKeys},
        {"signMultisigTransaction", SignMultisigTransaction},
    };

    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "PendingTransaction"));
    tpl->InstanceTemplate()->SetInternalFieldCount(functions.size());

    for (const auto& info: functions) {
        NODE_SET_PROTOTYPE_METHOD(tpl, info.name, info.func);
    }

    constructor.Reset(isolate, tpl->GetFunction());
}

void PendingTransaction::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.IsConstructCall()) {
    PendingTransaction* obj = new PendingTransaction(nullptr);
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    const int argc = 0;
    Local<Value> argv[1] = { Null(isolate) };
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    Local<Context> context = isolate->GetCurrentContext();
    Local<Object> instance = cons->NewInstance(context, argc, argv).ToLocalChecked();
    args.GetReturnValue().Set(instance);
  }
}

Local<Object> PendingTransaction::NewInstance(Isolate* isolate, Monero::PendingTransaction* tx) {
    const unsigned argc = 0;
    Local<Value> argv[1] = { Null(isolate) };
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    Local<Context> context = isolate->GetCurrentContext();
    Local<Object> instance = cons->NewInstance(context, argc, argv).ToLocalChecked();

    PendingTransaction* t = new PendingTransaction(tx);
    t->Wrap(instance);
    return instance;
}

void PendingTransaction::Commit(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    PendingTransaction* obj = ObjectWrap::Unwrap<PendingTransaction>(args.Holder());

    CommitTransactionTask* task = new CommitTransactionTask(isolate, obj->transaction);
    auto promise = task->Enqueue(isolate);
    args.GetReturnValue().Set(promise);
}

void PendingTransaction::Amount(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    PendingTransaction* obj = ObjectWrap::Unwrap<PendingTransaction>(args.Holder());
    auto amount = std::to_string(obj->transaction->amount());

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, amount.c_str()));
}

void PendingTransaction::Dust(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    PendingTransaction* obj = ObjectWrap::Unwrap<PendingTransaction>(args.Holder());
    auto dust = std::to_string(obj->transaction->dust());

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, dust.c_str()));
}

void PendingTransaction::Fee(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    PendingTransaction* obj = ObjectWrap::Unwrap<PendingTransaction>(args.Holder());
    auto fee = std::to_string(obj->transaction->fee());

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, fee.c_str()));
}

void PendingTransaction::TransactionsIds(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    PendingTransaction* obj = ObjectWrap::Unwrap<PendingTransaction>(args.Holder());

    auto transactions = obj->transaction->txid();
    auto result = Array::New(isolate, transactions.size());

    for (size_t i = 0; i < transactions.size(); ++i) {
        auto id = String::NewFromUtf8(isolate, transactions[i].c_str());
        if (result->Set(isolate->GetCurrentContext(), i, id).IsNothing()) {
            isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Couldn't make transactions list: unknown error")));
            return;
        }
    }

    args.GetReturnValue().Set(result);
}

void PendingTransaction::TransactionsCount(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    PendingTransaction* obj = ObjectWrap::Unwrap<PendingTransaction>(args.Holder());
    args.GetReturnValue().Set(Uint32::NewFromUnsigned(isolate, obj->transaction->txCount()));
}

void PendingTransaction::MultisigSignData(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    PendingTransaction* obj = ObjectWrap::Unwrap<PendingTransaction>(args.Holder());

    auto signData = obj->transaction->multisigSignData();
    if (obj->transaction->status() != Monero::Wallet::Status_Ok) {
        isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, obj->transaction->errorString().c_str())));
        return;
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, signData.c_str()));
}

void PendingTransaction::SignersKeys(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    PendingTransaction* obj = ObjectWrap::Unwrap<PendingTransaction>(args.Holder());

    auto keys = obj->transaction->signersKeys();
    auto res = Array::New(isolate, keys.size());
    for (size_t i = 0; i < keys.size(); ++i) {
        auto key = keys[i];
        if (res->Set(isolate->GetCurrentContext(), i, String::NewFromUtf8(isolate, key.c_str())).IsNothing()) {
            isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Error occured during insertion value into the array")));
            return;
        }
    }

    args.GetReturnValue().Set(res);
}

void PendingTransaction::SignMultisigTransaction(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    PendingTransaction* obj = ObjectWrap::Unwrap<PendingTransaction>(args.Holder());

    obj->transaction->signMultisigTx();
    if (obj->transaction->status() != Monero::Wallet::Status_Ok) {
        isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, obj->transaction->errorString().c_str())));
        return;
    }
}

}
