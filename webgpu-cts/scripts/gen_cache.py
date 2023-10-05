#!/usr/bin/env python3
#
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

import argparse
import tarfile
import datetime
import os
import sys


def gen_cache(out_dir):
    script_directory = os.path.dirname(os.path.abspath(sys.argv[0]))
    tar = tarfile.open(os.path.join(script_directory, '../cache.tar.gz'))
    tar.extractall(out_dir)
    # Update timestamps
    now = datetime.datetime.now().timestamp()
    for name in tar.getnames():
        path = os.path.join(out_dir, name)
        os.utime(path, (now, now))
    tar.close()


# Extract the cache for CTS runs.
if __name__ == '__main__':
    parser = argparse.ArgumentParser()

    # TODO(bclayton): Unused. Remove
    parser.add_argument('js_script', help='Path to gen_cache.js')

    parser.add_argument('out_dir', help='Output directory for the cache')
    args = parser.parse_args()

    gen_cache(args.out_dir)
