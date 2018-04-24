# monero-nodejs-libwallet
Wrapper for monero libwallet. All created wallet files are compatible with `monero-wallet-cli` and `monero-wallet-rpc`. 
Check `example.js` for usage examples of the library. 

# TODO
* prebuilt binaries for desktop platforms.

# API
## Module Functions
### createWallet
Create new wallet asynchronously:
```js
const monero = require('./build/Debug/monero');
monero.createWallet({
	'path': 'test-wallet',
	'password': '123', 
	'network': 'mainnet',
	'daemon_address': 'localhost:18081',
}).then((wallet) => console.log('New wallet succesfully created: ' + wallet.address()))
   .catch((e) => console.log('Failed to create new wallet: ' + e));
```
Arguments object:

| property | description | required |
|----------|----------|----------|
| path | path to wallet file| yes|
| password | password | yes |
| network | one of mainnet (default) / stagenet / testnet| no|
| daemon_address| remote node address| yes|
| language| language to generate mnemonics for ('English' is default)| no|

Returns promise object. Throws in case of arguments error.

### openWallet
Opens existing wallet asynchronously:
```js
const monero = require('./build/Debug/monero');
monero.openWallet({
	'path': 'test-wallet',
	'password': '123', 
	'network': 'mainnet',
	'daemon_address': 'localhost:18081',
}).then((wallet) => console.log('New wallet succesfully created: ' + wallet.address()))
   .catch((e) => console.log('Failed to create new wallet: ' + e));
```

Arguments object:

| property | description | required |
|----------|----------|----------|
| path | path to wallet file| yes|
| password | password | yes |
| network | one of mainnet (default) / stagenet / testnet| no|
| daemon_address| remote node address| yes|
| language| language to generate mnemonics for ('English' is default)| yes|

Returns promise object. Throws in case of arguments error.

### walletExists
Checks if wallet exists:
```js
const monero = require('./build/Debug/monero');
if (monero.walletExists('test-wallet')) {
    console.log('Wallet already exists');
} else {
    console.log('Wallet does not exist');
}
```

Returns boolean.

### genPaymentId
Generates new payment id:
```js
const monero = require('./build/Debug/monero');
console.log('New payment id: ' + monero.genPaymentId());
```

### paymentIdValid
Checks if given payment id is valid:
```js
const monero = require('./build/Debug/monero');
if (monero.paymentIdValid('180b67533011df75b74333e62599c160f5484bf8bb98779598520dfb90633198')) {
    console.log('Payment id is valid');
} else {
    console.log('Payment id is not valid');
}
```

### addressValid
Checks if given monero address valid in certain network type:
```js
const monero = require('./build/Debug/monero');
if (monero.addressValid('44zrUGhyRHYbHYrfiGAtLdJMHfe5DtoFTBeVPCE6MGKzZA2bJ4tCJFuhYk3Wjp3YxEWoQU8So5xUiiArgnkBHZgX8Fyhv6e', 'mainnet')) {
    console.log('Address is valid');
} else {
    console.log('Address is not valid');
}
```

## Wallet Functions
### address
Returns an address of the wallet:
```js
console.log('Wallet address: ' + wallet.address());
```

### seed
Returns mnemonic seed for the wallet:
```js
console.log('Wallet seed: ' + wallet.seed());
```

### on
Subscibes on wallet emitted events:
```js
wallet.on('newBlock', function (height) {
	console.log("blockchain updated, height: " + height);
});
wallet.on('refreshed', function () {
	console.log("wallet is synchronized");
});
wallet.on('updated', function () {
	console.log("updated");
});
wallet.on('unconfirmedMoneyReceived', function(tx, amount) {
	console.log("unconfirmed money received. tx: " + tx + ", amount: " + amount);
});
wallet.on('moneyReceived', function(tx, amount) {
	console.log("money received. tx: " + tx + ", amount: " + amount);
});
wallet.on('moneySpent', function(tx, amount) {
	console.log("money spent. tx: " + tx + ", amount: " + amount);
});
```

### store
Asynchronously saves wallet into the file:
```js
wallet.store()
	.then(() => {console.log("wallet stored")})
	.catch((e) => {console.log("unable to store wallet: " + e)});
```

