# Copyright 2017 The Dawn Authors
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

class Name:
    def __init__(self, name, native=False):
        self.native = native
        if native:
            self.chunks = [name]
        else:
            self.chunks = name.split(' ')

    def CamelChunk(self, chunk):
        return chunk[0].upper() + chunk[1:]

    def canonical_case(self):
        return (' '.join(self.chunks)).lower()

    def concatcase(self):
        return ''.join(self.chunks)

    def camelCase(self):
        return self.chunks[0] + ''.join([self.CamelChunk(chunk) for chunk in self.chunks[1:]])

    def CamelCase(self):
        return ''.join([self.CamelChunk(chunk) for chunk in self.chunks])

    def SNAKE_CASE(self):
        return '_'.join([chunk.upper() for chunk in self.chunks])

    def snake_case(self):
        return '_'.join(self.chunks)

class Type:
    def __init__(self, name, json_data, native=False):
        self.json_data = json_data
        self.dict_name = name
        self.name = Name(name, native=native)
        self.category = json_data['category']

EnumValue = namedtuple('EnumValue', ['name', 'value'])
class EnumType(Type):
    def __init__(self, name, json_data):
        Type.__init__(self, name, json_data)
        self.values = [EnumValue(Name(m['name']), m['value']) for m in self.json_data['values']]

BitmaskValue = namedtuple('BitmaskValue', ['name', 'value'])
class BitmaskType(Type):
    def __init__(self, name, json_data):
        Type.__init__(self, name, json_data)
        self.values = [BitmaskValue(Name(m['name']), m['value']) for m in self.json_data['values']]
        self.full_mask = 0
        for value in self.values:
            self.full_mask = self.full_mask | value.value

class NativeType(Type):
    def __init__(self, name, json_data):
        Type.__init__(self, name, json_data, native=True)

class NativelyDefined(Type):
    def __init__(self, name, json_data):
        Type.__init__(self, name, json_data)

# Methods and structures are both "records", so record members correspond to
# method arguments or structure members.
class RecordMember:
    def __init__(self, name, typ, annotation, optional, is_return_value):
        self.name = name
        self.type = typ
        self.annotation = annotation
        self.length = None
        self.optional = optional
        self.is_return_value = is_return_value
        self.handle_type = None

    def set_handle_type(self, handle_type):
        assert self.type.dict_name == "ObjectHandle"
        self.handle_type = handle_type

Method = namedtuple('Method', ['name', 'return_type', 'arguments'])
class ObjectType(Type):
    def __init__(self, name, json_data):
        Type.__init__(self, name, json_data)
        self.methods = []
        self.native_methods = []
        self.built_type = None

class Record:
    def __init__(self, name):
        self.name = Name(name)
        self.members = []
        self.has_dawn_object = False

    def update_metadata(self):
        def has_dawn_object(member):
            if isinstance(member.type, ObjectType):
                return True
            elif isinstance(member.type, StructureType):
                return member.type.has_dawn_object
            else:
                return False

        self.has_dawn_object = any(has_dawn_object(member) for member in self.members)

class StructureType(Record, Type):
    def __init__(self, name, json_data):
        Record.__init__(self, name)
        Type.__init__(self, name, json_data)
        self.extensible = json_data.get("extensible", False)

class Command(Record):
    def __init__(self, name, members=None):
        Record.__init__(self, name)
        self.members = members or []
        self.derived_object = None
        self.derived_method = None

def linked_record_members(json_data, types):
    members = []
    members_by_name = {}
    for m in json_data:
        member = RecordMember(Name(m['name']), types[m['type']],
                              m.get('annotation', 'value'), m.get('optional', False),
                              m.get('is_return_value', False))
        handle_type = m.get('handle_type')
        if handle_type:
            member.set_handle_type(types[handle_type])
        members.append(member)
        members_by_name[member.name.canonical_case()] = member

    for (member, m) in zip(members, json_data):
        if member.annotation != 'value':
            if not 'length' in m:
                if member.type.category != 'object':
                    member.length = "constant"
                    member.constant_length = 1
                else:
                    assert(False)
            elif m['length'] == 'strlen':
                member.length = 'strlen'
            else:
                member.length = members_by_name[m['length']]

    return members
