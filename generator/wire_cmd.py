# Copyright 2019 The Dawn Authors
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

from collections import namedtuple
from common import Name
import common

def concat_names(*names):
    return ' '.join([name.canonical_case() for name in names])

# Create wire commands from api methods
def compute_wire_params(api_params, wire_json):
    wire_params = api_params.copy()
    types = wire_params['types']

    commands = []
    return_commands = []

    # Generate commands from object methods
    for api_object in wire_params['by_category']['object']:
        for method in api_object.methods:
            command_name = concat_names(api_object.name, method.name)
            command_suffix = Name(command_name).CamelCase()

            # Only object return values or void are supported. Other methods must be handwritten.
            if method.return_type.category != 'object' and method.return_type.name.canonical_case() != 'void':
                assert(command_suffix in wire_json['special items']['client_handwritten_commands'])
                continue

            if command_suffix in wire_json['special items']['client_side_commands']:
                continue

            # Create object method commands by prepending "self"
            members = [common.RecordMember(Name('self'), types[api_object.dict_name], 'value', False, False)]
            members += method.arguments

            # Client->Server commands that return an object return the result object handle
            if method.return_type.category == 'object':
                result = common.RecordMember(Name('result'), types['ObjectHandle'], 'value', False, True)
                result.set_handle_type(method.return_type)
                members.append(result)

            command = common.Command(command_name, members)
            command.derived_object = api_object
            command.derived_method = method
            commands.append(command)

    for (name, json_data) in wire_json['commands'].items():
        commands.append(common.Command(name, common.linked_record_members(json_data, types)))

    for (name, json_data) in wire_json['return commands'].items():
        return_commands.append(common.Command(name, common.linked_record_members(json_data, types)))

    wire_params['cmd_records'] = {
        'command': commands,
        'return command': return_commands
    }

    for commands in wire_params['cmd_records'].values():
        for command in commands:
            command.update_metadata()
        commands.sort(key=lambda c: c.name.canonical_case())

    wire_params.update(wire_json.get('special items', {}))

    return wire_params
