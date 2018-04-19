#include "asynccallback.h"

namespace exawallet {

void Wallet::newBlock(uint64_t height) {
    AsyncCallback<uint64_t>* async = constructAsyncCallback<uint64_t>(this, "newBlock");
    async->param = height;
    async->Enqueue();
}

void Wallet::refreshed() {
    AsyncCallback<void>* async = constructAsyncCallback<void>(this, "refreshed");
    async->Enqueue();
}

void Wallet::moneySpent(const std::string& tx, uint64_t amount) {
    AsyncCallback<TransactionAmount>* async = constructAsyncCallback<TransactionAmount>(this, "moneySpent");
    async->param = TransactionAmount{tx, std::to_string(amount)};
    async->Enqueue();
}

void Wallet::moneyReceived(const std::string& tx, uint64_t amount) {
    AsyncCallback<TransactionAmount>* async = constructAsyncCallback<TransactionAmount>(this, "moneyReceived");
    async->param = TransactionAmount{tx, std::to_string(amount)};
    async->Enqueue();
}

void Wallet::unconfirmedMoneyReceived(const std::string& tx, uint64_t amount) {
    AsyncCallback<TransactionAmount>* async = constructAsyncCallback<TransactionAmount>(this, "unconfirmedMoneyReceived");
    async->param = TransactionAmount{tx, std::to_string(amount)};
    async->Enqueue();
}

void Wallet::updated() {
    AsyncCallback<void>* async = constructAsyncCallback<void>(this, "updated");
    async->Enqueue();
}

} //namespace exawallet
