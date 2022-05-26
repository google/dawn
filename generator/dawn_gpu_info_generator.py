#!/usr/bin/env python3
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

import json, os, sys
from collections import namedtuple

from generator_lib import Generator, run_generator, FileRender


class Name:
    def __init__(self, name):
        self.name = name
        self.chunks = name.split(' ')

    def get(self):
        return self.name

    def CamelChunk(self, chunk):
        return chunk[0].upper() + chunk[1:]

    def canonical_case(self):
        return (' '.join(self.chunks)).lower()

    def concatcase(self):
        return ''.join(self.chunks)

    def camelCase(self):
        return self.chunks[0] + ''.join(
            [self.CamelChunk(chunk) for chunk in self.chunks[1:]])

    def CamelCase(self):
        return ''.join([self.CamelChunk(chunk) for chunk in self.chunks])

    def SNAKE_CASE(self):
        return '_'.join([chunk.upper() for chunk in self.chunks])

    def snake_case(self):
        return '_'.join(self.chunks)

    def js_enum_case(self):
        result = self.chunks[0].lower()
        for chunk in self.chunks[1:]:
            if not result[-1].isdigit():
                result += '-'
            result += chunk.lower()
        return result


class Architecture:
    def __init__(self, name, json_data):
        self.name = Name(name)
        self.devices = []
        for device in json_data:
            self.devices.append(device)


class Vendor:
    def __init__(self, name, json_data):
        self.name = Name(name)
        self.id = json_data['id']

        self.deviceMask = None
        if 'deviceMask' in json_data:
            self.deviceMask = json_data['deviceMask']

        self.architectures = []

        if 'architecture' in json_data:
            for (arch_name, arch_data) in json_data['architecture'].items():
                # Skip any entries that start with an underscore. Used for comments.
                if arch_name[0] == '_':
                    continue

                self.architectures.append(Architecture(arch_name, arch_data))

    def maskDeviceId(self):
        if not self.deviceMask:
            return ''
        return ' & ' + self.deviceMask


def parse_json(json):
    vendors = []

    for (vendor, vendor_data) in json['vendors'].items():
        vendors.append(Vendor(vendor, vendor_data))

    return {'vendors': vendors}


class DawnGpuInfoGenerator(Generator):
    def get_description(self):
        return "Generates GPU Info Dawn code."

    def add_commandline_arguments(self, parser):
        parser.add_argument('--gpu-info-json',
                            required=True,
                            type=str,
                            help='The GPU Info JSON definition to use.')

    def get_dependencies(self, args):
        return [os.path.abspath(args.gpu_info_json)]

    def get_file_renders(self, args):
        with open(args.gpu_info_json) as f:
            loaded_json = json.loads(f.read())

        params = parse_json(loaded_json)

        return [
            FileRender("dawn/common/GPUInfo.h",
                       "src/dawn/common/GPUInfo_autogen.h", [params]),
            FileRender("dawn/common/GPUInfo.cpp",
                       "src/dawn/common/GPUInfo_autogen.cpp", [params]),
        ]


if __name__ == "__main__":
    sys.exit(run_generator(DawnGpuInfoGenerator()))
