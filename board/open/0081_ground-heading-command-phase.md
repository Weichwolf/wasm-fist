Type: Work item
Title: Consume the reached ground heading and command phase
Depends: 0080

## Contract

Recover the next reached complete ground phase, including its owned heading history/counter,
shared RNG consumption and nested controlled/AI command callback. Preserve one canonical actor
and world RNG owner. Keep the existing motion/history/maintenance/reload/contact contracts;
do not omit nested work to make a heading-only comparison pass. Complete living behavior,
inter-object contacts, battle scheduling/rendering and PCM remain required.

## Evidence

After maintenance, the actual installed TRAIN1 M1 prefix 7c1d..7c7e agrees through six updates.
Tick seven reaches phase 46, bank byte index 14, and calls ab03 plus RNG 0291. Raw words
+28/+2a/+2c/+2e and byte +42 first differ. With zero seeds, the retained world RNG cursor
advances from two to three; seeds remaining zero do not prove lack of random consumption.

Actual ab03 stores DI, calls RNG before its global admission check, increments +42 when
admitted and samples heading history on its low-four-bit boundary. MOVSX additions use signed
word operands with logical 32-bit SHR before storing the average word. It resolves both
platoon tables, selects a nested callback from controlled/AI banks and may call b152 for
the global selected actor. The first controlled wrap selects ab82 -> f69:b4ef, raw 1ab7f;
that service has actual behavior/flag/target/platoon-dependent branches and is not an empty
return. Verify all other class tables, callbacks and admission/global callers before claiming
their support. Original observation rejects reserved address-zero/one device scaffolding.

## Next

Reproduce the complete first difference and recover the nested callback plus caller/global
contract. Add the required saved heading/counter state and connect the shared RNG transaction
to actual canonical control and any still-delivered standalone boundary. Consume complete
controlled work and explicitly account for other callback requirements. Advance the original
living prefix again; keep the engine-audio device boundary declared until fully configured.

## Accept

Both production targets pass complete original phase/callback comparisons, all saved ground
snapshots, reaching TRAIN1 state and RNG traces, transactional failures, canonical/standalone
timed regressions, strict builds/style and memory checks. Preserve actual native/browser
controls and frames. Full class/battle/PCM/mission and final ten-run acceptance remain separate.
