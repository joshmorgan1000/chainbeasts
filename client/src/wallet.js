export async function connect() {
    if (typeof window !== 'undefined' && window.ethereum && window.ethereum.request) {
        await window.ethereum.request({ method: 'eth_requestAccounts' });
        console.log('Wallet connected');
        return;
    }
    console.log('Wallet provider not found');
}

export async function sendTransaction(to, data, value = '0x0') {
    if (!window.ethereum || !window.ethereum.request) {
        throw new Error('Wallet provider not found');
    }
    const [from] = await window.ethereum.request({ method: 'eth_requestAccounts' });
    return window.ethereum.request({
        method: 'eth_sendTransaction',
        params: [{ from, to, data, value }],
    });
}
