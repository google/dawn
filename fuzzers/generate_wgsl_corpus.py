#!/usr/bin/env python3

# Copyright 2021 The Tint Authors.
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

# Collect all .wgsl files under a given directory and copy them to a given
# corpus directory, flattening their file names by replacing path
# separators with underscores. If the output directory already exists, it
# will be deleted and re-created. Files ending with ".expected.spvasm" are
# skipped.
#
# The intended use of this script is to generate a corpus of WGSL shaders
# for fuzzing.
#
# Usage:
#    generate_wgsl_corpus.py <input_dir> <corpus_dir>


import os
import pathlib
import shutil
import sys


def list_wgsl_files(root_search_dir):
    for root, folders, files in os.walk(root_search_dir):
        for filename in folders + files:
            if pathlib.Path(filename).suffix == '.wgsl':
                yield os.path.join(root, filename)


def main():
    if len(sys.argv) != 3:
        print("Usage: " + sys.argv[0] + " <input dir> <output dir>")
        return 1
    input_dir: str = os.path.abspath(sys.argv[1].rstrip(os.sep))
    corpus_dir: str = os.path.abspath(sys.argv[2])
    if os.path.exists(corpus_dir):
        shutil.rmtree(corpus_dir)
    os.makedirs(corpus_dir)
    for in_file in list_wgsl_files(input_dir):
        if in_file.endswith(".expected.wgsl"):
            continue
        out_file = in_file[len(input_dir) + 1:].replace(os.sep, '_')
        shutil.copy(in_file, corpus_dir + os.sep + out_file)


if __name__ == "__main__":
    sys.exit(main())
