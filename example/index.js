const monero = require('monero-nodejs-libwallet');

monero.setupLog(1);

var wallet;
var sent = false;

var path = 'test-wallet'
var args = {
	'path': path,
	'password': '123', 
	'network': 'mainnet',
	'daemonAddress': 'monero.exan.tech:18081'
	,'restoreHeight': 1608000
	,'mnemonic' : 'nifty inflamed against focus gasp ethics spying gulp tiger cogs evicted cohesive woken nylon erosion tell saved fatal alkaline acquire lemon maps hull imitate saved'
}

if (!monero.walletExists(path)) {
	console.log("wallet doesn't exist. creating new one: " + path);
	if(args.mnemonic)
		promise = monero.recoveryWallet(args)
	else
		promise = monero.createWallet(args);
} else {
	console.log("wallet already exists. opening: " + path);
	promise = monero.openWallet(args);
}

const nextTick = () => {
    if (wallet) {
    	console.log("address: " + wallet.address());
    	console.log("balance: " + wallet.balance());
    	console.log("unlocked balance: " + wallet.unlockedBalance());
		console.log("seed: " + wallet.seed());
		console.log("secret view key: " + wallet.secretViewKey());
		console.log("secret spend key: " + wallet.secretSpendKey());
		console.log("public view key: " + wallet.publicViewKey());
		console.log("public spend key: " + wallet.publicSpendKey());
    }

    setTimeout(nextTick, 10000);
}

promise
	.then((w) => {
		console.log("address: " + w.address());
		console.log("seed: " + w.seed());

		wallet = w;
		wallet.on('newBlock', function (height) {
			console.log("blockchain updated, height: " + height);
		});

		wallet.on('refreshed', function () {
			console.log("wallet refreshed");

			wallet.store()
				.then(() => {console.log("wallet stored")})
				.catch((e) => {console.log("unable to store wallet: " + e)})

			if (!sent) {
				sent = true;
				wallet.createTransaction({
					'address': '44zrUGhyRHYbHYrfiGAtLdJMHfe5DtoFTBeVPCE6MGKzZA2bJ4tCJFuhYk3Wjp3YxEWoQU8So5xUiiArgnkBHZgX8Fyhv6e',
					'amount': '2000000000', //monero atomic units as string
				}).then((tx) => {
					console.log("transaction created: " + tx.transactionsIds());
					
					tx.commit().then(() => {
						console.log("transaction commited successfully");
					}).catch((e) => {
						console.log("error on commiting transaction: " + e);
					});
				}).catch((e) => {
					sent = false;
					console.log("couldn't create transaction: " + e);
				});
			}
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

		nextTick();
	})
	.catch((e) => {
		console.log("no luck tonight: " + e);
	});
