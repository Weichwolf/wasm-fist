#!/usr/bin/env python3
"""Actual SDL input/presentation/pause/shutdown gate in an isolated Xvfb display."""
import argparse
import hashlib
import os
import pathlib
import subprocess
import tempfile
import time


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--scenario', required=True, type=pathlib.Path)
    parser.add_argument('--assets', required=True, type=pathlib.Path)
    parser.add_argument('--native-preview', type=pathlib.Path,
                        default=pathlib.Path('/tmp/wasm-fist-rewrite/native/fist_driving_preview'))
    parser.add_argument('--output-dir', type=pathlib.Path)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='wasm-fist-driving-display-', dir='/tmp') as temporary:
        output = args.output_dir.resolve() if args.output_dir else pathlib.Path(temporary)
        if not output.is_relative_to(pathlib.Path('/tmp')) or output == pathlib.Path('/tmp'):
            parser.error('Visual evidence belongs in a dedicated directory under /tmp')
        output.mkdir(parents=True, exist_ok=True)
        display = subprocess.Popen(['Xvfb', '-displayfd', '1', '-screen', '0', '800x600x24', '-nolisten', 'tcp'],
                                   stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True)
        game = None
        try:
            number = display.stdout.readline().strip()
            if not number.isdecimal():
                raise RuntimeError('Isolated Xvfb did not start')
            env = dict(os.environ, DISPLAY=f':{number}', SDL_VIDEODRIVER='x11')
            def xdo(*arguments):
                return subprocess.check_output(['xdotool', *map(str, arguments)], env=env, timeout=5).decode().strip()
            game = subprocess.Popen([str(args.native_preview), str(args.scenario), str(args.assets), '2048'],
                                    env=env, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
            window = None
            for _ in range(100):
                if game.poll() is not None:
                    raise RuntimeError(f'Native scene exited early: {game.stderr.read().decode()}')
                try:
                    window = xdo('search', '--pid', game.pid, '--name', r'^Armored Fist \| W/S').splitlines()[0]
                    break
                except subprocess.CalledProcessError:
                    time.sleep(.05)
            if window is None:
                raise RuntimeError('Native scene did not create its window')
            xdo('windowfocus', '--sync', window)
            time.sleep(.1)
            xdo('key', '--window', window, 'p')
            time.sleep(.1)
            def capture(name):
                path = output / (name + '.png')
                subprocess.run(['import', '-window', window, str(path)], env=env, check=True, timeout=5)
                pixels = subprocess.check_output(['convert', str(path), '-depth', '8', 'rgba:-'], timeout=5)
                if len(pixels) != 640 * 400 * 4 or set(pixels[3::4]) != {255}:
                    raise AssertionError('Missing/incomplete native frame')
                if len({pixels[i:i + 3] for i in range(0, len(pixels), 4)}) <= 256:
                    raise AssertionError('Missing textured terrain/vehicle output')
                return hashlib.sha256(pixels).hexdigest()
            before = capture('native-before')
            time.sleep(.15)
            assert capture('native-paused') == before, 'Paused native scene must preserve the complete frame'
            xdo('key', '--window', window, 'p')
            xdo('keydown', '--window', window, 'w', 'd', 'e')
            time.sleep(1.5)
            xdo('keyup', '--window', window, 'w', 'd', 'e')
            xdo('key', '--window', window, 'p')
            time.sleep(.1)
            after = capture('native-after')
            assert after != before, 'Actual held SDL inputs must change the displayed scene'
            xdo('key', '--window', window, 'p')
            xdo('keydown', '--window', window, 'w')
            xdo('windowfocus', '0')
            time.sleep(.15)
            frozen = capture('native-focus-lost')
            xdo('windowfocus', '--sync', window)
            xdo('keyup', '--window', window, 'w')
            time.sleep(.2)
            assert capture('native-focus-return') == frozen, 'Focus loss must pause and release controls'
            # Escape closes on keydown; do not send keyup to the destroyed window.
            xdo('keydown', '--window', window, 'Escape')
            assert game.wait(timeout=5) == 0, game.stderr.read().decode()
            assert game.stderr.read() == b'', 'Native runtime must be clean'
            print(f'Native SDL: complete frames, held input, pause, focus and shutdown pass; before={before} after={after}')
        finally:
            if game is not None and game.poll() is None:
                game.terminate()
                game.wait(timeout=5)
            display.terminate()
            display.wait(timeout=5)


if __name__ == '__main__':
    main()
