import assert from 'assert';
import { connect, sendTransaction } from '../src/wallet.js';

async function testConnect() {
  let called = false;
  global.window = {
    ethereum: {
      request: async (opts) => {
        if (opts.method === 'eth_requestAccounts') {
          called = true;
          return ['0xabc'];
        }
        throw new Error('unexpected method');
      },
    },
  };
  await connect();
  assert.ok(called, 'connect should call wallet request');
}

async function testSendTransactionSuccess() {
  const requests = [];
  global.window = {
    ethereum: {
      request: async (opts) => {
        requests.push(opts);
        if (opts.method === 'eth_requestAccounts') {
          return ['0x123'];
        }
        if (opts.method === 'eth_sendTransaction') {
          return 'txhash';
        }
        throw new Error('unexpected method');
      },
    },
  };
  const tx = await sendTransaction('0xfeed', '0xdead');
  assert.equal(tx, 'txhash');
  assert.equal(requests.length, 2);
  assert.equal(requests[0].method, 'eth_requestAccounts');
  assert.equal(requests[1].method, 'eth_sendTransaction');
}

async function testSendTransactionError() {
  global.window = {};
  let threw = false;
  try {
    await sendTransaction('0x0', '0x');
  } catch (err) {
    threw = true;
  }
  assert.ok(threw, 'sendTransaction should throw without provider');
}

(async () => {
  await testConnect();
  await testSendTransactionSuccess();
  await testSendTransactionError();
  console.log('wallet tests passed');
})();
