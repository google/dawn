#!/usr/bin/env python3

# Copyright 2023 The Dawn Authors
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

import subprocess
import sys

env_file, bin, rest = sys.argv[1], sys.argv[2], sys.argv[3:]

# Read the environment block from the file. This is stored in the format used
# by CreateProcess. Drop last 2 NULs, one for list terminator, one for
# trailing vs. separator.
env_pairs = open(env_file).read()[:-2].split('\0')
env_dict = dict([item.split('=', 1) for item in env_pairs])

sys.exit(subprocess.call([bin] + rest, env=env_dict, shell=True))
