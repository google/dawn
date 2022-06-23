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


def get_git_hash(dawn_dir):
    try:
        result = subprocess.run([get_git(), "rev-parse", "HEAD"],
                                stdout=subprocess.PIPE,
                                cwd=dawn_dir)
        if result.returncode == 0:
            return result.stdout.decode("utf-8").strip()
    except Exception:
        return ""
    # No hash was available (possibly) because the directory was not a git checkout. Dawn should
    # explicitly handle its absenece and disable features relying on the hash, i.e. caching.
    return ""


def get_git_head(dawn_dir):
    return os.path.join(dawn_dir, ".git", "HEAD")


def git_exists(dawn_dir):
    return os.path.exists(get_git_head(dawn_dir))


def unpack_git_ref(packed, resolved):
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


def get_git_resolved_head(dawn_dir):
    result = subprocess.run(
        [get_git(), "rev-parse", "--symbolic-full-name", "HEAD"],
        stdout=subprocess.PIPE,
        cwd=dawn_dir)
    if result.returncode != 0:
        raise Exception("Failed to execute git rev-parse to resolve git head:", result.stdout)

    resolved = os.path.join(dawn_dir, ".git",
                            result.stdout.decode("utf-8").strip())

    # Check a packed-refs file exists. If so, we need to potentially unpack and include it as a dep.
    packed = os.path.join(dawn_dir, ".git", "packed-refs")
    if os.path.exists(packed) and unpack_git_ref(packed, resolved):
        return [packed, resolved]

    if not os.path.exists(resolved):
        raise Exception("Unable to resolve git HEAD hash file:", resolved)
    return [resolved]


def get_version(args):
    version_file = args.version_file
    if version_file:
        with open(version_file) as f:
            return f.read()
    return get_git_hash(os.path.abspath(args.dawn_dir))


def compute_params(args):
    return {
        "get_version": lambda: get_version(args),
    }


class DawnVersionGenerator(Generator):
    def get_description(self):
        return (
            "Generates version dependent Dawn code. Currently regenerated dependent on the version "
            "header (if available), otherwise tries to use git hash.")

    def add_commandline_arguments(self, parser):
        parser.add_argument(
            "--dawn-dir",
            required=True,
            type=str,
            help="The Dawn root directory path to use",
        )
        parser.add_argument(
            "--version-file",
            required=False,
            type=str,
            help=
            ("Path to one-liner version string file used when git may not be present. "
             "In general the version string is a git hash."))

    def get_dependencies(self, args):
        dawn_dir = os.path.abspath(args.dawn_dir)
        version_file = args.version_file

        if version_file:
            return [version_file]
        if git_exists(dawn_dir):
            try:
                return [get_git_head(dawn_dir)
                        ] + get_git_resolved_head(dawn_dir)
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
