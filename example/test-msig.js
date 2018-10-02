const monero = require('monero-nodejs-libwallet')
const _ = require('underscore')
const fs = require('fs')
const globby = require('globby');
const util = require('util')

if(fs.existsSync('xmr.log')) 
	fs.unlinkSync('xmr.log')

monero.setupLog(3, "xmr.log");

var nwallets = 2
var recreate = false;
var args = {
	'path': 'stagenet',
	'password': '123', 
	'network': 'stagenet',
	'daemonAddress': 'monero-stage.exan.tech:38081',
	'restoreHeight': 146046
}

const openWallets = async (args0) => {
	wallets = []
	for(var walletIndex=0; walletIndex<nwallets; walletIndex++) {
		const args = Object.assign({}, args0, {path: args0.path+"-"+walletIndex});
		exists = monero.walletExists(args.path);
		if (!exists || recreate) {
			if(exists) {
				lst = await globby(args.path+"*")
				lst.forEach(fs.unlinkSync)
			}
			console.log("wallet "+args.path+" doesn't exist. creating new one: " + args.path);
			if(args.mnemonic)
				wallets.push(await monero.recoveryWallet(args))
			else
				wallets.push(await monero.createWallet(args))
		} else {
			console.log("wallet already exists. opening: " + args.path);
			wallets.push(await monero.openWallet(args))
		}
	}
	return wallets;
}

var lastTime = new Date();
const reportOnWallets = (wallets) => {
	for(var i=0; i<wallets.length; i++) {
		const wallet = wallets[i];
		console.log("subscribing wallet "+wallet.address())
		wallet.on('newBlock', function (height) {
			if((new Date()-lastTime) > 1000) {
				console.log("blockchain updated, height: " + height);
				lastTime = new Date()
			}
		});

		wallet.on('refreshed', function () {
			console.log(i+" "+wallet.address()+" wallet refreshed callback");
		});

		wallet.on('updated', function () {
			console.log(i+" "+wallet.address()+" updated");
		});

		wallet.on('unconfirmedMoneyReceived', function(tx, amount) {
			console.log(i+" "+wallet.address()+" unconfirmed money received. tx: " + tx + ", amount: " + amount);
		});

		wallet.on('moneyReceived', function(tx, amount) {
			console.log(i+" "+wallet.address()+" money received. tx: " + tx + ", amount: " + amount);
		});

		wallet.on('moneySpent', function(tx, amount) {
			console.log(i+" "+wallet.address()+" money spent. tx: " + tx + ", amount: " + amount);
		});
	}
}

const printWallets = (wallets) => {
	for(var i = 0; i < wallets.length; i++) {
		const wallet = wallets[i];
		console.log(i+" address: " + wallet.address());
		console.log(i+" balance: " + wallet.balance());
		console.log(i+" unlocked balance: " + wallet.unlockedBalance());
		console.log(i+" seed: " + wallet.seed());
		console.log(i+" secret view key: " + wallet.secretViewKey());
		console.log(i+" secret spend key: " + wallet.secretSpendKey());
		console.log(i+" public view key: " + wallet.publicViewKey());
		console.log(i+" public spend key: " + wallet.publicSpendKey());
    }
}

const printEach = (title, data) => {
	for(var i=0; i < data.length; i++) {
		console.log(i+" "+title+": "+data[i]);
	}
}

const sendToMultisig = async (args) => {
	const wallets = await openWallets(args)
	printWallets(wallets);
	reportOnWallets(wallets);
	if(args.recreate) {
		console.log("getMultisigInfo");
		const msigInfos = wallets.map(wallet => wallet.getMultisigInfo());
		printEach("multisig info", msigInfos);
		console.log("makeMultisig");
		const extraInfos = wallets.map(wallet => wallet.makeMultisig(msigInfos, wallets.length));
		printEach("extra infos", extraInfos);
		if(args.n_minus_1) {
			console.log("finalizeMultisig");
			wallets[0].finalizeMultisig(extraInfos);
		}
		console.log("we're multisig now")
		printWallets(wallets);
	}
	console.log("opening source wallet")
	const sourceWallet = await monero.openWallet(Object.assign({}, args, {path: "stagenet2"}));
	reportOnWallets([sourceWallet]);
	const onWallet = (wallet, what) => new Promise((resolve, reject) => {
		console.log("subscribing on "+what+" for wallet "+wallet.address())
		wallet.on(what,  () => {
			console.log("wallet event: "+what+" for wallet "+wallet.address())
			resolve(true)
		})})
	printWallets([sourceWallet])
	console.log("waiting until refreshed")
	const done = await onWallet(sourceWallet, "refreshed")
	console.log("refreshed");
	printWallets([sourceWallet])
	const destAddress = wallets[0].address();
	console.log("sending to multisig "+destAddress);
	const tx = await sourceWallet.createTransaction({address: destAddress, amount: "1"})
	await tx.commit()
	console.log("We got money into multisig")
	printWallets(wallets)
	console.log("Will send 'em back")
	const tx2 = await wallets[0].createTransaction({address: sourceWallet.address, amount: "1"})
	await tx2.commit()
	return {wallets:wallets, sourceWallet:sourceWallet};
}	

const main = async() => {
	const res = await sendToMultisig(args);
	console.log("done");
}
setInterval(()=>console.log("ping"), 60000)
main().then(()=>console.log("exiting"))
console.log("started")
