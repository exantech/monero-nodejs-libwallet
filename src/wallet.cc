#include "wallet.h"

#include <iostream>
#include <algorithm>

#include "deferredtask.h"
#include "walletargs.h"
#include "wallettasks.h"

namespace exawallet {

using namespace v8;

namespace {

std::string toStdString(Isolate* isolate, const Local<Value>& val) {
  String::Utf8Value utf8val(isolate, val);
  return std::string(*utf8val, utf8val.length());
}

bool toVectorString(Isolate* isolate, Local<Value> args, std::vector<std::string>& append) {
    Local<Array> items = Local<Array>::Cast(args);
    append.reserve(append.size() + items->Length());
    for (uint32_t i = 0; i < items->Length(); ++i) {
        auto msigInfo = items->Get(isolate->GetCurrentContext(), i).ToLocalChecked();
        if (!msigInfo->IsString()) {
            return false;
        }

        append.emplace_back(toStdString(isolate, msigInfo));
    }

    return true;
}

std::map<std::string, Monero::NetworkType> nettypes {
    {"mainnet", Monero::MAINNET},
    {"testnet", Monero::TESTNET},
    {"stagenet", Monero::STAGENET}
};

bool getNettype(const std::string& netstring, Monero::NetworkType& type) {
    auto it = nettypes.find(netstring);
    if (it == nettypes.end()) {
        return false;
    }

    type = it->second;
    return true;
}

bool convertNettype(Monero::NetworkType type, std::string& netstring) {
    auto it = std::find_if(nettypes.begin(), nettypes.end(), [type] (const std::pair<std::string, Monero::NetworkType> item) { return item.second == type; });
    if (it == nettypes.end()) {
        return false;
    }

    netstring = it->first;
    return true;
}

}

Persistent<Function> Wallet::constructor;

Wallet::~Wallet() {
    if (wallet_) {
        delete wallet_;
    }
}

void Wallet::WalletExists(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();
    if (args.Length() != 1 || !args[0]->IsString()) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Function accepts path to wallet")));
        return;
    }

    String::Utf8Value path(isolate, args[0]->ToString(isolate));
    auto manager = Monero::WalletManagerFactory::getWalletManager();
    bool exists = manager->walletExists(*path);
    args.GetReturnValue().Set(Boolean::New(isolate, exists));
}

void Wallet::CreateWallet(const FunctionCallbackInfo<Value>& args) {
    CreateWalletArgs walletArgs;
    std::string error = walletArgs.Init(args);
    if (!error.empty()) {
        args.GetIsolate()->ThrowException(Exception::Error(String::NewFromUtf8(args.GetIsolate(), error.c_str())));
        return;
    }

    CreateWalletTask* task = new CreateWalletTask(args.GetIsolate(), walletArgs);
    auto promise = task->Enqueue(args.GetIsolate());
    args.GetReturnValue().Set(promise);
}

void Wallet::OpenWallet(const FunctionCallbackInfo<Value>& args) {
    OpenWalletArgs walletArgs;
    std::string error = walletArgs.Init(args);
    if (!error.empty()) {
        args.GetIsolate()->ThrowException(Exception::Error(String::NewFromUtf8(args.GetIsolate(), error.c_str())));
        return;
    }

    OpenWalletTask* task = new OpenWalletTask(args.GetIsolate(), walletArgs);
    auto promise = task->Enqueue(args.GetIsolate());
    args.GetReturnValue().Set(promise);
}

MaybeLocal<Function> Wallet::FindCallback(Isolate* isolate, const std::string& name) {
    auto it = callbacks_.find(name);
    if (it == callbacks_.end()) {
        return MaybeLocal<Function>();
    }

    return MaybeLocal<Function>(Local<Function>::New(isolate, it->second));
}

