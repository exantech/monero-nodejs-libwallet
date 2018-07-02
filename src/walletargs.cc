#include "walletargs.h"

#include <stdexcept>

#include <node.h>

using namespace v8;

namespace exawallet {

const uint32_t CreateTransactionArgs::MINIMAL_MIXIN = 7;

template<typename T>
T convertValue(Isolate* isolate, Local<Value> value);

bool hasProperty(Local<Context> context, Local<Object> obj, Local<String> name) {
    auto maybe = obj->HasOwnProperty(context, name);
    bool has = false;
    maybe.To(&has);
    return has;
}

template<typename T>
bool getRequiredProperty(Isolate* isolate, Local<Object> obj, const char* name, T& value) {
    Local<Context> context = isolate->GetCurrentContext();

    auto key = String::NewFromUtf8(isolate, name);
    bool has = hasProperty(isolate->GetCurrentContext(), obj, key);
    if (!has){
        return false;
    }

    auto valueLocal = obj->Get(context, key).ToLocalChecked();
    value = convertValue<T>(isolate, valueLocal);
    return true;
}

template<typename T>
T getOptionalProperty(Isolate* isolate, Local<Object> obj, const char* name, const T& defaultValue) {
    Local<Context> context = isolate->GetCurrentContext();

    auto key = String::NewFromUtf8(isolate, name);
    bool has = hasProperty(isolate->GetCurrentContext(), obj, key);
    if (!has){
        return defaultValue;
    }

    auto valueLocal = obj->Get(context, key).ToLocalChecked();
    return convertValue<T>(isolate, valueLocal);
}

template<>
std::string convertValue<std::string>(Isolate* isolate, Local<Value> value) {
    return *String::Utf8Value(value);
}

template<>
double convertValue<double>(Isolate* isolate, Local<Value> value) {
    return value->ToNumber(isolate->GetCurrentContext()).ToLocalChecked()->Value();
}

template<>
uint32_t convertValue<uint32_t>(Isolate* isolate, Local<Value> value) {
    return value->ToUint32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
}

std::string CreateWalletArgs::Init(const FunctionCallbackInfo<Value>& args) {
    if (args.Length() != 1 || !args[0]->IsObject()) {
        return "Argument must be an object";
    }

    auto isolate = args.GetIsolate();
    auto obj = args[0]->ToObject(isolate);
    if (!getRequiredProperty<std::string>(isolate, obj, "path", path)) {
        return std::string("Required property not found: path");
    }

    if (!getRequiredProperty<std::string>(isolate, obj, "password", password)) {
        return std::string("Required property not found: password");
    }

    if (!getRequiredProperty<std::string>(isolate, obj, "daemonAddress", daemonAddress)) {
        return std::string("Required property not found: daemonAddress");
    }

    language = getOptionalProperty<std::string>(isolate, obj, "language", "English");
    auto net = getOptionalProperty<std::string>(isolate, obj, "network", "mainnet");
    if (net == "mainnet") {
        nettype = Monero::MAINNET;
    } else if (net == "testnet") {
        nettype = Monero::TESTNET;
    } else if (net == "stagenet") {
        nettype = Monero::STAGENET;
    } else {
        return "Invalid value for network: " + net;
    }

    return {};
}

std::string OpenWalletArgs::Init(const FunctionCallbackInfo<Value>& args) {
    if (args.Length() != 1 || !args[0]->IsObject()) {
        return "Argument must be an object";
    }

    auto isolate = args.GetIsolate();
    auto obj = args[0]->ToObject(isolate);
    if (!getRequiredProperty<std::string>(isolate, obj, "path", path)) {
        return std::string("Required property not found: path");
    }
    if (!getRequiredProperty<std::string>(isolate, obj, "password", password)) {
        return std::string("Required property not found: password");
    }
    if (!getRequiredProperty<std::string>(isolate, obj, "daemonAddress", daemonAddress)) {
        return std::string("Required property not found: daemonAddress");
    }

    auto net = getOptionalProperty<std::string>(isolate, obj, "network", "mainnet");
    if (net == "mainnet") {
        nettype = Monero::MAINNET;
    } else if (net == "testnet") {
        nettype = Monero::TESTNET;
    } else if (net == "stagenet") {
        nettype = Monero::STAGENET;
    } else {
        return "Invalid value for network: " + net;
    }

    return {};
}


std::string RecoveryWalletArgs::Init(const FunctionCallbackInfo<Value>& args) {
    if (args.Length() != 1 || !args[0]->IsObject()) {
        return "Argument must be an object";
    }

    auto isolate = args.GetIsolate();
    auto obj = args[0]->ToObject(isolate);
    if (!getRequiredProperty<std::string>(isolate, obj, "path", path)) {
        return std::string("Required property not found: path");
    }
    if (!getRequiredProperty<std::string>(isolate, obj, "password", password)) {
        return std::string("Required property not found: password");
    }
    if (!getRequiredProperty<std::string>(isolate, obj, "daemonAddress", daemonAddress)) {
        return std::string("Required property not found: daemonAddress");
    }

    auto net = getOptionalProperty<std::string>(isolate, obj, "network", "mainnet");
    if (net == "mainnet") {
        nettype = Monero::MAINNET;
    } else if (net == "testnet") {
        nettype = Monero::TESTNET;
    } else if (net == "stagenet") {
        nettype = Monero::STAGENET;
    } else {
        return "Invalid value for network: " + net;
    }

    restoreHeight = getOptionalProperty<uint64_t>(isolate, obj, "restoreHeight", 0);

    if(!getRequiredProperty<std::string>(isolate, obj, "mnemonic", mnemonic)) {
        return std::string("Required property not found: mnemonic");
    }

    return {};
}


std::string CreateTransactionArgs::Init(const FunctionCallbackInfo<Value>& args) {
    if (args.Length() != 1 || !args[0]->IsObject()) {
        return "Argument must be an object";
    }

    auto isolate = args.GetIsolate();
    auto obj = args[0]->ToObject(isolate);

    if (!getRequiredProperty<std::string>(isolate, obj, "address", address)) {
        return std::string("Required property not found: address");
    }

    std::string amountStr;
    if (!getRequiredProperty<std::string>(isolate, obj, "amount", amountStr)) {
        return std::string("Required property not found: amount");
    }
    amount = std::stoul(amountStr);
    paymentId = getOptionalProperty<std::string>(isolate, obj, "paymentId", "");
    mixin = getOptionalProperty<uint32_t>(isolate, obj, "mixin", MINIMAL_MIXIN);

    if (mixin < MINIMAL_MIXIN) {
        return "Minimal mixin: " + std::to_string(MINIMAL_MIXIN);
    }

    return {};
}

}
