Type: bug
Title: AI fire requests reach the appropriate weapon-class handlers

This engine can issue AI fire requests and execute the corresponding weapon-class countdowns.
The original “units never fire” diagnosis was disproven, not repaired by a new fire implementation.

## Evidence

A corrected AZER1 census observed a286 fire requests and class-specific 91b8 activity; the live
object count fell from 13 to 11. The initial counter watched only 7e29 and missed other classes.
The earlier claim that linear 0x7745 was the spawn entry also ignored segment context.

## Preserve

Count the actual handlers for every reached class and prove each diagnostic can observe a nonzero
case. Counter silence is not proof of unreachable engine behavior. Broader combat/oracle fidelity
belongs to board:0017; dispatch completeness belongs to board:0015.