void Wallet::Init(Isolate* isolate) {
    struct FunctionRegisterInfo {
        const char* name;
        FunctionCallback func;
    };

    static std::vector<FunctionRegisterInfo> walletFunctions = {
        {"address", Address},
        {"seed", Seed},
        {"on", On},
        {"store", Store},
        {"path", Path},
        {"network", NetType},
        {"secretViewKey", SecretViewKey},
        {"publicViewKey", PublicViewKey},
        {"secretSpendKey", SecretSpendKey},
        {"publicSpendKey", PublicSpendKey},
        {"setPassword", SetPassword},
        {"setRefreshFromBlockHeight", SetRefreshFromBlockHeight},
        {"getRefreshFromBlockHeight", GetRefreshFromBlockHeight},
        {"connected", Connected},
        {"setTrustedDaemon", SetTrustedDaemon},
        {"trustedDaemon", TrustedDaemon},
        {"balance", Balance},
        {"unlockedBalance", UnlockedBalance},
        {"blockchainHeight", BlockChainHeight},
        {"daemonBlockchainHeight", DaemonBlockChainHeight},
        {"synchronized", Synchronized},
        {"defaultMixin", DefaultMixin},
        {"setDefaultMixin", SetDefaultMixin},
        {"startRefresh", StartRefresh},
        {"pauseRefresh", PauseRefresh},
        {"createTransaction", CreateTransaction},
        {"publicMultisigSignerKey", PublicMultisigSignerKey},
        {"getMultisigInfo", GetMultisigInfo},
        {"makeMultisig", MakeMultisig},
        {"finalizeMultisig", FinalizeMultisig},
        {"exportMultisigImages", ExportMultisigImages},
        {"importMultisigImages", ImportMultisigImages},
        {"restoreMultisigTransaction", RestoreMultisigTransaction},
        {"multisigState", MultisigState},
        {"signMessage", SignMessage},
        {"verifySignedMessage", VerifySignedMessage},
        {"signMultisigParticipant", SignMultisigParticipant},
        {"verifyMessageWithPublicKey", VerifyMessageWithPublicKey}
    };

    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "Wallet"));
    tpl->InstanceTemplate()->SetInternalFieldCount(walletFunctions.size());

    for (const auto& info: walletFunctions) {
        NODE_SET_PROTOTYPE_METHOD(tpl, info.name, info.func);
    }

    constructor.Reset(isolate, tpl->GetFunction());
}

v8::Local<v8::Object> Wallet::NewInstance(v8::Isolate* isolate, Monero::Wallet* wallet) {
    const unsigned argc = 0;
    Local<Value> argv[1] = { Null(isolate) };
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    Local<Context> context = isolate->GetCurrentContext();
    Local<Object> instance = cons->NewInstance(context, argc, argv).ToLocalChecked();

    Wallet* w = new Wallet(wallet);
    wallet->setListener(w);
    //TODO: make it configurable
    wallet->segregatePreForkOutputs(false);
    wallet->keyReuseMitigation2(false);

    w->Wrap(instance);
    return instance;
}

void Wallet::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.IsConstructCall()) {
    Wallet* obj = new Wallet(nullptr);
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

void Wallet::Address(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());

    auto buf = obj->wallet_->address();
    auto addr = String::NewFromUtf8(isolate, buf.c_str());

    args.GetReturnValue().Set(addr);
}

void Wallet::Seed(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());

    auto buf = obj->wallet_->seed();
    auto seed = String::NewFromUtf8(isolate, buf.c_str());

    args.GetReturnValue().Set(seed);
}

void Wallet::On(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());

    if (args.Length() != 2) {
        isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "2 arguments are required")));
        return;
    }

    if (!args[0]->IsString() || !args[1]->IsFunction()) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Function accepts string and function arguments")));
        return;
    }

    obj->callbacks_[toStdString(isolate, args[0])] = CopyablePersistentFunction(isolate, Local<Function>::Cast(args[1]));
    args.GetReturnValue().Set(args.Holder());
}

void Wallet::Store(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());

    StoreWalletTask* task = new StoreWalletTask(isolate, obj->wallet_);
    auto promise = task->Enqueue(isolate);
    args.GetReturnValue().Set(promise);
}

void Wallet::Path(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, obj->wallet_->path().c_str()));
}

