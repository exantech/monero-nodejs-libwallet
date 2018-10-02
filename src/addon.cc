#include <nan.h>
#include "wallet.h"
#include "pendingtransaction.h"

extern void mlog_configure(const std::string &filename_base, bool console, const std::size_t max_log_file_size = 104850000);
extern void mlog_set_log(const char *log);


namespace exawallet {

NAN_METHOD(SetupLog) {
    if (info.Length() == 0 || !info[0]->IsNumber()) {
        Nan::ThrowError("Log level arument must be 0 - 4 integer");
        return;
    }

    const char* configureString = nullptr;
    auto level = v8::Integer::Cast(*info[0]);
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
        Nan::ThrowError("Log level arument must be 0 - 4 integer");
        return;
    }

    if (info.Length() == 1) {
        mlog_configure("", true);
        mlog_set_log(configureString);
        return;
    }

    if (!info[1]->IsString()) {
        Nan::ThrowError("Filename argument is expected");
        return;
    }

    Nan::Utf8String filename(info[1]->ToString());
    mlog_configure(*filename, false);
    mlog_set_log(configureString);
}



NAN_MODULE_INIT(Init) {
    Wallet::Init(target);
    PendingTransaction::Init(target);


    //Monero libwallet generates too much logs in stdout by default
    mlog_set_log("0");
    mlog_configure("", false);

    Nan::SetMethod(target, "createWallet", Wallet::CreateWallet);
    Nan::SetMethod(target, "walletExists", Wallet::WalletExists);
    Nan::SetMethod(target, "openWallet", Wallet::OpenWallet);
    Nan::SetMethod(target, "recoveryWallet", Wallet::RecoveryWallet);
    Nan::SetMethod(target, "genPaymentId", Wallet::GenPaymentId);
    Nan::SetMethod(target, "paymentIdValid", Wallet::PaymentIdValid);
    Nan::SetMethod(target, "addressValid", Wallet::AddressValid);
    Nan::SetMethod(target, "setupLog", SetupLog);
}


NODE_MODULE(NODE_GYP_MODULE_NAME, Init)

} //namespace exawallet
