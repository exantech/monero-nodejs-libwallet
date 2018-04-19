// addon.cc
#include <uv.h>

#include "wallet.h"
#include "pendingtransaction.h"

namespace exawallet {

void Init(v8::Local<v8::Object> exports) {
    Wallet::Init(exports->GetIsolate());
    PendingTransaction::Init(exports->GetIsolate());

    NODE_SET_METHOD(exports, "createWallet", Wallet::CreateWallet);
    NODE_SET_METHOD(exports, "walletExists", Wallet::WalletExists);
    NODE_SET_METHOD(exports, "openWallet", Wallet::OpenWallet);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Init)

} //namespace exawallet
