#pragma once

#include <node.h>
#include <node_object_wrap.h>

#include <map>
#include <mutex>

#include <wallet2_api.h>

namespace exawallet {

using CopyablePersistentFunction = v8::CopyablePersistentTraits<v8::Function>::CopyablePersistent;

class Wallet : public node::ObjectWrap, public Monero::WalletListener {
public:
    static void Init(v8::Isolate* isolate);
    static v8::Local<v8::Object> NewInstance(v8::Isolate* isolate, Monero::Wallet* wallet);

    static void WalletExists(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void CreateWallet(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void OpenWallet(const v8::FunctionCallbackInfo<v8::Value>& args);

    // returns either callback or empty maybe.
    v8::MaybeLocal<v8::Function> FindCallback(v8::Isolate* isolate, const std::string& name);

 private:
    explicit Wallet(Monero::Wallet* wallet): wallet_(wallet) {}
    ~Wallet();

    virtual void moneySpent(const std::string &txId, uint64_t amount) override;
    virtual void moneyReceived(const std::string &txId, uint64_t amount) override;
    virtual void unconfirmedMoneyReceived(const std::string &txId, uint64_t amount) override;
    virtual void newBlock(uint64_t height) override;
    virtual void updated() override;
    virtual void refreshed() override;

    static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void On(const v8::FunctionCallbackInfo<v8::Value>& args);

    static void Address(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Seed(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Store(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Path(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void NetType(const v8::FunctionCallbackInfo<v8::Value>& args);

    static void SecretViewKey(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void PublicViewKey(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void SecretSpendKey(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void PublicSpendKey(const v8::FunctionCallbackInfo<v8::Value>& args);

    static void SetPassword(const v8::FunctionCallbackInfo<v8::Value>& args);

    static void SetRefreshFromBlockHeight(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void GetRefreshFromBlockHeight(const v8::FunctionCallbackInfo<v8::Value>& args);

    static void Connected(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void SetTrustedDaemon(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void TrustedDaemon(const v8::FunctionCallbackInfo<v8::Value>& args);

    static void Balance(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void UnlockedBalance(const v8::FunctionCallbackInfo<v8::Value>& args);

    static void BlockChainHeight(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void DaemonBlockChainHeight(const v8::FunctionCallbackInfo<v8::Value>& args);

    static void Synchronized(const v8::FunctionCallbackInfo<v8::Value>& args);

    static void GenPaymentId(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void PaymentIdValid(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void AddressValid(const v8::FunctionCallbackInfo<v8::Value>& args);

    static void DefaultMixin(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void SetDefaultMixin(const v8::FunctionCallbackInfo<v8::Value>& args);

    static void StartRefresh(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void PauseRefresh(const v8::FunctionCallbackInfo<v8::Value>& args);

    static void CreateTransaction(const v8::FunctionCallbackInfo<v8::Value>& args);

    static v8::Persistent<v8::Function> constructor;

    Monero::Wallet* wallet_ = nullptr;
    std::map<std::string, CopyablePersistentFunction> callbacks_;
};

} //namespace exawallet
