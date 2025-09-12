# Copyright 2025 The Dawn & Tint Authors
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
"""Helper code for running isolated versions of NodeJS."""

import glob
import functools
import os

THIS_DIR = os.path.dirname(__file__)
DAWN_ROOT = os.path.realpath(os.path.join(THIS_DIR, '..', '..'))
NODE_DIR = os.path.join(DAWN_ROOT, 'third_party', 'node')


@functools.cache
def get_node_path() -> str:
    """Retrieves the path to the node executable.

    Returns:
        A filepath to the standalone node executable.
    """
    paths = glob.glob(os.path.join('*', 'bin', 'node'), root_dir=NODE_DIR)
    if not paths:
        raise RuntimeError(
            f'Unable to find the node binary under {NODE_DIR}. Is the '
            f'dawn_node gclient variable set?')
    if len(paths) > 1:
        raise RuntimeError(
            f'Found multiple node binaries when one is expected. Found: '
            f'{", ".join(paths)}')
    return os.path.join(NODE_DIR, paths[0])


@functools.cache
def get_npm_command() -> list[str]:
    """Retrieves a command to run npm

    Returns:
        A list of strings that will run "npm" when run as a command for a
        subprocess.
    """
    paths = glob.glob(os.path.join('*', 'lib', 'node_modules', 'npm', 'bin',
                                   'npm-cli.js'),
                      root_dir=NODE_DIR)
    if not paths:
        raise RuntimeError(
            f'Unable to find the npm-cli.js file under {NODE_DIR}. Is the '
            f'dawn_node gclient variable set?')
    if len(paths) > 1:
        raise RuntimeError(
            f'Found multiple npm-cli.js files when one is expected. Found: '
            f'{", ".join(paths)}')
    cmd = [
        get_node_path(),
        os.path.join(NODE_DIR, paths[0]),
    ]
    return cmd


@functools.cache
def get_npx_command() -> list[str]:
    """Retrieves a command to run npx.

    Returns:
        A list of strings that will run "npx" when run as a command for a
        subprocess.
    """
    # npx is normally an alias for "npm exec", so just use that instead of
    # looking for npx-cli.js.
    cmd = get_npm_command()
    cmd.append('exec')
    return cmd


def add_node_to_path() -> None:
    """Adds the directory for the standalone node binary to PATH."""
    node_install_dir = os.path.dirname(get_node_path())
    new_path = os.pathsep.join([node_install_dir, os.environ['PATH']])
    os.environ['PATH'] = new_path
