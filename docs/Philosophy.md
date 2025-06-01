# Project Philosophy – Chain‑Beasts

*Revision 1 • 1 June 2025*

---

## 1  Why We’re Doing This

Our **real goal** is to get *everyone*—kids, hobbyists, seasoned ML engineers—**hands‑on with training and fine‑tuning neural networks**.  Chain‑Beasts disguises that lesson inside a creature‑battling game where every click nudges parameters, burns compute, and leaves a cryptographically provable trace on‑chain.

1. **First‑contact learners** hatch a beast and visually watch loss drop as it learns to move or bite.
2. **Tinkerers** discover private curricula, strategy‑specific fine‑tunes, and the joys of hyper‑parameter schedules.
3. **Power users** venture into gradient‑stealing, zk‑proof engineering, and token‑economics—real research problems wearing a game skin.

If we succeed, teenagers will casually talk about “INT8 accumulator overflow” the way they now talk about critical hits in *that one game who's name shall not be mentioned.*

---

## 2  Heads, Sensors & Appendages — a Mental Model

Picture the creature as a modular agent:

```
┌────────────┐          ┌──────────┐        ┌──────────────┐
│  Sensors   │ ───────▶ │ Core NN  │ ─────▶ │ Appendages   │
└────────────┘  dx,dy   └──────────┘ logits └──────────────┘
```

* **Sensors** map raw integers from the battlefield or UI into embeddings.
* The **head / core model** (today an INT8 MLP, tomorrow maybe an Mixture‑of‑Experts or small LLM) reasons over those embeddings.
* **Appendages** translate logits into concrete moves, attacks, fashion poses, etc.

Because every slab is deterministic and proof‑friendly, players can swap shapes, widths, even whole sub‑nets at hatch time without breaking verifiability.  The architecture forces respect for *interface contracts*—a real‑world lesson in modular ML.

---

## 3  Learning Ladder & Hidden Depth

| Tier               | Player activity                                              | Under‑the‑hood concept learned      |
| ------------------ | ------------------------------------------------------------ | ----------------------------------- |
| **Newbie**         | Hatch, watch loss graph drop.                                | Weight initialisation, SGD.         |
| **Strate‑geek**    | Fine‑tune with private curriculum per match.                 | Curriculum learning, data leakage.  |
| **Data scientist** | Attempt to infer opponent’s dataset from observed gradients. | Model inversion, gradient analysis. |
| **Crypto hacker**  | Build alternative zk‑provers or break quorum replay.         | SNARKs, consensus economics.        |

The game never hides the math; it simply rewards curiosity with stronger beasts or more \$CORE.

---

## 4  Tokenomics in One Glance

| Token        | Role                  | Mint                                        | Burn                                      |
| ------------ | --------------------- | ------------------------------------------- | ----------------------------------------- |
| **\$CORE**   | Trade & governance    | Block reward proportional to proved compute | Breeding fees, trait locks                |
| **\$ENERGY** | Pay‑as‑you‑train fuel | Earned in battles & validator attestations  | Every 128‑step micro‑batch, growth spurts |

Full spec → [Token section of the whitepaper](whitepaper/ChainBeastsWhitepaper.tex#tokenomics).
Inflation curves are DAO‑tuned; ENERGY is non‑transferable to keep compute egalitarian.

---

## 5  Game Loops (High Level)

1. **Hatch** – deterministic genesis from birth block.  Details → [Hatching.md](Hatching.md).
2. **Customise** – lock body/sensor colours & name.  Details → [CustomVisualization.md](CustomVisualization.md).
3. **Train** – spend ENERGY, submit 128‑step checkpoints.  Kernel spec → [KernelNeuralSpec.md](KernelNeuralSpec.md).
4. **Private Coach** – commit & reveal per‑battle curriculum.  Protocol → [PreBattleCurriculum.md](PreBattleCurriculum.md)
5. **Battle** – deterministic grid combat.  Logic → [BattlefieldDesign.md](BattlefieldDesign.md).
6. **Proof‑of‑Useful‑Work** – your training *is* mining - [PoUWConsensus.md](PoUWConsensus.md).

---

## 6  Why Determinism, Proofs, and Public Math?

* **Auditability** – children can replay a match and know the chain didn’t lie.
* **Interoperability** – any indie dev can fork the contracts and kernel to start a custom league.
* **Research value** – gradient traces + on‑chain datasets form an open ML corpora.

The cost? Constraints on network size and INT‑only maths.  We think the educational upside dwarfs those limits.

---

## 7  Document Map

* Architecture blueprint → [Architecture.md](Architecture.md)
* Deterministic kernel → [KERNEL\_SPEC.md](KERNEL_SPEC.md)
* Variable anatomy & seeds → [HATCHING.md](HATCHING.md)
* Battle & labels → [COMBAT\_TRAINING.md](COMBAT_TRAINING.md)
* Private curricula → [PRIVATE\_CURRICULUM.md](PRIVATE_CURRICULUM.md)
* Reveal protocol → [CURRICULUM\_REVEAL\_PROTOCOL.md](CURRICULUM_REVEAL_PROTOCOL.md)

Start there, fork anything, share improvements—*that* is the spirit of Chain‑Beasts.

---

© 2025 ChainBeasts Labs – Let’s make learning ML as universal as learning to ride a bike.
