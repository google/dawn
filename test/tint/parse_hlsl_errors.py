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

# Parses expected.*.hlsl files for errors and outputs a report.
#
# Usage:
#    parse_hlsl_erors

import glob
import re
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('--list-files', dest='list_files', action='store_true')
parser.add_argument('--no-list-files', dest='list_files', action='store_false')
parser.set_defaults(list_files=True)
args = parser.parse_args()

def add_error(error_to_file, error, file):
    if not error in error_to_file:
        error_to_file[error] = [file]
    else:
        error_to_file[error].append(file)

def find_and_print_errors(glob_pathname):
    files = glob.glob(glob_pathname, recursive=True)
    error_to_file = {}

    for f in files:
        found_error = False
        with open(f, "r") as fs:
            all_lines = fs.readlines()
            first_line = all_lines[0]
            if not first_line.startswith("SKIP:"):
                continue
            # The most refined errors are printed at the end, so search for error lines from bottom-up
            all_lines.reverse()
            for line in all_lines:
                m = re.search('.*\.hlsl:[0-9]+:.*?(error.*)', line)
                if m:
                    add_error(error_to_file, m.groups()[0], f)
                    found_error = True
                else:
                    m = re.search('error( X[0-9]+)*?:(.*)', line)
                    if m:
                        add_error(error_to_file, m.group(), f)
                        found_error = True
                    else:
                        if "exit status" in line:
                            add_error(error_to_file, line, f)
                            found_error = True
                if found_error:
                    break # Stop on first error string found

        if not found_error:
            # If no error message was found, add the SKIP line as it may contain the reason for skipping
            add_error(error_to_file, first_line.strip(), f)

    for error,files in sorted(error_to_file.items()):
        print('{} (count: {})'.format(error, len(files)))
        if args.list_files:
            for f in files:
                print('\t{}'.format(f))

print("=== FXC ===")
find_and_print_errors('./**/*.fxc.hlsl')

print("=== DXC ===")
find_and_print_errors('./**/*.dxc.hlsl')
