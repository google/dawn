#!/usr/bin/env python
# Copyright 2020 The Tint Authors.
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

# Test runner for executing a test of tests with Tint. The runner will
# find all .wgsl files in the given folder and attempt to convert them
# to each of the backend formats. If the file contains a '.fail.' in the
# name then the runner will expect the file to fail conversion.

import base64
import copy
import difflib
import optparse
import os
import platform
import re
import subprocess
import sys
import tempfile


"""
A single test case to be executed. Stores the path to the test file
and the result of executing the test.
"""


class TestCase:
    def __init__(self, input_path, parse_only):
        self.input_path = input_path
        self.parse_only = parse_only
        self.results = {}

    def IsExpectedFail(self):
        fail_re = re.compile('^.+[\.]fail[\.]wgsl')
        return fail_re.match(self.GetInputPath())

    def IsParseOnly(self):
        return self.parse_only

    def GetInputPath(self):
        return self.input_path

    def GetResult(self, fmt):
        return self.results[fmt]


"""
The test runner, will execute a series of test cases and record the
results.
"""


class TestRunner:
    def RunTest(self, tc):
        """Runs a single test."""
        print("Testing {}".format(tc.GetInputPath()))

        cmd = [self.options.test_prog_path]
        if tc.IsParseOnly():
            cmd += ['--parse-only']

        languages = ["wgsl", "spvasm", "msl", "hlsl"]
        try:
            for lang in languages:
                lang_cmd = copy.copy(cmd)
                lang_cmd += ['--format', lang]
                lang_cmd += [tc.GetInputPath()]
                err = subprocess.check_output(lang_cmd,
                                              stderr=subprocess.STDOUT)

        except Exception as e:
            if not tc.IsExpectedFail():
                print("{}".format("".join(map(chr, bytearray(e.output)))))
                print(e)
            return False

        return True

    def RunTests(self):
        """Runs a set of test cases"""
        for tc in self.test_cases:
            result = self.RunTest(tc)

            if not tc.IsExpectedFail() and not result:
                self.failures.append(tc.GetInputPath())
            elif tc.IsExpectedFail() and result:
                print("Expected: " + tc.GetInputPath() +
                      " to fail but passed.")
                self.failures.append(tc.GetInputPath())

    def SummarizeResults(self):
        """Prints a summarization of the test results to STDOUT"""

        if len(self.failures) > 0:
            self.failures.sort()

            print('\nSummary of Failures:')
            for failure in self.failures:
                print(failure)

        print('')
        print('Test cases executed: {}'.format(len(self.test_cases)))
        print('  Successes:  {}'.format(
            (len(self.test_cases) - len(self.failures))))
        print('  Failures:   {}'.format(len(self.failures)))
        print('')

    def Run(self):
        """Executes the test runner."""
        base_path = os.path.abspath(
            os.path.join(os.path.dirname(__file__), '..'))

        usage = 'usage: %prog [options] (file)'
        parser = optparse.OptionParser(usage=usage)
        parser.add_option('--build-dir',
                          default=os.path.join(base_path, 'out', 'Debug'),
                          help='path to build directory')
        parser.add_option('--test-dir',
                          default=os.path.join(os.path.dirname(__file__), '..',
                                               'third_party', 'gpuweb-cts',
                                               'src', 'webgpu', 'shader',
                                               'validation', 'wgsl'),
                          help='path to directory containing test files')
        parser.add_option(
            '--test-prog-path',
            default=None,
            help='path to program to test (default build-dir/tint)')
        parser.add_option('--parse-only',
                          action="store_true",
                          default=False,
                          help='only parse test cases; do not compile')

        self.options, self.args = parser.parse_args()

        if self.options.test_prog_path == None:
            test_prog = os.path.abspath(
                os.path.join(self.options.build_dir, 'tint'))
            if not os.path.isfile(test_prog):
                print("Cannot find test program {}".format(test_prog))
                return 1

            self.options.test_prog_path = test_prog

        if not os.path.isfile(self.options.test_prog_path):
            print("Cannot find test program '{}'".format(
                self.options.test_prog_path))
            return 1

        input_file_re = re.compile('^.+[\.]wgsl')
        self.test_cases = []

        if self.args:
            for filename in self.args:
                input_path = os.path.join(self.options.test_dir, filename)
                if not os.path.isfile(input_path):
                    print("Cannot find test file '{}'".format(filename))
                    return 1

                self.test_cases.append(
                    TestCase(input_path, self.options.parse_only))

        else:
            for file_dir, _, filename_list in os.walk(self.options.test_dir):
                for input_filename in filename_list:
                    if input_file_re.match(input_filename):
                        input_path = os.path.join(file_dir, input_filename)
                        if os.path.isfile(input_path):
                            self.test_cases.append(
                                TestCase(input_path, self.options.parse_only))

        self.failures = []

        self.RunTests()
        self.SummarizeResults()

        return len(self.failures)


def main():
    runner = TestRunner()
    return runner.Run()


if __name__ == '__main__':
    sys.exit(main())
