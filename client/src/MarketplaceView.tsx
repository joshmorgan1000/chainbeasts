import { useEffect, useState } from 'react';
import { Listing, Lease, Marketplace } from './marketplace';

export default function MarketplaceView() {
  const [mp] = useState(() => new Marketplace());
  const [listings, setListings] = useState<Listing[]>([]);
  const [leases, setLeases] = useState<Lease[]>([]);
  const [tokenId, setTokenId] = useState('');
  const [price, setPrice] = useState('');
  const [leaseId, setLeaseId] = useState('');
  const [leasePrice, setLeasePrice] = useState('');
  const [duration, setDuration] = useState('');
  const [parentA, setParentA] = useState('');
  const [parentB, setParentB] = useState('');

  const refresh = async () => {
    setListings(await mp.getListings());
    setLeases(await mp.getLeases());
  };

  const list = async () => {
    await mp.list(Number(tokenId), Number(price));
    setTokenId('');
    setPrice('');
    await refresh();
  };

  const buy = async (id: number, p: number) => {
    await mp.buy(id, p);
    await refresh();
  };

  const listLease = async () => {
    await mp.listForLease(Number(leaseId), Number(leasePrice), Number(duration));
    setLeaseId('');
    setLeasePrice('');
    setDuration('');
    await refresh();
  };

  const rent = async (id: number, p: number) => {
    await mp.rent(id, p);
    await refresh();
  };

  const cancelLease = async (id: number) => {
    await mp.cancelLease(id);
    await refresh();
  };

  const breed = async () => {
    await mp.breed(Number(parentA), Number(parentB), new Uint8Array());
    setParentA('');
    setParentB('');
  };

  useEffect(() => {
    refresh();
    const id = setInterval(() => {
      refresh();
    }, 10000);
    return () => clearInterval(id);
  }, []);

  return (
    <div>
      <h2>Marketplace</h2>
      <button onClick={refresh}>Refresh</button>
      <div>
        <input
          placeholder="Token ID"
          value={tokenId}
          onChange={e => setTokenId(e.target.value)}
        />
        <input
          placeholder="Price"
          value={price}
          onChange={e => setPrice(e.target.value)}
        />
        <button onClick={list}>List</button>
      </div>
      <div>
        <input
          placeholder="Lease Token ID"
          value={leaseId}
          onChange={e => setLeaseId(e.target.value)}
        />
        <input
          placeholder="Lease Price"
          value={leasePrice}
          onChange={e => setLeasePrice(e.target.value)}
        />
        <input
          placeholder="Duration"
          value={duration}
          onChange={e => setDuration(e.target.value)}
        />
        <button onClick={listLease}>List Lease</button>
      </div>
      <ul>
        {listings.map(l => (
          <li key={l.id}>
            Token {l.id} – price {l.price} – seller {l.seller}
            <button onClick={() => buy(l.id, l.price)}>Buy</button>
          </li>
        ))}
      </ul>
      <ul>
        {leases.map(l => (
          <li key={l.id}>
            Lease {l.id} – price {l.price} – duration {l.duration} –
            {l.renter ? ` rented by ${l.renter}` : ' available'} – expiry {l.expiry}
            <button onClick={() => rent(l.id, l.price)}>Rent</button>
            <button onClick={() => cancelLease(l.id)}>Cancel</button>
          </li>
        ))}
      </ul>
      <div>
        <h3>Breed</h3>
        <input
          placeholder="Parent A"
          value={parentA}
          onChange={e => setParentA(e.target.value)}
        />
        <input
          placeholder="Parent B"
          value={parentB}
          onChange={e => setParentB(e.target.value)}
        />
        <button onClick={breed}>Breed</button>
      </div>
    </div>
  );
}
