Type: Work item
Title: Publish the rewrite on master and the reconstruction on ghidra
Depends: 0040

## Contract

Move active rewrite development to master as requested. Retain the complete decompiled and
patched reconstruction on ghidra at the immutable reference revision. Preserve history and
unfinished local work, update branch documentation and route rewrite CI to master.

## Evidence

Before migration, local and remote master both point to 349ad31a9fd21b350d435651bb2e90afda40cf60,
exactly the annotated reference/reconstruction-v1 tag. This revision is an ancestor of the
rewrite at 52d3f66a03627923284f466ec9445b32c8497763; no divergent master commits need merging.
The existing remote default branch is master. No ghidra branch previously exists.

## Next

Continue active 0084 verification on master, then the unchanged full rewrite goal. The former
rewrite/softgl branch remains available as history; reference source and tag stay immutable.

## Accept

master contains the complete committed rewrite history; ghidra resolves to the exact frozen
reference commit. Documentation identifies these branches and both rewrite CI triggers select
master. Publish both refs with a normal atomic push, then independently inspect remote refs.
This branch-only change does not claim acceptance of unfinished 0084 implementation or full
gameplay. Existing production and strict checks continue for that separate work item.