void Wallet::NetType(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());

    std::string nettype;
    if (!convertNettype(obj->wallet_->nettype(), nettype)) {
        assert(0);
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, nettype.c_str()));
}

void Wallet::SecretViewKey(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, obj->wallet_->secretViewKey().c_str()));
}

void Wallet::PublicViewKey(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, obj->wallet_->publicViewKey().c_str()));
}

void Wallet::SecretSpendKey(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, obj->wallet_->secretSpendKey().c_str()));
}

void Wallet::PublicSpendKey(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, obj->wallet_->publicSpendKey().c_str()));
}

void Wallet::SetPassword(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    if (args.Length() != 1 || !args[0]->IsString()) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(args.GetIsolate(), "String argument is required")));
        return;
    }

    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());
    if (!obj->wallet_->setPassword(toStdString(isolate, args[0]->ToString(isolate)))) {
        isolate->ThrowException(Exception::Error(String::NewFromUtf8(args.GetIsolate(), obj->wallet_->errorString().c_str())));
        return;
    }

    args.GetReturnValue().Set(args.Holder());
}

void Wallet::SetRefreshFromBlockHeight(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();

    if (args.Length() != 1 || !args[0]->IsInt32()) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(args.GetIsolate(), "Integer argument is required")));
        return;
    }

    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());
    obj->wallet_->setRefreshFromBlockHeight(args[0]->Uint32Value(isolate->GetCurrentContext()).ToChecked());

    args.GetReturnValue().Set(args.Holder());
}

void Wallet::GetRefreshFromBlockHeight(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());

    args.GetReturnValue().Set(Integer::NewFromUnsigned(isolate, obj->wallet_->getRefreshFromBlockHeight()));
}

void Wallet::Connected(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());

    std::string status;
    switch (obj->wallet_->connected()) {
    case Monero::Wallet::ConnectionStatus_Connected:
        status = "connected";
        break;
    case Monero::Wallet::ConnectionStatus_Disconnected:
        status = "disconnected";
        break;
    case Monero::Wallet::ConnectionStatus_WrongVersion:
        status = "incompatible";
        break;
    default:
        assert(0);
        break;
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, status.c_str()));
}

void Wallet::SetTrustedDaemon(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();

    if (args.Length() != 1 || !args[0]->IsBoolean()) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(args.GetIsolate(), "Integer argument is required")));
        return;
    }

    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());
    obj->wallet_->setTrustedDaemon(args[0]->ToBoolean(isolate)->Value());

    args.GetReturnValue().Set(args.Holder());
}

void Wallet::TrustedDaemon(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());

    args.GetReturnValue().Set(Boolean::New(isolate, obj->wallet_->trustedDaemon()));
}

void Wallet::Balance(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());

    // it seems v8 doesn't have uint64
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, std::to_string(obj->wallet_->balanceAll()).c_str()));
}

void Wallet::UnlockedBalance(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, std::to_string(obj->wallet_->unlockedBalanceAll()).c_str()));
}

void Wallet::BlockChainHeight(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());

    args.GetReturnValue().Set(Uint32::NewFromUnsigned(isolate, obj->wallet_->blockChainHeight()));
}

void Wallet::DaemonBlockChainHeight(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());

    args.GetReturnValue().Set(Uint32::NewFromUnsigned(isolate, obj->wallet_->daemonBlockChainHeight()));
}

void Wallet::Synchronized(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());

    args.GetReturnValue().Set(Boolean::New(isolate, obj->wallet_->synchronized()));
}

void Wallet::GenPaymentId(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, Monero::Wallet::genPaymentId().c_str()));
}

void Wallet::PaymentIdValid(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    if (args.Length() != 1 || !args[0]->IsString()) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "String argument is required")));
        return;
    }

    args.GetReturnValue().Set(Boolean::New(isolate, Monero::Wallet::paymentIdValid(toStdString(isolate, args[0]->ToString(isolate)))));
}

