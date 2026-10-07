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

## Verified publication

Commit 765d0144bac729184984af5736bea2b4557716dc contains only the branch/documentation/CI
migration. `git push --atomic origin master ghidra` returns zero without force. Independent
`git ls-remote --heads origin master ghidra rewrite/softgl` confirms master at that commit,
ghidra at 349ad31a9fd21b350d435651bb2e90afda40cf60 and the former rewrite branch at 52d3f66.
The immutable annotated tag still resolves to the exact ghidra commit. Both CI branch triggers
select master, and no unfinished 0084 source is included in the migration commit. A compact
receipt remains under /tmp/wasm-fist-0085-review/summary.json. Subsequent rewrite commits advance
master normally; ghidra and the frozen reference stay unchanged.
