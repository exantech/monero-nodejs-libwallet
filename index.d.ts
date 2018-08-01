export = monero;
export as namespace monero;

declare namespace monero {
  type Language =
    | 'English'
    | 'Nederlands'
    | 'Français'
    | 'Español'
    | 'Português'
    | '日本語'
    | 'Italiano'
    | 'Deutsch'
    | 'русский язык'
    | '简体中文 (中国)'
    | 'Esperanto'
    | 'Lojban';

  type WalletEvent =
    | 'newBlock'
    | 'refreshed'
    | 'updated'
    | 'unconfirmedMoneyReceived'
    | 'moneyReceived'
    | 'moneySpent';

  type Network =
    | 'mainnet'
    | 'stagenet'
    | 'testnet';

  type MultisigState = {
    isMultisig: boolean,
    isReady: boolean,
    threshold: number,
    total: number,
  };

  type Transaction = {
    commit(): Promise<void>;
    amount(): string;
    fee(): string;
    transactionsCount(): number;
    transactionsIds(): string[];
    multisigSignData(): string;
    signMultisigTransaction(): void;
    signersKeys(): string[];
  };

  type Transfer = {
    amount: string;
    address: string;
  };

  type TransactionInfo = {
    id: string;
    direction: 'in' | 'out';
    pending: boolean;
    failed: boolean;
    amount: string;
    fee: string;
    blockHeight: number;
    subAddresses: string;
    subAddrAccount: string;
    label: string;
    confirmations: number;
    unlockTime: number;
    timestamp: number;
    paymentId: number;
    transfers: Transfer[];
  };

  interface Wallet {
    address(): string;
    seed(): string;
    on(event: 'newBlock', callback: (height: number) => void): PersonalWallet & MultisigWallet;
    on(
      event: 'unconfirmedMoneyReceived' | 'moneyReceived' | 'moneySpent',
      callback: (tx: string, amount: string) => void,
    ): PersonalWallet & MultisigWallet;
    on(event: 'refreshed' | 'updated', callback: () => void): PersonalWallet & MultisigWallet;
    off(event?: WalletEvent): PersonalWallet & MultisigWallet;
    store(): Promise<void>;
    createTransaction(options: {
      address: string,
      amount: string,
      paymentId?: string,
      mixin?: number,
    }): Promise<Transaction>;
    history(): TransactionInfo[];
    path(): string;
    network(): Network;
    secretViewKey(): string;
    publicViewKey(): string;
    secretSpendKey(): string;
    publicSpendKey(): string;
    publicMultisigSignerKey(): string;
    setPassword(password: string): PersonalWallet & MultisigWallet;
    setRefreshFromBlockHeight(height: number): PersonalWallet & MultisigWallet;
    getRefreshFromBlockHeight(): string;
    connected(): boolean;
    setTrustedDaemon(value: boolean): PersonalWallet & MultisigWallet;
    trustedDaemon(): boolean;
    balance(): string;
    unlockedBalance(): string;
    blockchainHeight(): number;
    daemonBlockchainHeight(): number;
    synchronized(): boolean;
    pauseRefresh(): void;
    startRefresh(): void;
    multisigState(): MultisigState;
    verifyMessageWithPublicKey(message: string, key: string, signature: string): boolean;
    getMultisigInfo(): string;
    makeMultisig(multisigInfos: string[], walletsNumber: number): string;
    finalizeMultisig(multisigInfos: string[]): string;
    exportMultisigImages(): string;
    importMultisigImages(keys: string[]): void;
    restoreMultisigTransaction(sign: string): Promise<Transaction>;
  }

  interface MultisigWallet extends Wallet {
    signMultisigParticipant(message: string): string;
  }

  interface PersonalWallet extends Wallet {
    signMessage(message: string): string;
    verifySignedMessage(message: string, address: string, signature: string): boolean;
  }

  function setupLog(level: number, output?: string): void;
  function createWallet(options: {
    path: string,
    password: string,
    network?: Network,
    daemonAddress: string,
    language?: Language,
  }): Promise<PersonalWallet>;
  function openWallet(options: {
    path: string,
    password: string,
    network?: Network,
    daemonAddress: string,
    language?: Language,
  }): Promise<PersonalWallet & MultisigWallet>;
  function recoveryWallet(options: {
    path: string,
    password: string,
    network?: Network,
    daemonAddress: string,
    mnemonic: string,
    restoreHeight?: number,
  }): Promise<PersonalWallet>;
  function walletExists(path: string): boolean;
  function genPaymentId(): string;
  function paymentIdValid(paymentId: string): boolean;
  function addressValid(address: string): boolean;
}
