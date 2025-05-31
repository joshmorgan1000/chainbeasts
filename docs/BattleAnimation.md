# Battle Animation Workflow

The battle animation system provides a deterministic replay of every fight so clients can present the same outcome across devices. Each frame stores the attacker flag, remaining hit points and stamina for both creatures.

## 1. Overview

`BattleAnimator` in the C++ library mirrors the turn logic used by the matchmaker. When `run()` is called with two `CreatureStats` objects it simulates the encounter and records a list of `BattleFrame` structs.

## 2. Generating an Animation

```cpp
neuropet::CreatureStats a{1, 1, 1, 2};
neuropet::CreatureStats b{2, 1, 1, 2};
neuropet::BattleAnimator anim;
anim.run(a, b, /*seed=*/42);
```

`run()` stores each turn in `frames()` and sets `winner()` to the ID of the victor. A fixed seed ensures the same sequence on every platform.

## 3. Frame Format

Each `BattleFrame` contains:

* `attacker_is_a` – `true` when creature A attacks.
* `hp_a`/`hp_b` – remaining hit points after the action.
* `stamina_a`/`stamina_b` – stamina values after the action.

Clients can step through the vector to animate attacks or produce logs.

## 4. Replaying

Use the recorded frames to drive a UI or verify an on-chain result. Because the logic is deterministic, the animation will always match the official arena computation.

---

© 2025 Cognithesis Labs