void Wallet::AddressValid(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    if (args.Length() != 2 || !args[0]->IsString() || !args[1]->IsString()) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "2 string arguments are required")));
        return;
    }

    Monero::NetworkType nettype;
    if (!getNettype(toStdString(isolate, args[1]->ToString(isolate)), nettype)) {
        isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "wrong network type argument")));
        return;
    }

    bool valid = Monero::Wallet::addressValid(toStdString(isolate, args[0]->ToString()), nettype);
    args.GetReturnValue().Set(Boolean::New(isolate, valid));
}

void Wallet::DefaultMixin(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());

    args.GetReturnValue().Set(Integer::New(isolate, obj->wallet_->defaultMixin()));
}

void Wallet::SetDefaultMixin(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    if (args.Length() != 1 || !args[0]->IsInt32()) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Integer argument is required")));
        return;
    }

    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());
    obj->wallet_->setDefaultMixin(args[0]->ToInt32(isolate)->Value());
}

void Wallet::StartRefresh(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());
    obj->wallet_->startRefresh();
}

void Wallet::PauseRefresh(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());
    obj->wallet_->pauseRefresh();
}

void Wallet::CreateTransaction(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    CreateTransactionArgs txArgs;
    std::string error = txArgs.Init(args);
    if (!error.empty()) {
        isolate->ThrowException(Exception::Error(String::NewFromUtf8(args.GetIsolate(), error.c_str())));
        return;
    }

    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());
    CreateTransactionTask* task = new CreateTransactionTask(isolate, txArgs, obj->wallet_);
    auto promise = task->Enqueue(isolate);
    args.GetReturnValue().Set(promise);
}

void Wallet::PublicMultisigSignerKey(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());

    auto signerKey = obj->wallet_->publicMultisigSignerKey();
    if (signerKey.empty()) {
        isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "The wallet is not multisig")));
        return;
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, signerKey.c_str()));
}

void Wallet::GetMultisigInfo(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());

    auto msigInfo = obj->wallet_->getMultisigInfo();

    int status;
    std::string errorString;
    obj->wallet_->statusWithErrorString(status, errorString);
    if (status != Monero::Wallet::Status_Ok) {
        isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, errorString.c_str())));
        return;
    }

    return args.GetReturnValue().Set(String::NewFromUtf8(isolate, msigInfo.c_str()));
}

void Wallet::MakeMultisig(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    if (args.Length() != 2 || !args[0]->IsArray() || !args[1]->IsInt32()) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Function accepts array of strings and integer arguments")));
        return;
    }

    std::vector<std::string> infos;
    if (!toVectorString(isolate, args[0], infos)) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Function accepts array of strings and integer arguments")));
        return;
    }

    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());
    auto threshold = Local<Uint32>::Cast(args[1]);
    auto extraInfo = obj->wallet_->makeMultisig(infos, threshold->Value());

    int status;
    std::string errorString;
    obj->wallet_->statusWithErrorString(status, errorString);
    if (status != Monero::Wallet::Status_Ok) {
        isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, errorString.c_str())));
        return;
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, extraInfo.c_str()));
}

void Wallet::FinalizeMultisig(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    if (args.Length() != 1 || !args[0]->IsArray()) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Function accepts array of strings argument")));
        return;
    }

    std::vector<std::string> extraInfos;
    if (!toVectorString(isolate, args[0], extraInfos)) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Function accepts array of strings arguments")));
        return;
    }

    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());
    if (!obj->wallet_->finalizeMultisig(extraInfos)) {
        isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, obj->wallet_->errorString().c_str())));
        return;
    }
}

void Wallet::ExportMultisigImages(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());

    std::string images;
    if (!obj->wallet_->exportMultisigImages(images)) {
        auto errorString = obj->wallet_->errorString();
        isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, errorString.c_str())));
        return;
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, images.c_str()));
}

