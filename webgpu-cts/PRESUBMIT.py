# Copyright 2022 The Dawn & Tint Authors
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

import sys

USE_PYTHON3 = True


def _DoCommonChecks(input_api, output_api):
    sys.path += [input_api.change.RepositoryRoot()]

    from go_presubmit_support import go_path

    results = []
    try:
        tools_path = input_api.os_path.join(input_api.change.RepositoryRoot(),
                                            'tools')
        cts_bin = input_api.os_path.join(tools_path, 'bin', 'cts')
        if input_api.is_windows:
            cts_bin += '.exe'

        cmd = [
            go_path(input_api), 'build', '-o', cts_bin,
            input_api.os_path.join('.', 'cmd', 'cts')
        ]
        input_api.subprocess.check_call_out(cmd,
                                            stdout=input_api.subprocess.PIPE,
                                            stderr=input_api.subprocess.PIPE,
                                            cwd=input_api.os_path.join(
                                                tools_path, 'src'))

        cmd = [cts_bin, 'validate']
        input_api.subprocess.check_call_out(
            cmd,
            stdout=input_api.subprocess.PIPE,
            stderr=input_api.subprocess.PIPE,
            cwd=input_api.change.RepositoryRoot())
    except input_api.subprocess.CalledProcessError as e:
        results.append(output_api.PresubmitError('%s' % (e, )))
    return results


CheckChangeOnUpload = _DoCommonChecks
CheckChangeOnCommit = _DoCommonChecks
