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
"""Script for easily adding expectations to expectations.txt

Converts one or more WebGPU CTS queries into one or more individual expectations
and appends them to the end of the file.
"""

import argparse
import logging
import os
import subprocess
import sys

import dir_paths

LIST_SCRIPT_PATH = os.path.join(dir_paths.webgpu_cts_scripts_dir, 'list.py')
TRANSPILE_DIR = os.path.join(dir_paths.dawn_dir, '.node_transpile_work_dir')
EXPECTATION_FILE_PATH = os.path.join(dir_paths.dawn_dir, 'webgpu-cts',
                                     'expectations.txt')


def expand_query(query):
    cmd = [
        sys.executable,
        LIST_SCRIPT_PATH,
        '--js-out-dir',
        TRANSPILE_DIR,
        '--query',
        query,
    ]
    p = subprocess.run(cmd, stdout=subprocess.PIPE, check=True)
    return p.stdout.decode('utf-8').splitlines()


def generate_expectations(queries, tags, results, bug):
    tags = '[ %s ] ' % ' '.join(tags) if tags else ''
    results = ' [ %s ]' % ' '.join(results)
    bug = bug + ' ' if bug else ''
    content = ''
    for q in queries:
        test_names = expand_query(q)
        if not test_names:
            logging.warning('Did not get any test names for query %s', q)
        for tn in test_names:
            content += '{bug}{tags}{test}{results}\n'.format(bug=bug,
                                                             tags=tags,
                                                             test=tn,
                                                             results=results)
    with open(EXPECTATION_FILE_PATH, 'a') as outfile:
        outfile.write(content)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description=('Converts one or more WebGPU CTS queries into one or '
                     'more individual expectations and appends them to the '
                     'end of expectations.txt'))
    parser.add_argument('-b',
                        '--bug',
                        help='The bug link to associate with the expectations')
    parser.add_argument('-t',
                        '--tag',
                        action='append',
                        default=[],
                        dest='tags',
                        help=('A tag to restrict the expectation to. Can be '
                              'specified multiple times.'))
    parser.add_argument('-r',
                        '--result',
                        action='append',
                        default=[],
                        dest='results',
                        required=True,
                        help=('An expected result for the expectation. Can be '
                              'specified multiple times, although a single '
                              'result is the most common usage.'))
    parser.add_argument('-q',
                        '--query',
                        action='append',
                        default=[],
                        dest='queries',
                        help=('A CTS query to expand into expectations. Can '
                              'be specified multiple times.'))
    args = parser.parse_args()
    generate_expectations(args.queries, args.tags, args.results, args.bug)
