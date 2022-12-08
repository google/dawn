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


def go_path(input_api):
    go = input_api.os_path.join(input_api.change.RepositoryRoot(), "tools",
                                "golang", "bin", "go")
    if input_api.is_windows:
        go += '.exe'

    return go


def RunGoTests(input_api, output_api):
    results = []
    try:
        input_api.subprocess.check_call_out(
            [go_path(input_api), "test", "./..."],
            stdout=input_api.subprocess.PIPE,
            stderr=input_api.subprocess.PIPE,
            cwd=input_api.PresubmitLocalPath())
    except input_api.subprocess.CalledProcessError as e:
        results.append(output_api.PresubmitError('%s' % (e, )))
    return results
