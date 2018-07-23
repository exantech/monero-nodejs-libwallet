const t = require('tap');
const { createWallet, recoveryWallet, setupLog } = require('../index');
const path = require('path');
const fs = require('fs');
const rimraf = require('rimraf');

const WALLET_PATH = path.join(__dirname, 'test');

const walletOptions = {
  password: 'secret',
  daemonAddress: 'monero1.exan.tech:18081',
  path: WALLET_PATH,
};

const deleteWallet = () => rimraf.sync(WALLET_PATH);
const deleteKeys = () => rimraf.sync(`${WALLET_PATH}.keys`);

let mnemonic;

// setupLog(1);

deleteWallet();
deleteKeys();

createWallet(walletOptions).then(wallet => {
  return t.test('Wallet should be created if not exists on path', t => {
    mnemonic = wallet.seed();
    t.ok(fs.existsSync(WALLET_PATH));
    t.end();
  });
}).then(() => {
  return t.rejects(createWallet(walletOptions), 'Wallet shouldn\'t be created if it already exists')
}).then(() => {
  deleteWallet();
  deleteKeys();
  return recoveryWallet({
    ...walletOptions,
    mnemonic,
  }).then(wallet => {
    return t.test('Recover wallet works', t => {
      t.ok(fs.existsSync(WALLET_PATH));
      t.end();
    });
  });
}).then(() => {
  deleteWallet();
  deleteKeys();
}).catch(() => {
  deleteWallet();
  deleteKeys();
  t.threw();
});