void Wallet::ImportMultisigImages(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    if (args.Length() != 1 || !args[0]->IsArray()) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Function accepts array of strings argument")));
        return;
    }

    std::vector<std::string> images;
    if (!toVectorString(isolate, args[0], images)) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Function accepts array of strings arguments")));
        return;
    }

    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());
    uint32_t imported = obj->wallet_->importMultisigImages(images);

    int status;
    std::string errorString;
    obj->wallet_->statusWithErrorString(status, errorString);
    if (status != Monero::Wallet::Status_Ok) {
        isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, errorString.c_str())));
        return;
    }

    args.GetReturnValue().Set(imported);
}

void Wallet::RestoreMultisigTransaction(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    if (args.Length() != 1 || !args[0]->IsString()) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Function accepts string argument")));
        return;
    }

    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());
    RestoreMultisigTransactionTask* task = new RestoreMultisigTransactionTask(isolate, toStdString(isolate, args[0]), obj->wallet_);
    auto promise = task->Enqueue(isolate);
    args.GetReturnValue().Set(promise);
}

void Wallet::MultisigState(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());

    auto state = obj->wallet_->multisig();
    Local<Object> res = Object::New(isolate);
    res->Set(isolate->GetCurrentContext(),
             String::NewFromUtf8(isolate, "isMultisig"),
             Boolean::New(isolate, state.isMultisig));
    res->Set(isolate->GetCurrentContext(),
             String::NewFromUtf8(isolate, "isReady"),
             Boolean::New(isolate, state.isReady));
    res->Set(isolate->GetCurrentContext(),
             String::NewFromUtf8(isolate, "threshold"),
             Uint32::NewFromUnsigned(isolate, state.threshold));
    res->Set(isolate->GetCurrentContext(),
             String::NewFromUtf8(isolate, "total"),
             Uint32::NewFromUnsigned(isolate, state.total));

    args.GetReturnValue().Set(res);
}

void Wallet::SignMessage(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();

    if (args.Length() != 1 || !args[0]->IsString()) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Function accepts string argument")));
        return;
    }

    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());
    auto signature = obj->wallet_->signMessage(toStdString(isolate, args[0]));
    if (obj->wallet_->status() != Monero::Wallet::Status_Ok) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, obj->wallet_->errorString().c_str())));
        return;
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, signature.c_str()));
}

void Wallet::VerifySignedMessage(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();

    if (args.Length() != 3 || !args[0]->IsString() || !args[1]->IsString() || !args[2]->IsString()) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Function accepts message, monero address and signature as string arguments")));
        return;
    }

    auto message = toStdString(isolate, args[0]);
    auto address = toStdString(isolate, args[1]);
    auto signature = toStdString(isolate, args[2]);

    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());
    bool valid = obj->wallet_->verifySignedMessage(message, address, signature);

    if (obj->wallet_->status() != Monero::Wallet::Status_Ok) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, obj->wallet_->errorString().c_str())));
        return;
    }

    args.GetReturnValue().Set(valid);
}

void Wallet::SignMultisigParticipant(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();

    if (args.Length() != 1 || !args[0]->IsString()) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Function accepts string argument")));
        return;
    }

    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());
    auto signature = obj->wallet_->signMultisigParticipant(toStdString(isolate, args[0]));
    if (obj->wallet_->status() != Monero::Wallet::Status_Ok) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, obj->wallet_->errorString().c_str())));
        return;
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, signature.c_str()));
}

void Wallet::VerifyMessageWithPublicKey(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();

    if (args.Length() != 3 || !args[0]->IsString() || !args[1]->IsString() || !args[2]->IsString()) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Function accepts message, monero public key and signature as string arguments")));
        return;
    }

    auto message = toStdString(isolate, args[0]);
    auto publicKey = toStdString(isolate, args[1]);
    auto signature = toStdString(isolate, args[2]);

    Wallet* obj = ObjectWrap::Unwrap<Wallet>(args.Holder());
    bool valid = obj->wallet_->verifyMessageWithPublicKey(message, publicKey, signature);

    if (obj->wallet_->status() != Monero::Wallet::Status_Ok) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, obj->wallet_->errorString().c_str())));
        return;
    }

    args.GetReturnValue().Set(valid);
}

} //namespace exawallet
