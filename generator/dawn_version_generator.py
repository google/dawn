#!/usr/bin/env python3
# Copyright 2022 The Dawn Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os, subprocess, sys, shutil

from generator_lib import Generator, run_generator, FileRender

def get_git():
    # Will find git, git.exe, git.bat...
    git_exec = shutil.which("git")
    if not git_exec:
        raise Exception("No git executable found")

    return git_exec


def get_gitHash(dawnDir):
    try:
        result = subprocess.run([get_git(), "rev-parse", "HEAD"],
                                stdout=subprocess.PIPE,
                                cwd=dawnDir)
        if result.returncode == 0:
            return result.stdout.decode("utf-8").strip()
    except Exception:
        return ""
    # No hash was available (possibly) because the directory was not a git checkout. Dawn should
    # explicitly handle its absenece and disable features relying on the hash, i.e. caching.
    return ""


def get_gitHead(dawnDir):
    return os.path.join(dawnDir, ".git", "HEAD")


def gitExists(dawnDir):
    return os.path.exists(get_gitHead(dawnDir))


def unpackGitRef(packed, resolved):
    with open(packed) as fin:
        refs = fin.read().strip().split("\n")

    # Strip comments
    refs = [ref.split(" ") for ref in refs if ref.strip()[0] != "#"]

    # Parse results which are in the format [<gitHash>, <refFile>] from previous step.
    refs = [gitHash for (gitHash, refFile) in refs if refFile == resolved]
    if len(refs) == 1:
        with open(resolved, "w") as fout:
            fout.write(refs[0] + "\n")
        return True
    return False


def get_gitResolvedHead(dawnDir):
    result = subprocess.run(
        [get_git(), "rev-parse", "--symbolic-full-name", "HEAD"],
        stdout=subprocess.PIPE,
        cwd=dawnDir)
    if result.returncode != 0:
        raise Exception("Failed to execute git rev-parse to resolve git head:", result.stdout)

    resolved = os.path.join(dawnDir, ".git",
                            result.stdout.decode("utf-8").strip())

    # Check a packed-refs file exists. If so, we need to potentially unpack and include it as a dep.
    packed = os.path.join(dawnDir, ".git", "packed-refs")
    if os.path.exists(packed) and unpackGitRef(packed, resolved):
        return [packed, resolved]

    if not os.path.exists(resolved):
        raise Exception("Unable to resolve git HEAD hash file:", resolved)
    return [resolved]


def compute_params(args):
    return {
        "get_gitHash": lambda: get_gitHash(os.path.abspath(args.dawn_dir)),
    }


class DawnVersionGenerator(Generator):
    def get_description(self):
        return "Generates version dependent Dawn code. Currently regenerated dependent on git hash."

    def add_commandline_arguments(self, parser):
        parser.add_argument(
            "--dawn-dir",
            required=True,
            type=str,
            help="The Dawn root directory path to use",
        )

    def get_dependencies(self, args):
        dawnDir = os.path.abspath(args.dawn_dir)
        if gitExists(dawnDir):
            try:
                return [get_gitHead(dawnDir)] + get_gitResolvedHead(dawnDir)
            except Exception:
                return []
        return []

    def get_file_renders(self, args):
        params = compute_params(args)

        return [
            FileRender("dawn/common/Version.h",
                       "src/dawn/common/Version_autogen.h", [params]),
        ]


if __name__ == "__main__":
    sys.exit(run_generator(DawnVersionGenerator()))
