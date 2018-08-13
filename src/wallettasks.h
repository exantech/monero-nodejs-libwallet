#pragma once

#include "deferredtask.h"
#include "walletargs.h"

namespace exawallet {

class CreateWalletTask: public DeferredTask {
public:
    CreateWalletTask(v8::Isolate* isolate, const CreateWalletArgs& args): DeferredTask(isolate), args_(args) {}

    virtual std::string doWork() override;
    virtual v8::Local<v8::Value> afterWork(v8::Isolate* isolate, std::string& error) override;

private:
    CreateWalletArgs args_;
    Monero::Wallet* wallet_ = nullptr;
};

class OpenWalletTask: public DeferredTask {
public:
    OpenWalletTask(v8::Isolate* isolate, const OpenWalletArgs& args): DeferredTask(isolate), args_(args) {}

    virtual std::string doWork() override;
    virtual v8::Local<v8::Value> afterWork(v8::Isolate* isolate, std::string& error) override;

private:
    OpenWalletArgs args_;
    Monero::Wallet* wallet_ = nullptr;
};

class CloseWalletTask: public DeferredTask {
public:
    CloseWalletTask(v8::Isolate* isolate, Monero::Wallet* wallet, bool store): DeferredTask(isolate), wallet_(wallet), store_(store) {}

    virtual std::string doWork() override;
    virtual v8::Local<v8::Value> afterWork(v8::Isolate* isolate, std::string& error) override;

private:
    Monero::Wallet* wallet_ = nullptr;
    bool store_;
};

class RecoveryWalletTask: public DeferredTask {
public:
    RecoveryWalletTask(v8::Isolate* isolate, const RecoveryWalletArgs& args): DeferredTask(isolate), args_(args) {}

    virtual std::string doWork() override;
    virtual v8::Local<v8::Value> afterWork(v8::Isolate* isolate, std::string& error) override;

private:
    RecoveryWalletArgs args_;
    Monero::Wallet* wallet_ = nullptr;
};


class StoreWalletTask: public DeferredTask {
public:
    StoreWalletTask(v8::Isolate* isolate, Monero::Wallet* wallet): DeferredTask(isolate), wallet_(wallet) {}

    virtual std::string doWork() override;
    virtual v8::Local<v8::Value> afterWork(v8::Isolate* isolate, std::string& error) override;
private:
    Monero::Wallet* wallet_;
};

class CreateTransactionTask: public DeferredTask {
public:
    CreateTransactionTask(v8::Isolate* isolate, const CreateTransactionArgs& args, Monero::Wallet* wallet): DeferredTask(isolate), args_(args), wallet_(wallet) {}

    virtual std::string doWork() override;
    virtual v8::Local<v8::Value> afterWork(v8::Isolate* isolate, std::string& error) override;

private:
    CreateTransactionArgs args_;
    Monero::PendingTransaction* transaction_ = nullptr;
    Monero::Wallet* wallet_;
};

class CommitTransactionTask: public DeferredTask {
public:
    CommitTransactionTask(v8::Isolate* isolate, Monero::PendingTransaction* transaction): DeferredTask(isolate), transaction_(transaction) {}

    virtual std::string doWork() override;
    virtual v8::Local<v8::Value> afterWork(v8::Isolate* isolate, std::string& error) override;

private:
    Monero::PendingTransaction* transaction_;
};

class RestoreMultisigTransactionTask: public DeferredTask {
public:
    RestoreMultisigTransactionTask(v8::Isolate* isolate, const std::string& transactionData, Monero::Wallet* wallet): DeferredTask(isolate), transactionData_(transactionData), wallet_(wallet) {}

    virtual std::string doWork() override;
    virtual v8::Local<v8::Value> afterWork(v8::Isolate* isolate, std::string& error) override;

private:
    std::string transactionData_;
    Monero::PendingTransaction* transaction_ = nullptr;
    Monero::Wallet* wallet_;
};

} //namespace exawallet
