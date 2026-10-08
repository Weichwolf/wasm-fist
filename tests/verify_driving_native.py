#!/usr/bin/env python3
"""Actual SDL input/presentation/pause/shutdown gate in an isolated Xvfb display."""
import argparse
import hashlib
import os
import pathlib
import subprocess
import tempfile
import time


# Reviewed 640x400 native TRAIN1 frames show this authored PAUSED label.
# Synchronize presentation to that visible state before testing whole-frame
# stability; a fixed sleep cannot acknowledge queued SDL input/publication.
PAUSED_LABEL = 'a0f9f12a4cc37830cc11501af619207a8b36a43a34290a73982fc0e1eb68017c'


def has_paused_label(pixels):
    left, top, width, height = 178, 316, 70, 14
    label = b''.join(pixels[((top + row) * 640 + left) * 4:
                           ((top + row) * 640 + left + width) * 4] for row in range(height))
    return hashlib.sha256(label).hexdigest() == PAUSED_LABEL


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--mission', action='store_true')
    parser.add_argument('--scenario', required=True, type=pathlib.Path)
    parser.add_argument('--assets', required=True, type=pathlib.Path)
    parser.add_argument('--native-preview', type=pathlib.Path,
                        default=pathlib.Path('/tmp/wasm-fist-rewrite/native/fist_driving_preview'))
    parser.add_argument('--output-dir', type=pathlib.Path)
    parser.add_argument('--settle-seconds', type=float, default=.25,
                        help='Allow display publication after input; use 1 for sanitizer builds')
    parser.add_argument('--timeout-seconds', type=float, default=5,
                        help='Window/frame/shutdown deadline; use 30 for Valgrind')
    args = parser.parse_args()
    if not 0 < args.settle_seconds <= 5:
        parser.error('Display settlement must be positive and at most five seconds')
    if not 0 < args.timeout_seconds <= 60:
        parser.error('Display deadline must be positive and at most sixty seconds')
    with tempfile.TemporaryDirectory(prefix='wasm-fist-driving-display-', dir='/tmp') as temporary:
        output = args.output_dir.resolve() if args.output_dir else pathlib.Path(temporary)
        if not output.is_relative_to(pathlib.Path('/tmp')) or output == pathlib.Path('/tmp'):
            parser.error('Visual evidence belongs in a dedicated directory under /tmp')
        output.mkdir(parents=True, exist_ok=True)
        # Keep short-lived xdotool discovery clients from resetting the server
        # while SDL opens its separate display and request connections.
        display = subprocess.Popen(['Xvfb', '-displayfd', '1', '-screen', '0', '800x600x24',
                                    '-nolisten', 'tcp', '-noreset'],
                                   stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True)
        game = None
        try:
            number = display.stdout.readline().strip()
            if not number.isdecimal():
                raise RuntimeError('Isolated Xvfb did not start')
            env = dict(os.environ, DISPLAY=f':{number}', SDL_VIDEODRIVER='x11')
            def xdo(*arguments):
                return subprocess.check_output(['xdotool', *map(str, arguments)], env=env, timeout=5).decode().strip()
            game = subprocess.Popen([str(args.native_preview), str(args.scenario), str(args.assets), '2048', *(['mission'] if args.mission else [])],
                                    env=env, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
            window = None
            deadline = time.monotonic() + args.timeout_seconds
            while time.monotonic() < deadline:
                if game.poll() is not None:
                    raise RuntimeError(f'Native scene exited early: {game.stderr.read().decode()}')
                try:
                    window = xdo('search', '--onlyvisible', '--pid', game.pid,
                                 '--name', r'^Armored Fist \| W/S').splitlines()[0]
                    break
                except subprocess.CalledProcessError:
                    time.sleep(.05)
            if window is None:
                raise RuntimeError('Native scene did not create its window')
            xdo('windowfocus', '--sync', window)
            time.sleep(args.settle_seconds)
            xdo('key', '--window', window, 'p')
            time.sleep(args.settle_seconds)
            def capture(name, *, paused=None):
                path = output / (name + '.png')
                subprocess.run(['import', '-window', window, str(path)], env=env, check=True, timeout=5)
                pixels = subprocess.check_output(['convert', str(path), '-depth', '8', 'rgba:-'], timeout=5)
                if len(pixels) != 640 * 400 * 4 or set(pixels[3::4]) != {255}:
                    raise AssertionError('Missing/incomplete native frame')
                if len({pixels[i:i + 3] for i in range(0, len(pixels), 4)}) <= 256:
                    # A mapped SDL window may precede its first rendered frame.
                    # Await publication only inside the bounded acknowledgement;
                    # direct/stability captures must already be complete.
                    if paused is not None:
                        return None
                    raise AssertionError('Missing textured terrain/vehicle output')
                if paused is not None and has_paused_label(pixels) != paused:
                    return None
                return hashlib.sha256(pixels).hexdigest()
            def published_frame(name, paused):
                deadline = time.monotonic() + args.timeout_seconds
                attempts = 0
                while time.monotonic() < deadline:
                    waiting = f'{name}-await-{attempts}'
                    digest = capture(waiting, paused=paused)
                    if digest is not None:
                        (output / (waiting + '.png')).replace(output / (name + '.png'))
                        if attempts:
                            print(f'Native visible pause={paused} acknowledged after {attempts} earlier frames: {name}', flush=True)
                        return digest
                    attempts += 1
                    time.sleep(.05)
                raise AssertionError(f'Native pause={paused} was not published: {name}')
            before = published_frame('native-before', True)
            time.sleep(.15)
            assert capture('native-paused') == before, 'Paused native scene must preserve the complete frame'
            xdo('key', '--window', window, 'p')
            published_frame('native-driving-resumed', False)
            xdo('keydown', '--window', window, 'w', 'd', 'e')
            time.sleep(1.5)
            xdo('keyup', '--window', window, 'w', 'd', 'e')
            xdo('key', '--window', window, 'p')
            time.sleep(args.settle_seconds)
            after = published_frame('native-after', True)
            assert after != before, 'Actual held SDL inputs must change the displayed scene'
            xdo('key', '--window', window, '2')
            time.sleep(args.settle_seconds)
            assert capture('native-weapon-paused') == after, 'Paused weapon presses must not change the scene'
            xdo('key', '--window', window, 'p')
            published_frame('native-weapon-resumed', False)
            xdo('key', '--window', window, '1', '2')
            xdo('keydown', '--window', window, '1')
            time.sleep(.1)
            xdo('keydown', '--window', window, '1')
            xdo('keyup', '--window', window, '1')
            xdo('key', '--window', window, 'p')
            time.sleep(args.settle_seconds)
            selected = published_frame('native-weapon-selected', True)
            assert selected != after, 'Actual SDL weapon selection must reach the displayed HUD'
            time.sleep(.15)
            assert capture('native-weapon-stable') == selected, 'Paused reload/weapon display remains stable'
            xdo('key', '--window', window, 'p')
            published_frame('native-cycle-resumed', False)
            xdo('key', '--window', window, 'Tab')
            xdo('key', '--window', window, 'p')
            time.sleep(args.settle_seconds)
            assert published_frame('native-weapon-cycled', True) != selected, 'SDL Tab must update the displayed weapon and store'
            xdo('key', '--window', window, 'p')
            published_frame('native-focus-resumed', False)
            xdo('keydown', '--window', window, 'w')
            xdo('windowfocus', '0')
            time.sleep(args.settle_seconds)
            frozen = published_frame('native-focus-lost', True)
            xdo('windowfocus', '--sync', window)
            xdo('keyup', '--window', window, 'w')
            time.sleep(.2)
            assert capture('native-focus-return') == frozen, 'Focus loss must pause and release controls'
            # Escape closes on keydown; do not send keyup to the destroyed window.
            xdo('keydown', '--window', window, 'Escape')
            assert game.wait(timeout=args.timeout_seconds) == 0, game.stderr.read().decode()
            assert game.stderr.read() == b'', 'Native runtime must be clean'
            print(f'Native SDL: complete frames, held input, pause, focus and shutdown pass; before={before} after={after}')
        finally:
            if game is not None and game.poll() is None:
                game.terminate()
                game.wait(timeout=args.timeout_seconds)
            display.terminate()
            display.wait(timeout=5)


if __name__ == '__main__':
    main()
