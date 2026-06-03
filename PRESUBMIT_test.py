#!/usr/bin/env python3
# Copyright 2026 The Dawn & Tint Authors
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

import os
import sys
import unittest

# Import PRESUBMIT first before depot_tools modifies sys.path
import PRESUBMIT

# Add depot_tools to path to import mocks
sys.path.append(
    os.path.join(os.path.dirname(__file__), 'third_party', 'depot_tools'))

from testing_support.presubmit_canned_checks_test_mocks import (
    MockAffectedFile, MockInputApi, MockOutputApi)


class CheckChangeTodoHasOwnerTest(unittest.TestCase):

    def _create_mock_input_api(self):
        mock_input_api = MockInputApi()

        # Mock AffectedFiles to support file_filter
        def mock_affected_files(include_dirs=False,
                                include_deletes=True,
                                file_filter=None):
            if file_filter:
                return [f for f in mock_input_api.files if file_filter(f)]
            return mock_input_api.files

        mock_input_api.change.AffectedFiles = mock_affected_files
        return mock_input_api

    def testNoTodo(self):
        mock_input_api = self._create_mock_input_api()
        mock_input_api.files = [
            MockAffectedFile('src/dawn/Foo.cpp', [
                'void Foo() {',
                '    int x = 0;',
                '}',
            ])
        ]
        errors = PRESUBMIT.CheckChangeTodoHasOwner(mock_input_api,
                                                   MockOutputApi())
        self.assertEqual(0, len(errors))

    def testValidTodo(self):
        mock_input_api = self._create_mock_input_api()
        mock_input_api.files = [
            MockAffectedFile('src/dawn/Foo.cpp', [
                '// TODO(crbug.com/123): fix this',
                '// TODO: owner - fix this',
            ])
        ]
        errors = PRESUBMIT.CheckChangeTodoHasOwner(mock_input_api,
                                                   MockOutputApi())
        self.assertEqual(0, len(errors))

    def testInvalidTodo(self):
        mock_input_api = self._create_mock_input_api()
        mock_input_api.files = [
            MockAffectedFile('src/dawn/Foo.cpp', [
                '// TODO: fix this',
            ])
        ]
        errors = PRESUBMIT.CheckChangeTodoHasOwner(mock_input_api,
                                                   MockOutputApi())
        self.assertEqual(1, len(errors))
        self.assertIn('Found TODO with no issue number', errors[0].message)

    def testDawnUnsafeTodoIgnored(self):
        mock_input_api = self._create_mock_input_api()
        mock_input_api.files = [
            MockAffectedFile('src/dawn/Foo.cpp', [
                'DAWN_UNSAFE_TODO(ptr[0] = 0);',
            ])
        ]
        errors = PRESUBMIT.CheckChangeTodoHasOwner(mock_input_api,
                                                   MockOutputApi())
        self.assertEqual(0, len(errors))

    def testTodoAndDawnUnsafeTodo(self):
        mock_input_api = self._create_mock_input_api()
        mock_input_api.files = [
            MockAffectedFile('src/dawn/Foo.cpp', [
                'DAWN_UNSAFE_TODO(ptr[0] = 0); // TODO: fix this',
            ])
        ]
        errors = PRESUBMIT.CheckChangeTodoHasOwner(mock_input_api,
                                                   MockOutputApi())
        self.assertEqual(1, len(errors))
        self.assertIn('Found TODO with no issue number', errors[0].message)

    def testMixedTodos(self):
        mock_input_api = self._create_mock_input_api()
        mock_input_api.files = [
            MockAffectedFile('src/dawn/Foo.cpp', [
                '// TODO: fix this',
                'DAWN_UNSAFE_TODO(ptr[0] = 0);',
            ])
        ]
        errors = PRESUBMIT.CheckChangeTodoHasOwner(mock_input_api,
                                                   MockOutputApi())
        self.assertEqual(1, len(errors))
        self.assertIn('src/dawn/Foo.cpp:1', errors[0].message)
        self.assertNotIn('src/dawn/Foo.cpp:2', errors[0].message)

    def testPresubmitFilesIgnored(self):
        mock_input_api = self._create_mock_input_api()
        mock_input_api.files = [
            MockAffectedFile('PRESUBMIT.py', [
                '// TODO: fix this',
                'DAWN_UNSAFE_TODO(ptr[0] = 0);',
            ]),
            MockAffectedFile('PRESUBMIT_test.py', [
                '// TODO: fix this',
                'DAWN_UNSAFE_TODO(ptr[0] = 0);',
            ])
        ]
        errors = PRESUBMIT.CheckChangeTodoHasOwner(mock_input_api,
                                                   MockOutputApi())
        self.assertEqual(0, len(errors))


if __name__ == '__main__':
    unittest.main()