### createTransaction
Creates pending transaction:
```js
wallet.createTransaction({
	'address': '44zrUGhyRHYbHYrfiGAtLdJMHfe5DtoFTBeVPCE6MGKzZA2bJ4tCJFuhYk3Wjp3YxEWoQU8So5xUiiArgnkBHZgX8Fyhv6e',
	'amount': '2000000000', //monero atomic units as string
}).then((tx) => {
	console.log("transaction created: " + tx.transactionsIds());
}).catch((e) => {
	sent = false;
	console.log("couldn't create transaction: " + e);
});
```
Accepts arguments object:

| property | description | required |
|-----|-----|-----|
| address | valid monero address | yes |
| amount | monero atomic units as string | yes |
| payment_id | valid payment id | no |
| mixin | ring signature size (current minimum is `7`) | no |

Throws in case of arguments error. 
Returns promise. `then`-callback accepts `PendingTransaction` object, `catch`-callback accepts error string.


### path
Returns current wallet file path:
```js
console.log("wallet located at : " + wallet.path());
```

### network
Returns current network type (either `mainnet`, `stagenet` or `testnet`):
```js
console.log("network : " + wallet.network());
```

### secretViewKey, publicViewKey, secretSpendKey, publicSpendKey, publicMultisigSignerKey
Returns corresponding hex-encoded wallet key:
```js
console.log("secret view key : " + wallet.secretViewKey());
console.log("public view key : " + wallet.publicViewKey());
console.log("secret spend key : " + wallet.secretSpendKey());
console.log("public spend key : " + wallet.publicSpendKey());
console.log("public spend key : " + wallet.publicMultisigSignerKey());
```

### setPassword
Sets new password to the wallet:
```js
wallet.setPassword('iamsherlocked');
```

### setRefreshFromBlockHeight
Sets the blockchain height to start refresh from:
```js
wallet.setRefreshFromBlockHeight(1555371);
```

### getRefreshFromBlockHeight
```js
console.log('refreshing from: ' + wallet.getRefreshFromBlockHeight());
```

### connected
Checks if wallet is connected to the daemon:
```js
console.log('connected: ' + wallet.connected());
```

### setTrustedDaemon, trustedDaemon
Sets `trusted` flag to the daemon:
```js
if (wallet.trustedDamon()) {
    wallet.setTrustedDaemon(false); // trust no one
}
```

### balance, unlockedBalance
Returns wallet balance:
```js
console.log('wallet: ' + wallet.address());
console.log('balance: ' + wallet.balance());
console.log('unlocked balance: ' + wallet.unlockedBalance());
```

### blockchainHeight
Returns local blockchain height:
```js
console.log('synced till: ' + wallet.blockchainHeight());
```

### daemonBlockchainHeight
Returns remote node's blockhain height:
```js
console.log('synchronized till: ' + wallet.daemonBlockchainHeight());
```

### synchronized
Checks if wallet is synchronized with a network:
```js
console.log('wallet synchronized: ' + wallet.synchronized());
```

### pauseRefresh
Stops synchronization.

### startRefresh
Resumes synchronization.

### multisigState
Returns an object representing multisig wallet state:
```js
var state = wallet.multisigState();
console.log('is multisig: ' + state.isMultisig);
console.log('is multisig ready: ' + state.isReady); //indicates multisig wallet creation process is finished
console.log('multisig threshold: ' + state.threshold); //number of required signatures
console.log('multisig total: ' + state.total); //number of participants
```

### getMultisigInfo
Returns serialized to string multisig info data necessary for multisig wallet creation (see example below).

### makeMultisig
Accepts multisig infos from all participants and the threshold to set wallet into multisig state (see example below). 
In case of N / N wallets it's the last step in wallet creation.
In case of N - 1 / N wallets returns extra multisig info which must be passed to another participants.

### finalizeMultisig
Accepts extra multisig info (result of `makeMultisig` invocation) from all participants and finalizes N - 1 / N multisig wallet creation.

