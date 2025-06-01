# Visual Customisation & Naming – Draft v0.1

---

## 1  Overview

Players may pay a small \$CORE fee to lock **visual traits** (body, sensor, appendage shapes & colours) plus a **display name**.  These traits are stored on‑chain as immutable metadata and rendered client‑side through composable React components.

---

## 2  Trait Encoding

| Category             | Bits        | Values                                                                 |
| -------------------- | ----------- | ---------------------------------------------------------------------- |
| **Body shape**       | 4           | 16 SVG / mesh presets (slug, drake, beetle …)                          |
| **Body colour**      | 12          | 4 bits hue, 4 sat, 4 val – quantised H S V                             |
| **Sensor skin**      | 4           | 16 icon presets (antenna, radar dish …)                                |
| **Sensor colour**    | 12          | Same HSV quantisation                                                  |
| **Appendage shape**  | 4 per slot  | Claw, wing, wheel, tail …                                              |
| **Appendage colour** | 12 per slot | HSV                                                                    |
| **Name hash**        | 32          | `keccak256(UTF‑8 name)` stored; raw string lives in off‑chain tokenURI |

Total worst‑case extra storage ≤ 128 bits + one 32‑byte hash → < 80 gas.

---

## 3  Lock‑In Flow

1. **Customise UI** – drag‑drop palette and preset picker (React).
2. **Preview** – client renders `<BeastCanvas traits={…} />`.
3. **MintTx** `setTraits(creatureID, packedTraits, nameHash)` plus **fee = TRAIT\_LOCK\_FEE\_CORE**.
4. Contract checks:

   * Name length ≤ 24 chars, UTF‑8 printable.
   * Fee paid & trait slots ≤ active appendages/sensors.
5. Emits `TraitsLocked` event; traits become immutable.

DAO can adjust `TRAIT_LOCK_FEE_CORE` via SeasonRegistry.

---

## 4  React Component Structure

```tsx
<BeastCanvas>
  <Body preset={bodyShape} colour={bodyColour} />
  {sensors.map((s,i)=>(
     <Sensor key={i} preset={s.shape} colour={s.colour} />))}
  {appendages.map((a,i)=>(
     <Appendage key={i} preset={a.shape} colour={a.colour} />))}
  <NameTag>{displayName}</NameTag>
</BeastCanvas>
```

* Components read HSV, convert to RGB via util `hsvToRgb(h,s,v)`.
* All geometry is SVG or Three.js meshes keyed by **preset id**.

---

## 5  Off‑Chain Name Storage

* `tokenURI` JSON includes:

  ```json
  {
    "name": "FlareDrake",
    "attributes": [ … HSV & preset ids … ],
    "image": "data:image/svg+xml;base64,…"
  }
  ```
* `nameHash` on‑chain prevents tampering; UI loads full string from `tokenURI`.

---

## 6  Anti‑Abuse Filters

1. **Profanity** – client‑side + optional Chainlink oracle for on‑chain veto.
2. **HSV range clamp** – avoids seizure‑inducing flashes.
3. **One‑time lock** – traits cannot be changed after `setTraits` succeeds.

---

## 7  Gas & Cost

| Action                     | Gas (Arbitrum) | Est. Cost |
| -------------------------- | -------------- | --------- |
| Trait lock                 |  ≈60 k         |  < \$0.01 |
| Name oracle veto (if used) |  +40 k         | optional  |

---

*Draft prepared for UI/Smart Contract teams – 1 June 2025*
