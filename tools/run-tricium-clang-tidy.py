#!/usr/bin/env python3
# Copyright 2026 The Dawn & Tint Authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# For info about this script, see docs/clang-tidy.md.

import argparse
import datetime
import json
import shlex
import shutil
import subprocess
import sys
import time
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
DAWN_ROOT = SCRIPT_DIR.parent


def fail(message: str) -> None:
    print(message, file=sys.stderr)
    sys.exit(1)


def run_command(args: list, capture_stdout: bool = False) -> str:
    str_args = [str(arg) for arg in args]
    try:
        result = subprocess.run(
            str_args,
            stdout=subprocess.PIPE if capture_stdout else None,
            text=True,
            check=True,
        )
        return result.stdout if capture_stdout else ''
    except subprocess.CalledProcessError as e:
        fail(f'Command failed: {" ".join(str_args)}\nError: {e}')


def find_source_files(paths: list[Path]) -> list[str]:
    extensions = {
        '.c',
        '.cc',
        '.cpp',
        '.cxx',
        '.h',
        '.hh',
        '.hpp',
        '.hxx',
        '.m',
        '.mm',
    }
    files = []
    for path in paths:
        p = Path(path).resolve()
        if p.is_file():
            if p.suffix.lower() in extensions:
                files.append(str(p.relative_to(DAWN_ROOT)))
            else:
                print(f'Skipped non-C/C++ file: {path}', file=sys.stderr)
        elif p.is_dir():
            for item in p.rglob('*'):
                if item.is_file() and item.suffix.lower() in extensions:
                    files.append(str(item.relative_to(DAWN_ROOT)))
        else:
            fail(f'Not found: {path}')
    return files


