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


def parse_mask(mask):
    if mask:
        return int(mask, 0)
    return 0xffffffff


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
    def __init__(self, name, json_data, mask):
        self.name = Name(name)
        self.devices = []

        mask_num = parse_mask(mask)

        for device in json_data:
            device_num = int(device, 0)

            # Don't allow duplicate entries
            assert device not in self.devices, 'Architecture "{}" contained duplicate deviceID "{}"'.format(
                self.name.get(), device)
            # Ensure that all device IDs don't contain bits outside the mask
            assert device_num & mask_num == device_num, 'Architecture "{}" contained deviceID "{}" which doesn\'t match the given mask of "{}"'.format(
                self.name.get(), device, mask)

            self.devices.append(device)


class DeviceSet:
    def __init__(self, json_data):
        self.mask = None
        self.internal = False

        if 'mask' in json_data:
            self.mask = json_data['mask']

        if 'internal' in json_data:
            self.internal = json_data['internal']

        self.architectures = []
        if 'architecture' in json_data:
            for (arch_name, arch_data) in json_data['architecture'].items():
                # Skip any entries that start with an underscore. Used for comments.
                if arch_name[0] == '_':
                    continue

                architecture = Architecture(arch_name, arch_data, self.mask)

                # Validate that deviceIDs are only allowed to be in one Architecture at a time
                for other_architecture in self.architectures:
                    for device in architecture.devices:
                        assert device not in other_architecture.devices, 'Architectures "{}" and "{}" both contain deviceID "{}"'.format(
                            architecture.name.get(),
                            other_architecture.name.get(), device)

                self.architectures.append(architecture)

    def validate_devices(self, other_devices, other_mask):
        combined_mask = parse_mask(self.mask) & parse_mask(other_mask)

        for other_device in other_devices:
            other_device_num = int(other_device, 0) & combined_mask
            for architecture in self.architectures:
                for device in architecture.devices:
                    device_num = int(device, 0) & combined_mask
                    assert device_num != other_device_num, 'DeviceID "{}" & mask "{}" conflicts with deviceId "{}" & mask "{}" in architecture "{}"'.format(
                        other_device, other_mask, device, self.mask,
                        architecture.name.get())

    def maskDeviceId(self):
        if not self.mask:
            return ''
        return ' & ' + self.mask


class Vendor:
    def __init__(self, name, json_data):
        self.name = Name(name)
        self.id = json_data['id']

        architecture_dict = {}
        internal_architecture_dict = {}

        self.device_sets = []
        if 'devices' in json_data:
            for device_data in json_data['devices']:
                device_set = DeviceSet(device_data)

                for architecture in device_set.architectures:

                    # Validate that deviceIDs are unique across device sets
                    for other_device_set in self.device_sets:
                        # Only validate device IDs between internal and public device sets.
                        if other_device_set.internal == device_set.internal:
                            assert device_set.mask != other_device_set.mask, 'Vendor "{}" contained duplicate device masks "{}"'.format(
                                self.name.get(), device_set.mask)
                            other_device_set.validate_devices(
                                architecture.devices, device_set.mask)

                        # Validate that architecture names are unique between internal and public device sets.
                        else:
                            for other_architecture in other_device_set.architectures:
                                assert architecture.name.canonical_case(
                                ) != other_architecture.name.canonical_case(
                                ), '"{}" is defined as both an internal and public architecture'.format(
                                    architecture.name.get())

                    if device_set.internal:
                        internal_architecture_dict[
                            architecture.name.canonical_case(
                            )] = architecture.name
                    else:
                        architecture_dict[architecture.name.canonical_case(
                        )] = architecture.name

                self.device_sets.append(device_set)

        # List of unique architecture names under this vendor
        self.architecture_names = architecture_dict.values()
        self.internal_architecture_names = internal_architecture_dict.values()


def parse_json(json):
    vendors = []
    internal_architecture_count = 0

    for (vendor_name, vendor_data) in json['vendors'].items():
        # Skip vendors that have a leading underscore. Those are intended to be "comments".
        if vendor_name[0] == '_':
            continue

        vendor = Vendor(vendor_name, vendor_data)
        vendors.append(vendor)
        internal_architecture_count += len(vendor.internal_architecture_names)

    return {
        'vendors': vendors,
        'has_internal': internal_architecture_count > 0
    }


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
