#!/usr/bin/env python3
"""Generate every owned map recipe sequentially; no original files are needed."""
import argparse
import json
from pathlib import Path
import subprocess
import sys

from mapgen import load_recipe, validate


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--recipes-dir', type=Path,
                        default=Path(__file__).resolve().parents[1] / 'maps')
    parser.add_argument('--output-dir', type=Path, required=True)
    args = parser.parse_args()
    recipes = sorted(args.recipes_dir.resolve().glob('*.json'))
    if not recipes:
        raise ValueError('No map recipes found')
    names = []
    for path in recipes:
        recipe = load_recipe(path); validate(recipe)
        if recipe['name'] != path.stem or recipe['name'] in names:
            raise ValueError('Map names must be unique and match recipe filenames')
        names.append(recipe['name'])
    generator = Path(__file__).resolve().with_name('mapgen.py')
    for path in recipes:
        subprocess.run([sys.executable, str(generator), str(path),
                        '--output-dir', str(args.output_dir.resolve())], check=True)
    print(json.dumps({'success': True, 'maps': names}, sort_keys=True))


if __name__ == '__main__':
    main()