def main():
    if Path.cwd().resolve() != DAWN_ROOT.resolve():
        fail(f'Error: must be run from the Dawn root directory.')

    parser = argparse.ArgumentParser(
        description='Run clang-tidy on Dawn source files and directories.',
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    # Options
    parser.add_argument(
        '--clean',
        action=argparse.BooleanOptionalAction,
        required=True,
        help=
        'Pass --clean to clean the build before building and running clang-tidy. Pass --no-clean to skip that, and promise there are no leftover no-longer-used generated files from old Dawn revisions.'
    )
    parser.add_argument(
        '--stdout-only',
        action='store_true',
        help=
        'Print the summary to stdout instead of saving the JSON and summary files to disk. Useful when iterating on a specific small directory of files.'
    )
    parser.add_argument('outdir', help='The build directory, e.g., out/debug')

    # Source files
    source_group = parser.add_mutually_exclusive_group(required=True)
    source_group.add_argument(
        '--default',
        action='store_true',
        help='Use default sources (all of our source files).')
    source_group.add_argument(
        '--all',
        action='store_true',
        help='Run across all C/C++ files known to ninja (not recommended).')
    source_group.add_argument(
        'sources',
        nargs='*',
        help=
        'Specific source files and/or directories to run clang-tidy on. Any files which are not part of the build will be ignored.'
    )

    args = parser.parse_args()

    outdir = Path(args.outdir)

    # Locate required binaries/scripts
    clang_tidy = Path('third_party/llvm-build/Release+Asserts/bin/clang-tidy')
    if sys.platform == 'win32':
        clang_tidy = clang_tidy.with_suffix('.exe')
    if not clang_tidy.is_file():
        fail(f'{clang_tidy} missing. See docs/clang-tidy.md.')

    recipe_script = Path(
        'third_party/chromium-tools-build/src/recipes/recipe_modules/tricium_clang_tidy/resources/tricium_clang_tidy_script.py'
    )
    if not recipe_script.is_file():
        fail(f'{recipe_script} missing. See docs/clang-tidy.md.')

    run_command(['gn', 'gen', outdir])

    # Check that Wasm is disabled, because clang-tidy doesn't understand em++
    # commands (it doesn't know where the sysroot is because it's implicit).
    #
    # TODO(crbug.com/501491694): Fix this by setting the sysroot via additional
    # flag in .clang-tidy, but is there a way to do this only for Emscripten
    # commands? Also add third_party/emdawnwebgpu and outdir/wasm/gen
    # (or outdir/*/gen) to --default if this is fixed.
    gn_has_wasm = run_command(
        ['gn', 'args', outdir, '--short', '--list=dawn_build_emdawnwebgpu'],
        capture_stdout=True,
    ).strip() != 'dawn_build_emdawnwebgpu = false'
    if gn_has_wasm:
        fail('Wasm build must be disabled for clang-tidy. Set '
             'dawn_build_emdawnwebgpu = false in args.gn.')

    # Clean if requested.
    if args.clean:
        run_command(['gn', 'clean', outdir])

    # Make sure the build is up to date. Really we only need to regenerate
    # generated files, but building everything doesn't take that long.
    run_command(['autoninja', '-C', outdir])

    # Get the source paths from the command line, or from the defaults.
    source_paths = []
    if args.default:
        source_paths = [
            DAWN_ROOT / 'include',
            DAWN_ROOT / 'src',
            outdir / 'gen' / 'include',
            outdir / 'gen' / 'src',
        ]
    elif args.sources:
        source_paths = [Path(s) for s in args.sources]

    # Find source files
    source_files = []
    if not args.all:
        source_files = find_source_files(source_paths)
        if not source_files:
            fail('No matching source files found to run clang-tidy on.')

    timestamp = datetime.datetime.now().strftime('%Y-%m-%d_%H%M%S')
    findings_file = Path(f'clang-tidy-{timestamp}-findings.json')
    summary_file = Path(f'clang-tidy-{timestamp}-summary.txt')

    script_args = [
        f'--base_path={DAWN_ROOT}',
        f'--out_dir={outdir}',
        f'--clang_tidy_binary={clang_tidy}',
        f'--findings_file={findings_file}',
        '--no_clean',  # We handled cleaning/building already, if requested
        '--windows',  # Also recognize windows-style compile commands if there are any
    ]

    print('Running on:')
    if args.all:
        script_args.append('--all')
        print('  --all')
        print(f'Clang-tidy will run on ALL files used in the build.')
    else:
        for d in source_paths:
            print(f'  {d}')
        print(f'Found {len(source_files)} source files to lint.')
        script_args.extend(source_files)

    run_command([recipe_script] + script_args)

    # Lists of diagnostics. Each item is a comparable tuple of strings+numbers,
    # allowing the array to be sorted. Then the tuple is concatenated to print.
    summary_unknowns = []
    summary_compiler = []
    summary_tidy = []

    with open(findings_file) as f:
        findings = json.load(f)

        seen_file_paths = set()
        for diagnostic in findings['diagnostics']:
            file_path = diagnostic['file_path']
            line_number = diagnostic['line_number']
            diag_name = diagnostic['diag_name']
            message = diagnostic['message']
            line = (f'{file_path}:', line_number, f': {message} [{diag_name}]')
            if diag_name.startswith('clang-diagnostic-'):
                summary_compiler.append(line)
            else:
                summary_tidy.append(line)
            seen_file_paths.add(file_path)
            for expansion_loc in diagnostic['expansion_locs']:
                seen_file_paths.add(expansion_loc['file_path'])

        for file_path in findings['failed_tidy_files']:
            if file_path not in seen_file_paths:
                summary_unknowns.append(
                    (file_path, ': found in failed_tidy_files'))
        for file_path in findings['failed_src_files']:
            if file_path not in seen_file_paths:
                summary_unknowns.append(
                    (file_path, ': found in failed_src_files'))
        for file_path in findings['timed_out_src_files']:
            if file_path not in seen_file_paths:
                summary_unknowns.append(
                    (file_path, ': found in timed_out_src_files'))

    summary_unknowns.sort()
    summary_compiler.sort()
    summary_tidy.sort()

    print('\n------------------------------------------------------------\n')
    with (sys.stdout if args.stdout_only else open(summary_file, 'w')) as f:
        f.write('Command line:\n')
        f.write('  ' + shlex.join(sys.argv[:]) + '\n')
        f.write('\nFiles that failed with no associated diagnostics:\n')
        for line in summary_unknowns:
            f.write('  ' + ''.join(map(str, line)) + '\n')
        f.write('\nCompiler diagnostics (clang-diagnostic-*):\n')
        for line in summary_compiler:
            f.write('  ' + ''.join(map(str, line)) + '\n')
        f.write('\nClang-Tidy diagnostics:\n')
        for line in summary_tidy:
            f.write('  ' + ''.join(map(str, line)) + '\n')

    if args.stdout_only:
        findings_file.unlink()
    else:
        print(f'Full findings JSON saved to {findings_file}')
        print(f'Readable summary saved to {summary_file}')


if __name__ == '__main__':
    main()
