#include "wallet.h"
#include "pendingtransaction.h"

extern void mlog_configure(const std::string &filename_base, bool console, const std::size_t max_log_file_size = 104850000);
extern void mlog_set_log(const char *log);

using namespace v8;

namespace exawallet {

void SetupLog(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();
    if (args.Length() == 0 || !args[0]->IsNumber()) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Log level arument must be 0 - 4 integer")));
        return;
    }

    const char* configureString = nullptr;
    auto level = Integer::Cast(*args[0]);
    switch (level->Value()) {
    case 0:
        configureString = "0";
        break;
    case 1:
        configureString = "1";
        break;
    case 2:
        configureString = "2";
        break;
    case 3:
        configureString = "3";
        break;
    case 4:
        configureString = "4";
        break;
    default:
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Log level arument must be 0 - 4 integer")));
        return;
    }

    if (args.Length() == 1) {
        mlog_configure("", true);
        mlog_set_log(configureString);
        return;
    }

    if (!args[1]->IsString()) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Filename argument is expected")));
        return;
    }

    auto filename = args[1]->ToString(isolate->GetCurrentContext()).ToLocalChecked();

    mlog_configure(*String::Utf8Value(isolate, filename), false);
    mlog_set_log(configureString);
}

void Init(Local<Object> exports) {
    Wallet::Init(exports->GetIsolate());
    PendingTransaction::Init(exports->GetIsolate());

    //Monero libwallet generates too much logs in stdout by default
    mlog_set_log("0");
    mlog_configure("", false);

    NODE_SET_METHOD(exports, "createWallet", Wallet::CreateWallet);
    NODE_SET_METHOD(exports, "walletExists", Wallet::WalletExists);
    NODE_SET_METHOD(exports, "openWallet", Wallet::OpenWallet);
    NODE_SET_METHOD(exports, "genPaymentId", Wallet::GenPaymentId);
    NODE_SET_METHOD(exports, "paymentIdValid", Wallet::PaymentIdValid);
    NODE_SET_METHOD(exports, "addressValid", Wallet::AddressValid);

    NODE_SET_METHOD(exports, "setupLog", SetupLog);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Init)

} //namespace exawallet
