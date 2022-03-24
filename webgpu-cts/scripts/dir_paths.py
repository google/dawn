#!/usr/bin/env python
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

import os

webgpu_cts_scripts_dir = os.path.dirname(os.path.abspath(__file__))
dawn_dir = os.path.dirname(os.path.dirname(webgpu_cts_scripts_dir))
dawn_third_party_dir = os.path.join(dawn_dir, 'third_party')
gn_webgpu_cts_dir = os.path.join(dawn_third_party_dir, 'gn', 'webgpu-cts')
webgpu_cts_root_dir = os.path.join(dawn_third_party_dir, 'webgpu-cts')
chromium_third_party_dir = None
node_dir = None

_possible_chromium_third_party_dir = os.path.dirname(
    os.path.dirname(dawn_third_party_dir))
_possible_node_dir = os.path.join(_possible_chromium_third_party_dir, 'node')
if os.path.exists(_possible_node_dir):
    chromium_third_party_dir = _possible_chromium_third_party_dir
    node_dir = _possible_node_dir
