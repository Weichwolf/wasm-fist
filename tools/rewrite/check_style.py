#!/usr/bin/env python3
"""Strict formatting and clang-tidy gate for owned rewrite sources only."""
import argparse
import json
import os
import pathlib
import subprocess

ROOT = pathlib.Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", type=pathlib.Path,
                        default=pathlib.Path("/tmp/wasm-fist-rewrite/native"))
    args = parser.parse_args()
    formatter = os.environ.get("CLANG_FORMAT", "clang-format")
    tidy = os.environ.get("CLANG_TIDY", "clang-tidy")
    for program in (formatter, tidy):
        result = subprocess.run([program, "--version"], check=True, capture_output=True, text=True)
        if "19.1." not in result.stdout:
            parser.error(f"{program} 19.1.x is required for reproducible strict checks")
    sources = sorted(path for base in (ROOT / "src", ROOT / "tools/rewrite")
                     for path in base.rglob("*") if path.suffix in (".c", ".h"))
    subprocess.run([formatter, "--dry-run", "--Werror", *map(str, sources)], check=True)
    database = json.loads((args.build_dir / "compile_commands.json").read_text())
    units = sorted({entry["file"] for entry in database
                    if pathlib.Path(entry["file"]).resolve() in sources})
    if not units:
        parser.error("No owned rewrite translation units in the compile database")
    missing = {str(path) for path in sources if path.suffix == ".c"} - set(units)
    if missing:
        parser.error(f"Owned C is missing from the checked compile database: {sorted(missing)}")
    subprocess.run([tidy, "--quiet", "-p", str(args.build_dir), *units], check=True)


if __name__ == "__main__":
    main()
