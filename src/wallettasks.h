#pragma once

#include "deferredtask.h"
#include "walletargs.h"

namespace exawallet {

class CreateWalletTask: public DeferredTask {
public:
    CreateWalletTask(const CreateWalletArgs& args): args_(args) {}

    virtual std::string doWork() override;
    virtual v8::Local<v8::Value> afterWork(std::string& error) override;

private:
    CreateWalletArgs args_;
    Monero::Wallet* wallet_ = nullptr;
};

class OpenWalletTask: public DeferredTask {
public:
    OpenWalletTask(const OpenWalletArgs& args): args_(args) {}

    virtual std::string doWork() override;
    virtual v8::Local<v8::Value> afterWork(std::string& error) override;

private:
    OpenWalletArgs args_;
    Monero::Wallet* wallet_ = nullptr;
};

class CloseWalletTask: public DeferredTask {
public:
    CloseWalletTask(Monero::Wallet* wallet, bool store): wallet_(wallet), store_(store) {}

    virtual std::string doWork() override;
    virtual v8::Local<v8::Value> afterWork(std::string& error) override;

private:
    Monero::Wallet* wallet_ = nullptr;
    bool store_;
};

class RecoveryWalletTask: public DeferredTask {
public:
    RecoveryWalletTask(const RecoveryWalletArgs& args): args_(args) {}

    virtual std::string doWork() override;
    virtual v8::Local<v8::Value> afterWork(std::string& error) override;

private:
    RecoveryWalletArgs args_;
    Monero::Wallet* wallet_ = nullptr;
};


class StoreWalletTask: public DeferredTask {
public:
    StoreWalletTask(Monero::Wallet* wallet): wallet_(wallet) {}

    virtual std::string doWork() override;
    virtual v8::Local<v8::Value> afterWork(std::string& error) override;
private:
    Monero::Wallet* wallet_;
};

class CreateTransactionTask: public DeferredTask {
public:
    CreateTransactionTask(const CreateTransactionArgs& args, Monero::Wallet* wallet): args_(args), wallet_(wallet) {}

    virtual std::string doWork() override;
    virtual v8::Local<v8::Value> afterWork(std::string& error) override;

private:
    CreateTransactionArgs args_;
    Monero::PendingTransaction* transaction_ = nullptr;
    Monero::Wallet* wallet_;
};

class CommitTransactionTask: public DeferredTask {
public:
    CommitTransactionTask(Monero::PendingTransaction* transaction): transaction_(transaction) {}

    virtual std::string doWork() override;
    virtual v8::Local<v8::Value> afterWork(std::string& error) override;

private:
    Monero::PendingTransaction* transaction_;
};

class RestoreMultisigTransactionTask: public DeferredTask {
public:
    RestoreMultisigTransactionTask(const std::string& transactionData, Monero::Wallet* wallet)
        : transactionData_(transactionData), wallet_(wallet) {}

    virtual std::string doWork() override;
    virtual v8::Local<v8::Value> afterWork(std::string& error) override;

private:
    std::string transactionData_;
    Monero::PendingTransaction* transaction_ = nullptr;
    Monero::Wallet* wallet_;
};

} //namespace exawallet
