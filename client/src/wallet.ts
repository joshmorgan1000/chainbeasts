export async function connect(): Promise<void> {
  const eth = (window as any).ethereum;
  if (typeof window !== 'undefined' && eth?.request) {
    await eth.request({ method: 'eth_requestAccounts' });
    console.log('Wallet connected');
  } else {
    console.log('Wallet provider not found');
    throw new Error('Wallet provider not found');
  }
}

export async function sendTransaction(
  to: string,
  data: string,
  value: string = '0x0'
): Promise<string> {
  const eth = (window as any).ethereum;
  if (!eth?.request) {
    throw new Error('Wallet provider not found');
  }
  const [from] = await eth.request({ method: 'eth_requestAccounts' });
  return eth.request({
    method: 'eth_sendTransaction',
    params: [{ from, to, data, value }],
  });
}
