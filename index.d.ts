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

  type Wallet = {
    address(): string;
    seed(): string;
    on(event: 'newBlock', callback: (height: number) => void): Wallet;
    on(event: 'unconfirmedMoneyReceived' | 'moneyReceived' | 'moneySpent', callback: (tx: string, amount: number) => void): Wallet;
    on(event: 'refreshed' | 'updated', callback: () => void): Wallet;
    off(event?: WalletEvent): Wallet;
    store(): Promise<void>;
    createTransaction(options: {
      address: string,
      amount: string,
      paymentId?: string,
      mixin?: number,
    }): Promise<Transaction>;
    path(): string;
    network(): Network;
    secretViewKey(): string;
    publicViewKey(): string;
    secretSpendKey(): string;
    publicSpendKey(): string;
    publicMultisigSignerKey(): string;
    setPassword(password: string): Wallet;
    setRefreshFromBlockHeight(height: number): Wallet;
    getRefreshFromBlockHeight(): string;
    connected(): boolean;
    setTrustedDaemon(value: boolean): Wallet;
    trustedDaemon(): boolean;
    balance(): string;
    unlockedBalance(): string;
    blockchainHeight(): number;
    daemonBlockchainHeight(): number;
    synchronized(): boolean;
    pauseRefresh(): void;
    startRefresh(): void;
    multisigState(): MultisigState;
    signMessage(message: string): string; // TODO non-multisig only
    verifySignedMessage(message: string, address: string, signature: string): boolean; // TODO non-multisig only
    signMultisigParticipant(message: string): string; // TODO multisig only
    verifyMessageWithPublicKey(message: string, key: string, signature: string): boolean;
    getMultisigInfo(): string;
    makeMultisig(multisigInfos: string[], walletsNumber: number): string;
    finalizeMultisig(multisigInfos: string[]): string;
    exportMultisigImages(): string;
    importMultisigImages(keys: string[]): void;
    restoreMultisigTransaction(sign: string): Promise<Transaction>;
  };

  function setupLog(level: number, output: string): void;
  function createWallet(options: {
    path: string,
    password: string,
    network?: Network,
    daemonAddress: string,
    language: Language,
  }): Promise<Wallet>;
  function openWallet(options: {
    path: string,
    password: string,
    network?: Network,
    daemonAddress: string,
    language: Language,
  }): Promise<Wallet>;
  function walletExists(path: string): boolean;
  function genPaymentId(): string;
  function paymentIdValid(paymentId: string): boolean;
  function addressValid(address: string): boolean;
}
