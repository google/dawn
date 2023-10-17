#!/usr/bin/env python3

# Copyright 2021 The Dawn & Tint Authors
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

# Collect all .spvasm files under a given directory, assemble them using
# spirv-as, and emit the assembled binaries to a given corpus directory,
# flattening their file names by replacing path separators with underscores.
# If the output directory already exists, it will be deleted and re-created.
# Files ending with ".expected.spvasm" are skipped.
#
# The intended use of this script is to generate a corpus of SPIR-V
# binaries for fuzzing.
#
# Usage:
#    generate_spirv_corpus.py <input_dir> <corpus_dir> <path to spirv-as>

import os
import pathlib
import shutil
import subprocess
import sys


def list_spvasm_files(root_search_dir):
    for root, folders, files in os.walk(root_search_dir):
        for filename in folders + files:
            if pathlib.Path(filename).suffix == ".spvasm":
                yield os.path.join(root, filename)


def main():
    if len(sys.argv) != 4:
        print("Usage: " + sys.argv[0] +
              " <input dir> <output dir> <spirv-as path>")
        return 1
    input_dir: str = os.path.abspath(sys.argv[1].rstrip(os.sep))
    corpus_dir: str = os.path.abspath(sys.argv[2])
    spirv_as_path: str = os.path.abspath(sys.argv[3])
    if os.path.exists(corpus_dir):
        shutil.rmtree(corpus_dir)
    os.makedirs(corpus_dir)

    # It might be that some of the attempts to convert SPIR-V assembly shaders
    # into SPIR-V binaries go wrong. It is sensible to tolerate a small number
    # of such errors, to avoid fuzzer preparation failing due to bugs in
    # spirv-as. But it is important to know when a large number of failures
    # occur, in case something is more deeply wrong.
    num_errors = 0
    max_tolerated_errors = 10
    logged_errors = ""

    for in_file in list_spvasm_files(input_dir):
        if ".expected." in in_file:
            continue
        out_file = os.path.splitext(
            corpus_dir + os.sep +
            in_file[len(input_dir) + 1:].replace(os.sep, '_'))[0] + ".spv"
        cmd = [
            spirv_as_path, "--target-env", "spv1.3", in_file, "-o", out_file
        ]
        proc = subprocess.Popen(cmd,
                                stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE)
        stdout, stderr = proc.communicate()
        if proc.returncode != 0:
            num_errors += 1
            logged_errors += "Error running " + " ".join(
                cmd) + ": " + stdout.decode('utf-8') + stderr.decode('utf-8')

    if num_errors > max_tolerated_errors:
        print("Too many (" + str(num_errors) +
              ") errors occurred while generating the SPIR-V corpus.")
        print(logged_errors)
        return 1


if __name__ == "__main__":
    sys.exit(main())