3 / 3 multisig wallet creation example: 
```js
var msigInfo1 = wallet1.getMultisigInfo();
var msigInfo2 = wallet2.getMultisigInfo();
var msigInfo3 = wallet3.getMultisigInfo();

wallet1.makeMultisig([msigInfo2, msigInfo3], 3);
wallet2.makeMultisig([msigInfo1, msigInfo3], 3);
wallet3.makeMultisig([msigInfo1, msigInfo2], 3);

console.log('multisig wallet created');
console.log('wallet1 address: ' + wallet1.address());
console.log('wallet2 address: ' + wallet2.address());
console.log('wallet3 address: ' + wallet3.address());
```

2 / 3 multisig wallet creation example: 
```js
var msigInfo1 = wallet1.getMultisigInfo();
var msigInfo2 = wallet2.getMultisigInfo();
var msigInfo3 = wallet3.getMultisigInfo();

var emsigInfo1 = wallet1.makeMultisig([msigInfo2, msigInfo3], 2);
var emsigInfo2 = wallet2.makeMultisig([msigInfo1, msigInfo3], 2);
var emsigInfo3 = wallet3.makeMultisig([msigInfo1, msigInfo2], 2);

wallet1.finalizeMultisig([emsigInfo2, emsigInfo3]);
wallet2.finalizeMultisig([emsigInfo1, emsigInfo3]);
wallet3.finalizeMultisig([emsigInfo1, emsigInfo2]);

console.log('multisig wallet created');
console.log('wallet1 address: ' + wallet1.address());
console.log('wallet2 address: ' + wallet2.address());
console.log('wallet3 address: ' + wallet3.address());
```

### exportMultisigImages
Returns string of exported partial key images (see example below).

### importMultisigImages
Imports exported partial key images from other participants: 
```js
var ki1 = msigWallet1.exportMultisigImages();
var ki2 = msigWallet2.exportMultisigImages();
var ki3 = msigWallet3.exportMultisigImages();

msigWallet1.importMultisigImages([ki2, ki3]);
msigWallet2.importMultisigImages([ki1, ki3]);
msigWallet3.importMultisigImages([ki1, ki2]);
```

### restoreMultisigTransaction
Deserializes multisig transaction from other participant. Returns promise object.

## Pending Transaction's Functions
### commit
Asynchronously sends transaction into the network:
```js
tx.commit().then(() => {
	console.log("transaction commited successfully");
}).catch((e) => {
	console.log("error on commiting transaction: " + e);
});
```

### amount
Returns amount of the transaction:
```js
console.log("transaction amount: " + tx.amount());
```

### fee
Returns transaction fee:
```js
console.log("transaction fee: " + tx.fee());
```

### transactionsCount
If generated transaction is too big libwallet splits it into several transactions. This method returns number of generated low-level transactions:
```js
console.log("generated " + tx.transactionsCount() + ' low level transactions');
```

### transactionsIds
Returns transactions ids of newly generated pending transaction:
```js
if (tx.transactionsCount() > 1) {
    for (i = 0; i < tx.transactionsCount(); i++) {
        console.log('transaction #' + i + ': ' + tx.transactionsIds()[i]);
    }
} else {
    console.log('transaction id' + tx.transactionsIds());
}
```

### multisigSignData
Returns serialized multisig transaction which has to be passed to another participants for signature (see example below).

### signMultisigTransaction
Signs multisig transaction:
```js
wallet1.createTransaction({
	'address': '44zrUGhyRHYbHYrfiGAtLdJMHfe5DtoFTBeVPCE6MGKzZA2bJ4tCJFuhYk3Wjp3YxEWoQU8So5xUiiArgnkBHZgX8Fyhv6e',
	'amount': '2000000000',
}).then((tx) => {
	var signData = tx.multisigSignData();

	wallet2.restoreMultisigTransaction(signData).then((tx) => {
		console.log('multisig transaction restored successfully. Signing...');
		tx.signMultisigTransaction();
	}).catch((e) => {
		console.log("couldn't restore multisig transaction: " + e);
	});
}).catch((e) => {
	console.log("couldn't create transaction: " + e);
});
```

### signersKeys
Returns an array of public signers key of participants who already signed this transaction:
```js
console.log('participants already signed this transaction: ' + tx.signersKeys());
```