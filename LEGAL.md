# LEGAL & COMPLIANCE OVERVIEW – CHAIN‑BEASTS

*Last updated: 1 June 2025 · Draft for community review*

> **Disclaimer**  This document is provided **for informational purposes only** and does *not* constitute legal advice. Consult qualified counsel in your jurisdiction before relying on any statement herein.

---

## 1 Jurisdictional Scope

* Primary analysis aligns with **United States** federal law (SEC, CFTC, FinCEN) and selected **EU** regulations (MiCA, AMLD6, GDPR).
* Local statutes may impose additional or conflicting obligations—operators outside the U.S./EU must run their own review.

---

## 2 Token Classification

| Token        | Intended legal posture               | Key arguments                                                                                                                                     |
| ------------ | ------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------- |
| **\$CORE**   | Digital commodity / utility token    | *Use-based mint* (PoUW), no passive profit expectation, open-source protocol; mirrors frameworks in the **Digital Commodity Exchange Act** draft. |
| **\$ENERGY** | In‑game consumable, non‑transferable | Cannot be traded for value; solely burns to access compute or cast fashion votes → falls outside Howey / Reves scope.                             |

> If \$CORE ever becomes **redeemable** for off‑chain assets, Chain‑Beasts Labs (or its successor DAO) must register as a **Money Services Business** (FinCEN) and acquire state money‑transmitter licences or use a licensed custodian (see § 5).

---

## 3 KYC & AML Controls

* **Allowed‑list** — all \$CORE transfers invoke an on‑chain check against a zero‑knowledge KYC oracle (Sardine/Persona).
* **Sanctions screening** — wallet addresses flagged by OFAC or Council Regulation 2580/2001 automatically blocked.
* **Minors** — under‑13 users restricted to ENERGY‑only gameplay; no on‑chain \$CORE custody (COPPA alignment).

---

## 4 Securities & DAO Governance

1. **Governance staking** grants voting rights but **no revenue share**.
2. DAO proposals are limited to *protocol parameters*; treasury disbursements require an additional “business purpose” vote to avoid implicit dividends.
3. `Governance.sol` includes **no expectation‑of‑profit** disclaimer in proposal metadata.

---

## 5 Redemption & Custodial Path (future‑proof)

If \$CORE can be swapped for fiat:

1. Burn event emits `RedemptionRequest(owner, amount)`.
2. Licensed custodian (or multiple, region‑specific) off‑chain KYC match.
3. Funds wired via ACH/SEPA; custodian files SAR/CTR reports as required.
4. Chain‑Beasts DAO maintains list of compliant custodians; governance may rotate providers.

---

## 6 Intellectual Property

* **Code** — Apache‑2.0; contributors sign DCO.
* **Art & SVG meshes** — CC‑BY‑4.0, unless otherwise noted.
* **“Chain‑Beasts”™** — intent‑to‑use trademark filed USPTO; third‑party forks must avoid confusing similarity.

User‑generated names and curricula remain property of the creator; by submitting on‑chain, the creator grants a **worldwide, perpetual, sublicensable licence** for gameplay and marketing.

---

## 7 Privacy & Data Protection

* On‑chain datasets may inadvertently contain personal data.
* **Dataset reveal** Tx warns uploader and requests they strip PII; uploader bears liability.
* Off‑chain mirrors must provide GDPR Article 17 “right to erasure” pathways; DAO treasury earmarks funds for takedown compliance.

---

## 8 Loot‑Box / Gambling Analysis

* Fashion‑duel quadratic voting resembles staking, **not chance** → outside loot‑box definitions.
* Battle seeds derive from deterministic block hashes; no paid random loot.
* Therefore, low risk under U.S. state gambling tests (Dominant Factor Test).

---

## 9 Tax Considerations (U.S.)

| Activity             | Possible tax treatment                                            |
| -------------------- | ----------------------------------------------------------------- |
| Block rewards (CORE) | Ordinary income at receipt FMV                                    |
| Sale of CORE         | Capital gain/loss                                                 |
| Breeding fee burn    | N/A (capital basis reduces)                                       |
| ENERGY earnings      | Non‑taxable in‑game units unless converted to CORE (not possible) |

Players must track CORE basis; DAO will provide optional **Form 8949 CSV export** in the wallet UI.

---

## 10 Risk Factors (Non‑exhaustive)

* Regulatory shifts could reclassify CORE as a security.
* Bugs in zk‑proof verifier may allow fake blocks.
* Dataset reveals might leak proprietary or sensitive data.
* Energy inflation or DAO capture could erode incentive alignment.

---

## 11 Future Legal Work

1. EU MiCA white‑paper registration (if trading venue in EU).
2. Canada MSB review under FINTRAC (if custodial redemption enabled).
3. Australian DCE licence feasibility study.
4. COPPA Safe‑Harbor certification for under‑13 gameplay.

---

© 2025 ChainBeasts Labs – All rights reserved. This document may be reproduced with attribution.")}
