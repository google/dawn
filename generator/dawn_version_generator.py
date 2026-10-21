#!/usr/bin/env python3
# Copyright 2022 The Dawn & Tint Authors
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

import argparse, json, os, subprocess, sys, shutil

from dawn_json_parsing import parse_addin, parse_json
from generator_lib import Generator, run_generator, FileRender, GeneratorOutput


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
        raise Exception("Failed to execute git rev-parse to resolve git head:",
                        result.stdout)

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


def get_version_byte_array(version_str):
    version_str = version_str.strip()
    if not version_str:
        # If the version string is empty, generate an array of 20 zero bytes.
        return ", ".join(["0x00"] * 20)
    assert len(
        version_str
    ) == 40, "Version string must be a 40-character git hash if not empty."
    byte_values = [
        f"0x{version_str[i:i+2]}" for i in range(0, len(version_str), 2)
    ]
    return ", ".join(byte_values)


def _get_stable_versions(version_addins):
    versions = [
        v["stable_version"] for v in version_addins.values()
        if "stable_version" in v
    ]
    return len(versions), max(versions)


# Hard-coded to 1 for now since we don't anticipate needing to ever increment
# the major version (i.e. Dawn will always be backwards compatible), but we
# introduce the major version nonetheless to provide a mechanism to make
# breaking changes if it's ever needed.
def get_major_version():
    return 1


def get_minor_version(json_filepath):
    return _get_stable_versions(parse_addin(json_filepath))[1]


def compute_params(args):
    version = get_version(args)
    version_json = os.path.abspath(args.version_json)
    return {
        "get_version_byte_array": lambda: get_version_byte_array(version),
        "get_major_version": get_major_version,
        "get_minor_version": lambda: get_minor_version(version_json),
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
        parser.add_argument(
            "--version-json",
            required=True,
            type=str,
            help="The DAWN VERSION JSON definition to use.",
        )

    def get_dependencies(self, args):
        dawn_dir = os.path.abspath(args.dawn_dir)
        version_file = args.version_file

        deps = [os.path.abspath(args.version_json)]
        if version_file:
            return deps + [version_file]
        if git_exists(dawn_dir):
            try:
                return deps + [get_git_head(dawn_dir)
                               ] + get_git_resolved_head(dawn_dir)
            except Exception:
                return deps
        return deps

    def get_outputs(self, args):
        params = compute_params(args)

        renders = [
            FileRender("dawn/dawn_version.h", "include/dawn/dawn_version.h",
                       [params]),
        ]
        return GeneratorOutput(renders=renders, imported_templates=[])


def run_check(args):
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--dawn-json",
        required=True,
        type=str,
        help="The DAWN JSON definition to use.",
    )
    parser.add_argument(
        "--old-version-json",
        required=True,
        type=str,
        help="The old dawn_version.json file path before changes.",
    )
    parser.add_argument(
        "--new-version-json",
        required=True,
        type=str,
        help="The new dawn_version.json file path after changes.",
    )
    options = parser.parse_args(args)

    with open(options.dawn_json) as f:
        dawn_json = json.loads(f.read())
    new_addins = parse_addin(options.new_version_json)
    old_addins = parse_addin(options.old_version_json)

    # Verify that the new addins file doesn't contain any mismatched entries
    # with respect to dawn.json.
    parse_json(dawn_json, enabled_tags=["dawn", "native"], addins=[new_addins])

    # TODO(crbug.com/575526443): Currently, we are just using heuristic checks
    # on the number of entries and the max version number. We can probably do
    # a dictionary diff on the addins once we have the infrastructure in place.
    old_count, old_max = _get_stable_versions(old_addins)
    new_count, new_max = _get_stable_versions(new_addins)

    if new_count < old_count:
        print(f"The number of 'stable_version' entries in dawn_version.json "
              f"decreased from {old_count} to {new_count}. Stable API entries "
              f"cannot be removed.")
        return 1

    if new_count == old_count and new_max != old_max:
        print(
            f"The number of 'stable_version' entries in dawn_version.json did "
            f"not change ({new_count}), but the maximum 'stable_version' "
            f"changed from {old_max} to {new_max}.")
        return 1

    if new_count > old_count and new_max <= old_max:
        print(f"The number of 'stable_version' entries in dawn_version.json "
              f"increased from {old_count} to {new_count}, but the maximum "
              f"'stable_version' ({new_max}) did not increase from {old_max}.")
        return 1

    return 0


def main(args):
    parser = argparse.ArgumentParser()
    parser.add_argument("action", choices=("gen", "check"))
    options, remaining_args = parser.parse_known_args(args[1:])

    if options.action == "gen":
        return run_generator(DawnVersionGenerator(), remaining_args)

    if options.action == "check":
        return run_check(remaining_args)


if __name__ == "__main__":
    sys.exit(main(sys.argv))
