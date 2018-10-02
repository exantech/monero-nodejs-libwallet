#include "pendingtransaction.h"

#include <cmath>

#include <wallet2_api.h>
#include "wallettasks.h"

using namespace v8;

namespace exawallet {

Nan::Persistent<Function> PendingTransaction::constructor;

PendingTransaction::~PendingTransaction() {
    if (transaction) {
        delete transaction;
    }
}

NAN_MODULE_INIT(PendingTransaction::Init) {
    struct FunctionRegisterInfo {
        const char* name;
        Nan::FunctionCallback func;
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

    Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(PendingTransaction::New);
    tpl->SetClassName(Nan::New("PendingTransaction").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(functions.size());

    for (const auto& info: functions) {
        Nan::SetPrototypeMethod(tpl, info.name, info.func);
    }

    constructor.Reset(tpl->GetFunction());
}

NAN_METHOD(PendingTransaction::New) {

  if (info.IsConstructCall()) {
    PendingTransaction* obj = new PendingTransaction(nullptr);
    obj->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } else {
    const int argc = 0;
    Local<Value> argv[1] = { Nan::Null() };
    Local<Function> cons = Nan::New(constructor);
    Local<Context> context = Nan::GetCurrentContext();
    Local<Object> instance = cons->NewInstance(context, argc, argv).ToLocalChecked();
    info.GetReturnValue().Set(instance);
  }
}

Local<Object> PendingTransaction::NewInstance(Monero::PendingTransaction* tx) {
    const unsigned argc = 0;
    Local<Value> argv[1] = { Nan::Null() };
    Local<Function> cons = Nan::New(constructor);
    Local<Context> context = Nan::GetCurrentContext();
    Local<Object> instance = cons->NewInstance(context, argc, argv).ToLocalChecked();

    PendingTransaction* t = new PendingTransaction(tx);
    t->Wrap(instance);
    return instance;
}

NAN_METHOD(PendingTransaction::Commit) {
    PendingTransaction* obj = ObjectWrap::Unwrap<PendingTransaction>(info.Holder());

    CommitTransactionTask* task = new CommitTransactionTask(obj->transaction);
    auto promise = task->Enqueue();
    info.GetReturnValue().Set(promise);
}

NAN_METHOD(PendingTransaction::Amount) {
    PendingTransaction* obj = ObjectWrap::Unwrap<PendingTransaction>(info.Holder());
    auto amount = std::to_string(obj->transaction->amount());

    info.GetReturnValue().Set(Nan::New(amount.c_str()).ToLocalChecked());
}

NAN_METHOD(PendingTransaction::Dust) {
    PendingTransaction* obj = ObjectWrap::Unwrap<PendingTransaction>(info.Holder());
    auto dust = std::to_string(obj->transaction->dust());

    info.GetReturnValue().Set(Nan::New(dust.c_str()).ToLocalChecked());
}

NAN_METHOD(PendingTransaction::Fee) {
    PendingTransaction* obj = ObjectWrap::Unwrap<PendingTransaction>(info.Holder());
    auto fee = std::to_string(obj->transaction->fee());

    info.GetReturnValue().Set(Nan::New(fee.c_str()).ToLocalChecked());
}

NAN_METHOD(PendingTransaction::TransactionsIds) {
    PendingTransaction* obj = ObjectWrap::Unwrap<PendingTransaction>(info.Holder());

    auto transactions = obj->transaction->txid();
    auto result = Nan::New<Array>(transactions.size());

    for (size_t i = 0; i < transactions.size(); ++i) {
        auto id = Nan::New(transactions[i].c_str()).ToLocalChecked();
        if (result->Set(Nan::GetCurrentContext(), i, id).IsNothing()) {
            Nan::ThrowError("Couldn't make transactions list: unknown error");
            return;
        }
    }

    info.GetReturnValue().Set(result);
}

NAN_METHOD(PendingTransaction::TransactionsCount) {
    PendingTransaction* obj = ObjectWrap::Unwrap<PendingTransaction>(info.Holder());
    info.GetReturnValue().Set(Nan::New((uint32_t)obj->transaction->txCount()));
}

NAN_METHOD(PendingTransaction::MultisigSignData) {
    PendingTransaction* obj = ObjectWrap::Unwrap<PendingTransaction>(info.Holder());

    auto signData = obj->transaction->multisigSignData();
    if (obj->transaction->status() != Monero::Wallet::Status_Ok) {
        Nan::ThrowError(obj->transaction->errorString().c_str());
        return;
    }

    info.GetReturnValue().Set(Nan::New(signData.c_str()).ToLocalChecked());
}

NAN_METHOD(PendingTransaction::SignersKeys) {
    PendingTransaction* obj = ObjectWrap::Unwrap<PendingTransaction>(info.Holder());

    auto keys = obj->transaction->signersKeys();
    auto res = Nan::New<Array>(keys.size());
    for (size_t i = 0; i < keys.size(); ++i) {
        auto key = keys[i];
        if (res->Set(Nan::GetCurrentContext(), i, Nan::New(key.c_str()).ToLocalChecked()).IsNothing()) {
            Nan::ThrowError("Error occured during insertion value into the array");
            return;
        }
    }

    info.GetReturnValue().Set(res);
}

NAN_METHOD(PendingTransaction::SignMultisigTransaction) {
    PendingTransaction* obj = ObjectWrap::Unwrap<PendingTransaction>(info.Holder());

    obj->transaction->signMultisigTx();
    if (obj->transaction->status() != Monero::Wallet::Status_Ok) {
        Nan::ThrowError(obj->transaction->errorString().c_str());
        return;
    }
}

}
