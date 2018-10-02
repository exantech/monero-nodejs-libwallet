#pragma once

#include <nan.h>

#include <map>
#include <mutex>

#include <wallet2_api.h>

namespace exawallet {

using CopyablePersistentFunction = Nan::CopyablePersistentTraits<v8::Function>::CopyablePersistent;

class Wallet : public node::ObjectWrap, public Monero::WalletListener {
public:
    static NAN_MODULE_INIT(Init);

    static v8::Local<v8::Object> NewInstance(Monero::Wallet* wallet);

    static NAN_METHOD(WalletExists);
    static NAN_METHOD(CreateWallet);
    static NAN_METHOD(OpenWallet);
    static NAN_METHOD(RecoveryWallet);
    static NAN_METHOD(GenPaymentId);
    static NAN_METHOD(PaymentIdValid);
    static NAN_METHOD(AddressValid);

    static NAN_METHOD(Close);

    // returns either callback or empty maybe.
    v8::MaybeLocal<v8::Function> FindCallback(const std::string& name);

 private:
    explicit Wallet(Monero::Wallet* wallet): wallet_(wallet) {}
    ~Wallet();

    virtual void moneySpent(const std::string &txId, uint64_t amount) override;
    virtual void moneyReceived(const std::string &txId, uint64_t amount) override;
    virtual void unconfirmedMoneyReceived(const std::string &txId, uint64_t amount) override;
    virtual void newBlock(uint64_t height) override;
    virtual void updated() override;
    virtual void refreshed() override;

    static NAN_METHOD(New);
    static NAN_METHOD(On);
    static NAN_METHOD(Off);

    static NAN_METHOD(Address);
    static NAN_METHOD(Seed);
    static NAN_METHOD(Store);
    static NAN_METHOD(Path);
    static NAN_METHOD(NetType);

    static NAN_METHOD(SecretViewKey);
    static NAN_METHOD(PublicViewKey);
    static NAN_METHOD(SecretSpendKey);
    static NAN_METHOD(PublicSpendKey);

    static NAN_METHOD(SetPassword);

    static NAN_METHOD(SetRefreshFromBlockHeight);
    static NAN_METHOD(GetRefreshFromBlockHeight);

    static NAN_METHOD(Connected);
    static NAN_METHOD(SetTrustedDaemon);
    static NAN_METHOD(TrustedDaemon);

    static NAN_METHOD(Balance);
    static NAN_METHOD(UnlockedBalance);

    static NAN_METHOD(BlockChainHeight);
    static NAN_METHOD(DaemonBlockChainHeight);

    static NAN_METHOD(Synchronized);

    static NAN_METHOD(DefaultMixin);
    static NAN_METHOD(SetDefaultMixin);

    static NAN_METHOD(StartRefresh);
    static NAN_METHOD(PauseRefresh);

    static NAN_METHOD(TransactionHistory);

    static NAN_METHOD(CreateTransaction);

    static NAN_METHOD(PublicMultisigSignerKey);
    static NAN_METHOD(GetMultisigInfo);
    static NAN_METHOD(MakeMultisig);
    static NAN_METHOD(FinalizeMultisig);
    static NAN_METHOD(ExportMultisigImages);
    static NAN_METHOD(ImportMultisigImages);
    static NAN_METHOD(RestoreMultisigTransaction);
    static NAN_METHOD(MultisigState);

    static NAN_METHOD(SignMessage);
    static NAN_METHOD(VerifySignedMessage);

    // multisig signatures
    static NAN_METHOD(SignMultisigParticipant);
    static NAN_METHOD(VerifyMessageWithPublicKey);

    static Nan::Persistent<v8::Function> constructor;

    Monero::Wallet* wallet_ = nullptr;
    std::map<std::string, CopyablePersistentFunction> callbacks_;
};

} //namespace exawallet
