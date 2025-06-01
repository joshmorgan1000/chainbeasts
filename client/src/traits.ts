import { connect, sendTransaction } from './wallet';

const CREATURE_NFT_ADDR =
  (window as any).CREATURE_NFT_ADDR || '0x0000000000000000000000000000000000000000';
const LOCK_SELECTOR = '0x16260880'; // lockTraits(uint256,uint256,bytes32)

function encUint(v: number | bigint): string {
  return BigInt(v).toString(16).padStart(64, '0');
}

function pad32(hex: string): string {
  return hex.replace(/^0x/, '').padStart(64, '0');
}

export async function lockTraits(
  tokenId: number,
  traitsHex: string,
  nameHash: string,
): Promise<void> {
  await connect();
  const data =
    LOCK_SELECTOR + encUint(tokenId) + pad32(traitsHex) + pad32(nameHash);
  await sendTransaction(CREATURE_NFT_ADDR, '0x' + data);
}
