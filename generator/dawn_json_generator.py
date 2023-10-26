#!/usr/bin/env python3
# Copyright 2017 The Dawn & Tint Authors
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

import json, os, sys
from collections import namedtuple

from generator_lib import Generator, run_generator, FileRender

############################################################
# OBJECT MODEL
############################################################


class Metadata:
    def __init__(self, metadata):
        self.api = metadata['api']
        self.namespace = metadata['namespace']
        self.c_prefix = metadata.get('c_prefix', self.namespace.upper())
        self.proc_table_prefix = metadata['proc_table_prefix']
        self.impl_dir = metadata.get('impl_dir', '')
        self.native_namespace = metadata['native_namespace']
        self.copyright_year = metadata.get('copyright_year', None)


class Name:
    def __init__(self, name, native=False):
        self.native = native
        self.name = name
        if native:
            self.chunks = [name]
        else:
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

    def namespace_case(self):
        return '::'.join(self.chunks)

    def Dirs(self):
        return '/'.join(self.chunks)

    def js_enum_case(self):
        result = self.chunks[0].lower()
        for chunk in self.chunks[1:]:
            if not result[-1].isdigit():
                result += '-'
            result += chunk.lower()
        return result


def concat_names(*names):
    return ' '.join([name.canonical_case() for name in names])


class Type:
    def __init__(self, name, json_data, native=False):
        self.json_data = json_data
        self.dict_name = name
        self.name = Name(name, native=native)
        self.category = json_data['category']
        self.is_wire_transparent = False


EnumValue = namedtuple('EnumValue', ['name', 'value', 'valid', 'json_data'])


class EnumType(Type):
    def __init__(self, is_enabled, name, json_data):
        Type.__init__(self, name, json_data)

        self.values = []
        self.contiguousFromZero = True
        lastValue = -1
        for m in self.json_data['values']:
            if not is_enabled(m):
                continue
            value = m['value']
            if value != lastValue + 1:
                self.contiguousFromZero = False
            lastValue = value
            self.values.append(
                EnumValue(Name(m['name']), value, m.get('valid', True), m))

        # Assert that all values are unique in enums
        all_values = set()
        for value in self.values:
            if value.value in all_values:
                raise Exception("Duplicate value {} in enum {}".format(
                    value.value, name))
            all_values.add(value.value)
        self.is_wire_transparent = True


BitmaskValue = namedtuple('BitmaskValue', ['name', 'value', 'json_data'])


class BitmaskType(Type):
    def __init__(self, is_enabled, name, json_data):
        Type.__init__(self, name, json_data)
        self.values = [
            BitmaskValue(Name(m['name']), m['value'], m)
            for m in self.json_data['values'] if is_enabled(m)
        ]
        self.full_mask = 0
        for value in self.values:
            self.full_mask = self.full_mask | value.value
        self.is_wire_transparent = True


class FunctionPointerType(Type):
    def __init__(self, is_enabled, name, json_data):
        Type.__init__(self, name, json_data)
        self.return_type = None
        self.arguments = []


class TypedefType(Type):
    def __init__(self, is_enabled, name, json_data):
        Type.__init__(self, name, json_data)
        self.type = None


class NativeType(Type):
    def __init__(self, is_enabled, name, json_data):
        Type.__init__(self, name, json_data, native=True)
        self.is_wire_transparent = json_data.get('wire transparent', True)


# Methods and structures are both "records", so record members correspond to
# method arguments or structure members.
class RecordMember:
    def __init__(self,
                 name,
                 typ,
                 annotation,
                 json_data,
                 optional=False,
                 is_return_value=False,
                 default_value=None,
                 skip_serialize=False):
        self.name = name
        self.type = typ
        self.annotation = annotation
        self.json_data = json_data
        self.length = None
        self.optional = optional
        self.is_return_value = is_return_value
        self.handle_type = None
        self.id_type = None
        self.default_value = default_value
        self.skip_serialize = skip_serialize

    def set_handle_type(self, handle_type):
        assert self.type.dict_name == "ObjectHandle"
        self.handle_type = handle_type

    def set_id_type(self, id_type):
        assert self.type.dict_name == "ObjectId"
        self.id_type = id_type


Method = namedtuple(
    'Method', ['name', 'return_type', 'arguments', 'autolock', 'json_data'])


class ObjectType(Type):
    def __init__(self, is_enabled, name, json_data):
        json_data_override = {'methods': []}
        if 'methods' in json_data:
            json_data_override['methods'] = [
                m for m in json_data['methods'] if is_enabled(m)
            ]
        Type.__init__(self, name, dict(json_data, **json_data_override))


class Record:
    def __init__(self, name):
        self.name = Name(name)
        self.members = []
        self.may_have_dawn_object = False

    def update_metadata(self):
        def may_have_dawn_object(member):
            if isinstance(member.type, ObjectType):
                return True
            elif isinstance(member.type, StructureType):
                return member.type.may_have_dawn_object
            else:
                return False

        self.may_have_dawn_object = any(
            may_have_dawn_object(member) for member in self.members)

        # Set may_have_dawn_object to true if the type is chained or
        # extensible. Chained structs may contain a Dawn object.
        if isinstance(self, StructureType):
            self.may_have_dawn_object = (self.may_have_dawn_object
                                         or self.chained or self.extensible)


class StructureType(Record, Type):
    def __init__(self, is_enabled, name, json_data):
        Record.__init__(self, name)
        json_data_override = {}
        if 'members' in json_data:
            json_data_override['members'] = [
                m for m in json_data['members'] if is_enabled(m)
            ]
        Type.__init__(self, name, dict(json_data, **json_data_override))
        self.chained = json_data.get('chained', None)
        self.extensible = json_data.get('extensible', None)
        if self.chained:
            assert self.chained == 'in' or self.chained == 'out'
            assert 'chain roots' in json_data
            self.chain_roots = []
        if self.extensible:
            assert self.extensible == 'in' or self.extensible == 'out'
        # Chained structs inherit from wgpu::ChainedStruct, which has
        # nextInChain, so setting both extensible and chained would result in
        # two nextInChain members.
        assert not (self.extensible and self.chained)
        self.extensions = []

    def update_metadata(self):
        Record.update_metadata(self)

        if self.may_have_dawn_object:
            self.is_wire_transparent = False
            return

        assert not (self.chained or self.extensible)

        def get_is_wire_transparent(member):
            return member.type.is_wire_transparent and member.annotation == 'value'

        self.is_wire_transparent = all(
            get_is_wire_transparent(m) for m in self.members)

    @property
    def output(self):
        return self.chained == "out" or self.extensible == "out"

    @property
    def has_free_members_function(self):
        if not self.output:
            return False
        for m in self.members:
            if m.annotation != 'value':
                return True
        return False


class ConstantDefinition():
    def __init__(self, is_enabled, name, json_data):
        self.type = None
        self.value = json_data['value']
        self.json_data = json_data
        self.name = Name(name)


class FunctionDeclaration():
    def __init__(self, is_enabled, name, json_data, no_cpp=False):
        self.return_type = None
        self.arguments = []
        self.json_data = json_data
        self.name = Name(name)
        self.no_cpp = no_cpp


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
        member = RecordMember(Name(m['name']),
                              types[m['type']],
                              m.get('annotation', 'value'),
                              m,
                              optional=m.get('optional', False),
                              is_return_value=m.get('is_return_value', False),
                              default_value=m.get('default', None),
                              skip_serialize=m.get('skip_serialize', False))
        handle_type = m.get('handle_type')
        if handle_type:
            member.set_handle_type(types[handle_type])
        id_type = m.get('id_type')
        if id_type:
            member.set_id_type(types[id_type])
        members.append(member)
        members_by_name[member.name.canonical_case()] = member

    for (member, m) in zip(members, json_data):
        if member.annotation != 'value':
            if not 'length' in m:
                if member.type.category != 'object':
                    member.length = "constant"
                    member.constant_length = 1
                else:
                    assert False
            elif m['length'] == 'strlen':
                member.length = 'strlen'
            elif isinstance(m['length'], int):
                assert m['length'] > 0
                member.length = "constant"
                member.constant_length = m['length']
            else:
                member.length = members_by_name[m['length']]

    return members


def mark_lengths_non_serializable_lpm(record_members):
    # Remove member length values from command metadata,
    # these are set to the length of the protobuf array.
    for record_member in record_members:
        lengths = set()
        for member in record_member.members:
            lengths.add(member.length)

        for member in record_member.members:
            if member in lengths:
                member.skip_serialize = True

############################################################
# PARSE
############################################################


def link_object(obj, types):
    # Disable method's autolock if obj's "no autolock" = True
    obj_scoped_autolock_enabled = not obj.json_data.get('no autolock', False)

    def make_method(json_data):
        arguments = linked_record_members(json_data.get('args', []), types)
        autolock_enabled = obj_scoped_autolock_enabled and not json_data.get(
            'no autolock', False)
        return Method(Name(json_data['name']),
                      types[json_data.get('returns', 'void')], arguments,
                      autolock_enabled, json_data)

    obj.methods = [make_method(m) for m in obj.json_data.get('methods', [])]
    obj.methods.sort(key=lambda method: method.name.canonical_case())


def link_structure(struct, types):
    struct.members = linked_record_members(struct.json_data['members'], types)
    for root in struct.json_data.get('chain roots', []):
        struct.chain_roots.append(types[root])
        types[root].extensions.append(struct)
    struct.chain_roots = [types[root] for root in struct.json_data.get('chain roots', [])]
    assert all((root.category == 'structure' for root in struct.chain_roots))


def link_function_pointer(function_pointer, types):
    link_function(function_pointer, types)


def link_typedef(typedef, types):
    typedef.type = types[typedef.json_data['type']]


def link_constant(constant, types):
    constant.type = types[constant.json_data['type']]
    assert constant.type.name.native


def link_function(function, types):
    function.return_type = types[function.json_data.get('returns', 'void')]
    function.arguments = linked_record_members(function.json_data['args'],
                                               types)

# Sort structures so that if struct A has struct B as a member, then B is
# listed before A.
#
# This is a form of topological sort where we try to keep the order reasonably
# similar to the original order (though the sort isn't technically stable).
#
# It works by computing for each struct type what is the depth of its DAG of
# dependents, then re-sorting based on that depth using Python's stable sort.
# This makes a toposort because if A depends on B then its depth will be bigger
# than B's. It is also nice because all nodes with the same depth are kept in
# the input order.
def topo_sort_structure(structs):
    for struct in structs:
        struct.visited = False
        struct.subdag_depth = 0

    def compute_depth(struct):
        if struct.visited:
            return struct.subdag_depth

        max_dependent_depth = 0
        for member in struct.members:
            if member.type.category == 'structure':
                max_dependent_depth = max(max_dependent_depth,
                                          compute_depth(member.type) + 1)

        struct.subdag_depth = max_dependent_depth
        struct.visited = True
        return struct.subdag_depth

    for struct in structs:
        compute_depth(struct)

    result = sorted(structs, key=lambda struct: struct.subdag_depth)

    for struct in structs:
        del struct.visited
        del struct.subdag_depth

    return result


def parse_json(json, enabled_tags, disabled_tags=None):
    is_enabled = lambda json_data: item_is_enabled(
        enabled_tags, json_data) and not item_is_disabled(
            disabled_tags, json_data)
    category_to_parser = {
        'bitmask': BitmaskType,
        'enum': EnumType,
        'native': NativeType,
        'function pointer': FunctionPointerType,
        'object': ObjectType,
        'structure': StructureType,
        'typedef': TypedefType,
        'constant': ConstantDefinition,
        'function': FunctionDeclaration
    }

    types = {}

    by_category = {}
    for name in category_to_parser.keys():
        by_category[name] = []

    for (name, json_data) in json.items():
        if name[0] == '_' or not is_enabled(json_data):
            continue
        category = json_data['category']
        parsed = category_to_parser[category](is_enabled, name, json_data)
        by_category[category].append(parsed)
        types[name] = parsed

    for obj in by_category['object']:
        link_object(obj, types)

    for struct in by_category['structure']:
        link_structure(struct, types)

        if struct.has_free_members_function:
            name = struct.name.get() + " free members"
            func_decl = FunctionDeclaration(
                True,
                name, {
                    "returns":
                    "void",
                    "args": [{
                        "name": "value",
                        "type": struct.name.get(),
                        "annotation": "value",
                    }]
                },
                no_cpp=True)
            types[name] = func_decl
            by_category['function'].append(func_decl)

    for function_pointer in by_category['function pointer']:
        link_function_pointer(function_pointer, types)

    for typedef in by_category['typedef']:
        link_typedef(typedef, types)

    for constant in by_category['constant']:
        link_constant(constant, types)

    for function in by_category['function']:
        link_function(function, types)

    for category in by_category.keys():
        by_category[category] = sorted(
            by_category[category], key=lambda typ: typ.name.canonical_case())

    by_category['structure'] = topo_sort_structure(by_category['structure'])

    for struct in by_category['structure']:
        struct.update_metadata()

    api_params = {
        'types': types,
        'by_category': by_category,
        'enabled_tags': enabled_tags,
        'disabled_tags': disabled_tags,
    }
    return {
        'metadata': Metadata(json['_metadata']),
        'types': types,
        'by_category': by_category,
        'enabled_tags': enabled_tags,
        'disabled_tags': disabled_tags,
        'c_methods': lambda typ: c_methods(api_params, typ),
        'c_methods_sorted_by_name': get_c_methods_sorted_by_name(api_params),
    }


############################################################
# WIRE STUFF
############################################################


# Create wire commands from api methods
def compute_wire_params(api_params, wire_json):
    wire_params = api_params.copy()
    types = wire_params['types']

    commands = []
    return_commands = []

    wire_json['special items']['client_handwritten_commands'] += wire_json[
        'special items']['client_side_commands']

    # Generate commands from object methods
    for api_object in wire_params['by_category']['object']:
        for method in api_object.methods:
            command_name = concat_names(api_object.name, method.name)
            command_suffix = Name(command_name).CamelCase()

            # Only object return values or void are supported.
            # Other methods must be handwritten.
            is_object = method.return_type.category == 'object'
            is_void = method.return_type.name.canonical_case() == 'void'
            if not (is_object or is_void):
                assert command_suffix in (
                    wire_json['special items']['client_handwritten_commands']
                ), command_suffix
                continue

            if command_suffix in (
                    wire_json['special items']['client_side_commands']):
                continue

            # Create object method commands by prepending "self"
            members = [
                RecordMember(Name('self'), types[api_object.dict_name],
                             'value', {})
            ]
            members += method.arguments

            # Client->Server commands that return an object return the
            # result object handle
            if method.return_type.category == 'object':
                result = RecordMember(Name('result'),
                                      types['ObjectHandle'],
                                      'value', {},
                                      is_return_value=True)
                result.set_handle_type(method.return_type)
                members.append(result)

            command = Command(command_name, members)
            command.derived_object = api_object
            command.derived_method = method
            commands.append(command)

    for (name, json_data) in wire_json['commands'].items():
        commands.append(Command(name, linked_record_members(json_data, types)))

    for (name, json_data) in wire_json['return commands'].items():
        return_commands.append(
            Command(name, linked_record_members(json_data, types)))

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

############################################################
# DAWN LPM FUZZ STUFF
############################################################


def compute_lpm_params(api_and_wire_params, lpm_json):
    # Start with all commands in dawn.json and dawn_wire.json
    lpm_params = api_and_wire_params.copy()

    # Commands that are built through codegen
    generated_commands = []

    # All commands, including hand written commands that we can't generate
    # through codegen
    all_commands = []

    # Remove blocklisted commands from protobuf generation params
    blocklisted_cmds_proto = lpm_json.get('blocklisted_cmds')
    custom_cmds_proto = lpm_json.get('custom_cmds')
    for command in lpm_params['cmd_records']['command']:
        blocklisted = command.name.get() in blocklisted_cmds_proto
        custom = command.name.get() in custom_cmds_proto

        if blocklisted:
            continue

        if not custom:
            generated_commands.append(command)
        all_commands.append(command)

    # Set all fields that are marked as the "length" of another field to
    # skip_serialize. The values passed by libprotobuf-mutator will cause
    # an instant crash during serialization if these don't match the length
    # of the data they are passing. These values aren't used in
    # deserialization.
    mark_lengths_non_serializable_lpm(
        api_and_wire_params['cmd_records']['command'])
    mark_lengths_non_serializable_lpm(
        api_and_wire_params['by_category']['structure'])

    lpm_params['cmd_records'] = {
        'proto_generated_commands': generated_commands,
        'proto_all_commands': all_commands,
        'cpp_generated_commands': generated_commands,
        'lpm_info': lpm_json.get("lpm_info")
    }

    return lpm_params


def as_protobufTypeLPM(member):
    assert 'type' in member.json_data

    if member.type.name.native:
        typ = member.json_data['type']
        cpp_to_protobuf_type = {
            "bool": "bool",
            "float": "float",
            "double": "double",
            "int8_t": "int32",
            "int16_t": "int32",
            "int32_t": "int32",
            "int64_t": "int64",
            "uint8_t": "uint32",
            "uint16_t": "uint32",
            "uint32_t": "uint32",
            "uint64_t": "uint64",
            "size_t": "uint64",
        }

        assert typ in cpp_to_protobuf_type

        return cpp_to_protobuf_type[typ]

    return member.type.name.CamelCase()


# Helper that generates names for protobuf grammars from contents
# of dawn*.json like files. example: membera
def as_protobufNameLPM(*names):
    # `descriptor` is a reserved keyword in lib-protobuf-mutator
    if (names[0].concatcase() == "descriptor"):
        return "desc"
    return as_varName(*names)


# Helper to generate member accesses within C++ of protobuf objects
# example: cmd.membera().memberb()
def as_protobufMemberNameLPM(*names):
    # `descriptor` is a reserved keyword in lib-protobuf-mutator
    if (names[0].concatcase() == "descriptor"):
        return "desc"
    return ''.join([name.concatcase().lower() for name in names])


def unreachable_code():
    assert False


#############################################################
# Generator
#############################################################


def as_varName(*names):
    return names[0].camelCase() + ''.join(
        [name.CamelCase() for name in names[1:]])


def as_cType(c_prefix, name):
    # Special case for 'bool' because it has a typedef for compatibility.
    if name.native and name.get() != 'bool':
        return name.concatcase()
    else:
        return c_prefix + name.CamelCase()


def as_cReturnType(c_prefix, typ):
    if typ.category != 'bitmask':
        return as_cType(c_prefix, typ.name)
    else:
        return as_cType(c_prefix, typ.name) + 'Flags'


def as_cppType(name):
    # Special case for 'bool' because it has a typedef for compatibility.
    if name.native and name.get() != 'bool':
        return name.concatcase()
    else:
        return name.CamelCase()


def as_jsEnumValue(value):
    if 'jsrepr' in value.json_data: return value.json_data['jsrepr']
    return "'" + value.name.js_enum_case() + "'"


def convert_cType_to_cppType(typ, annotation, arg, indent=0):
    if typ.category == 'native':
        return arg
    if annotation == 'value':
        if typ.category == 'object':
            return '{}::Acquire({})'.format(as_cppType(typ.name), arg)
        elif typ.category == 'structure':
            converted_members = [
                convert_cType_to_cppType(
                    member.type, member.annotation,
                    '{}.{}'.format(arg, as_varName(member.name)), indent + 1)
                for member in typ.members
            ]

            converted_members = [(' ' * 4) + m for m in converted_members]
            converted_members = ',\n'.join(converted_members)

            return as_cppType(typ.name) + ' {\n' + converted_members + '\n}'
        elif typ.category == 'function pointer':
            return 'reinterpret_cast<{}>({})'.format(as_cppType(typ.name), arg)
        else:
            return 'static_cast<{}>({})'.format(as_cppType(typ.name), arg)
    else:
        return 'reinterpret_cast<{} {}>({})'.format(as_cppType(typ.name),
                                                    annotation, arg)


def decorate(name, typ, arg, make_const=False):
    maybe_const = ' const ' if make_const else ' '
    if arg.annotation == 'value':
        return typ + maybe_const + name
    elif arg.annotation == '*':
        return typ + ' *' + maybe_const + name
    elif arg.annotation == 'const*':
        return typ + ' const *' + maybe_const + name
    elif arg.annotation == 'const*const*':
        return 'const ' + typ + '* const *' + maybe_const + name
    else:
        assert False


def annotated(typ, arg, make_const=False):
    name = as_varName(arg.name)
    return decorate(name, typ, arg, make_const)


def item_is_enabled(enabled_tags, json_data):
    tags = json_data.get('tags')
    if tags is None: return True
    return any(tag in enabled_tags for tag in tags)


def item_is_disabled(disabled_tags, json_data):
    if disabled_tags is None: return False
    tags = json_data.get('tags')
    if tags is None: return False

    return any(tag in disabled_tags for tag in tags)


def as_cppEnum(value_name):
    assert not value_name.native
    if value_name.concatcase()[0].isdigit():
        return "e" + value_name.CamelCase()
    return value_name.CamelCase()


def as_MethodSuffix(type_name, method_name):
    assert not type_name.native and not method_name.native
    return type_name.CamelCase() + method_name.CamelCase()


def as_frontendType(metadata, typ):
    if typ.category == 'object':
        return typ.name.CamelCase() + 'Base*'
    elif typ.category in ['bitmask', 'enum'] or typ.name.get() == 'bool':
        return metadata.namespace + '::' + typ.name.CamelCase()
    elif typ.category == 'structure':
        return as_cppType(typ.name)
    else:
        return as_cType(metadata.c_prefix, typ.name)


def as_wireType(metadata, typ):
    if typ.category == 'object':
        return typ.name.CamelCase() + '*'
    elif typ.category in ['bitmask', 'enum', 'structure']:
        return metadata.c_prefix + typ.name.CamelCase()
    else:
        return as_cppType(typ.name)


def as_formatType(typ):
    # Unsigned integral types
    if typ.json_data['type'] in ['bool', 'uint32_t', 'uint64_t']:
        return 'u'

    # Defaults everything else to strings.
    return 's'


def c_methods(params, typ):
    return typ.methods + [
        Method(Name('reference'), params['types']['void'], [], False, {}),
        Method(Name('release'), params['types']['void'], [], False, {}),
    ]

def get_c_methods_sorted_by_name(api_params):
    unsorted = [(as_MethodSuffix(typ.name, method.name), typ, method) \
            for typ in api_params['by_category']['object'] \
            for method in c_methods(api_params, typ) ]
    return [(typ, method) for (_, typ, method) in sorted(unsorted)]


def has_callback_arguments(method):
    return any(arg.type.category == 'function pointer' for arg in method.arguments)


def make_base_render_params(metadata):
    c_prefix = metadata.c_prefix

    def as_cTypeEnumSpecialCase(typ):
        if typ.category == 'bitmask':
            return as_cType(c_prefix, typ.name) + 'Flags'
        return as_cType(c_prefix, typ.name)

    def as_cEnum(type_name, value_name):
        assert not type_name.native and not value_name.native
        return c_prefix + type_name.CamelCase() + '_' + value_name.CamelCase()

    def as_cMethod(type_name, method_name):
        c_method = c_prefix.lower()
        if type_name != None:
            assert not type_name.native
            c_method += type_name.CamelCase()
        assert not method_name.native
        c_method += method_name.CamelCase()
        return c_method

    def as_cProc(type_name, method_name):
        c_proc = c_prefix + 'Proc'
        if type_name != None:
            assert not type_name.native
            c_proc += type_name.CamelCase()
        assert not method_name.native
        c_proc += method_name.CamelCase()
        return c_proc

    return {
            'Name': lambda name: Name(name),
            'as_annotated_cType': \
                lambda arg, make_const=False: annotated(as_cTypeEnumSpecialCase(arg.type), arg, make_const),
            'as_annotated_cppType': \
                lambda arg, make_const=False: annotated(as_cppType(arg.type.name), arg, make_const),
            'as_cEnum': as_cEnum,
            'as_cppEnum': as_cppEnum,
            'as_cMethod': as_cMethod,
            'as_MethodSuffix': as_MethodSuffix,
            'as_cProc': as_cProc,
            'as_cType': lambda name: as_cType(c_prefix, name),
            'as_cReturnType': lambda typ: as_cReturnType(c_prefix, typ),
            'as_cppType': as_cppType,
            'as_jsEnumValue': as_jsEnumValue,
            'convert_cType_to_cppType': convert_cType_to_cppType,
            'as_varName': as_varName,
            'decorate': decorate,
            'as_formatType': as_formatType
        }


class MultiGeneratorFromDawnJSON(Generator):
    def get_description(self):
        return 'Generates code for various target from Dawn.json.'

    def add_commandline_arguments(self, parser):
        allowed_targets = [
            'dawn_headers', 'cpp_headers', 'cpp', 'proc', 'mock_api', 'wire',
            'native_utils', 'dawn_lpmfuzz_cpp', 'dawn_lpmfuzz_proto'
        ]

        parser.add_argument('--dawn-json',
                            required=True,
                            type=str,
                            help='The DAWN JSON definition to use.')
        parser.add_argument('--wire-json',
                            default=None,
                            type=str,
                            help='The DAWN WIRE JSON definition to use.')
        parser.add_argument("--lpm-json",
                            default=None,
                            type=str,
                            help='The DAWN LPM FUZZER definitions to use.')
        parser.add_argument(
            '--targets',
            required=True,
            type=str,
            help=
            'Comma-separated subset of targets to output. Available targets: '
            + ', '.join(allowed_targets))

    def get_file_renders(self, args):
        with open(args.dawn_json) as f:
            loaded_json = json.loads(f.read())

        targets = args.targets.split(',')

        wire_json = None
        if args.wire_json:
            with open(args.wire_json) as f:
                wire_json = json.loads(f.read())

        lpm_json = None
        if args.lpm_json:
            with open(args.lpm_json) as f:
                lpm_json = json.loads(f.read())

        renders = []

        params_dawn = parse_json(loaded_json,
                                 enabled_tags=['dawn', 'native', 'deprecated'])
        metadata = params_dawn['metadata']
        RENDER_PARAMS_BASE = make_base_render_params(metadata)

        api = metadata.api.lower()
        prefix = metadata.proc_table_prefix.lower()
        if 'headers' in targets:
            renders.append(
                FileRender('api.h', 'include/dawn/' + api + '.h',
                           [RENDER_PARAMS_BASE, params_dawn]))
            renders.append(
                FileRender('dawn_proc_table.h',
                           'include/dawn/' + prefix + '_proc_table.h',
                           [RENDER_PARAMS_BASE, params_dawn]))

        if 'cpp_headers' in targets:
            renders.append(
                FileRender('api_cpp.h', 'include/dawn/' + api + '_cpp.h',
                           [RENDER_PARAMS_BASE, params_dawn]))

            renders.append(
                FileRender('api_cpp_print.h',
                           'include/dawn/' + api + '_cpp_print.h',
                           [RENDER_PARAMS_BASE, params_dawn]))

            renders.append(
                FileRender('api_cpp_chained_struct.h',
                           'include/dawn/' + api + '_cpp_chained_struct.h',
                           [RENDER_PARAMS_BASE, params_dawn]))

        if 'proc' in targets:
            renders.append(
                FileRender('dawn_proc.c', 'src/dawn/' + prefix + '_proc.c',
                           [RENDER_PARAMS_BASE, params_dawn]))
            renders.append(
                FileRender('dawn_thread_dispatch_proc.cpp',
                           'src/dawn/' + prefix + '_thread_dispatch_proc.cpp',
                           [RENDER_PARAMS_BASE, params_dawn]))

        if 'webgpu_dawn_native_proc' in targets:
            renders.append(
                FileRender('dawn/native/api_dawn_native_proc.cpp',
                           'src/dawn/native/webgpu_dawn_native_proc.cpp',
                           [RENDER_PARAMS_BASE, params_dawn]))

        if 'cpp' in targets:
            renders.append(
                FileRender('api_cpp.cpp', 'src/dawn/' + api + '_cpp.cpp',
                           [RENDER_PARAMS_BASE, params_dawn]))

        if 'webgpu_headers' in targets:
            params_upstream = parse_json(loaded_json,
                                         enabled_tags=['upstream', 'native'],
                                         disabled_tags=['dawn'])
            renders.append(
                FileRender('api.h', 'webgpu-headers/' + api + '.h',
                           [RENDER_PARAMS_BASE, params_upstream]))

        if 'emscripten_bits' in targets:
            params_emscripten = parse_json(loaded_json,
                                           enabled_tags=['emscripten'])
            renders.append(
                FileRender('api.h', 'emscripten-bits/' + api + '.h',
                           [RENDER_PARAMS_BASE, params_emscripten]))
            renders.append(
                FileRender('api_cpp.h', 'emscripten-bits/' + api + '_cpp.h',
                           [RENDER_PARAMS_BASE, params_emscripten]))
            renders.append(
                FileRender('api_cpp.cpp', 'emscripten-bits/' + api + '_cpp.cpp',
                           [RENDER_PARAMS_BASE, params_emscripten]))
            renders.append(
                FileRender('api_struct_info.json',
                           'emscripten-bits/' + api + '_struct_info.json',
                           [RENDER_PARAMS_BASE, params_emscripten]))
            renders.append(
                FileRender('library_api_enum_tables.js',
                           'emscripten-bits/library_' + api + '_enum_tables.js',
                           [RENDER_PARAMS_BASE, params_emscripten]))

        if 'mock_api' in targets:
            mock_params = [
                RENDER_PARAMS_BASE, params_dawn, {
                    'has_callback_arguments': has_callback_arguments
                }
            ]
            renders.append(
                FileRender('mock_api.h', 'src/dawn/mock_' + api + '.h',
                           mock_params))
            renders.append(
                FileRender('mock_api.cpp', 'src/dawn/mock_' + api + '.cpp',
                           mock_params))

        if 'native_utils' in targets:
            frontend_params = [
                RENDER_PARAMS_BASE,
                params_dawn,
                {
                    # TODO: as_frontendType and co. take a Type, not a Name :(
                    'as_frontendType': lambda typ: as_frontendType(metadata, typ),
                    'as_annotated_frontendType': \
                        lambda arg: annotated(as_frontendType(metadata, arg.type), arg),
                }
            ]

            impl_dir = metadata.impl_dir + '/' if metadata.impl_dir else ''
            native_dir = impl_dir + Name(metadata.native_namespace).Dirs()
            namespace = metadata.namespace
            renders.append(
                FileRender('dawn/native/ValidationUtils.h',
                           'src/' + native_dir + '/ValidationUtils_autogen.h',
                           frontend_params))
            renders.append(
                FileRender('dawn/native/ValidationUtils.cpp',
                           'src/' + native_dir + '/ValidationUtils_autogen.cpp',
                           frontend_params))
            renders.append(
                FileRender('dawn/native/dawn_platform.h',
                           'src/' + native_dir + '/' + prefix + '_platform_autogen.h',
                           frontend_params))
            renders.append(
                FileRender('dawn/native/api_structs.h',
                           'src/' + native_dir + '/' + namespace + '_structs_autogen.h',
                           frontend_params))
            renders.append(
                FileRender('dawn/native/api_structs.cpp',
                           'src/' + native_dir + '/' + namespace + '_structs_autogen.cpp',
                           frontend_params))
            renders.append(
                FileRender('dawn/native/ProcTable.cpp',
                           'src/' + native_dir + '/ProcTable.cpp', frontend_params))
            renders.append(
                FileRender('dawn/native/ChainUtils.h',
                           'src/' + native_dir + '/ChainUtils_autogen.h',
                           frontend_params))
            renders.append(
                FileRender('dawn/native/ChainUtils.cpp',
                           'src/' + native_dir + '/ChainUtils_autogen.cpp',
                           frontend_params))
            renders.append(
                FileRender('dawn/native/Features.h',
                           'src/' + native_dir + '/Features_autogen.h',
                           frontend_params))
            renders.append(
                FileRender('dawn/native/Features.inl',
                           'src/' + native_dir + '/Features_autogen.inl',
                           frontend_params))
            renders.append(
                FileRender('dawn/native/api_absl_format.h',
                           'src/' + native_dir + '/' + api + '_absl_format_autogen.h',
                           frontend_params))
            renders.append(
                FileRender('dawn/native/api_absl_format.cpp',
                           'src/' + native_dir + '/' + api + '_absl_format_autogen.cpp',
                           frontend_params))
            renders.append(
                FileRender(
                    'dawn/native/api_StreamImpl.cpp', 'src/' + native_dir +
                    '/' + api + '_StreamImpl_autogen.cpp', frontend_params))
            renders.append(
                FileRender('dawn/native/ObjectType.h',
                           'src/' + native_dir + '/ObjectType_autogen.h',
                           frontend_params))
            renders.append(
                FileRender('dawn/native/ObjectType.cpp',
                           'src/' + native_dir + '/ObjectType_autogen.cpp',
                           frontend_params))

        if 'wire' in targets:
            params_dawn_wire = parse_json(loaded_json,
                                          enabled_tags=['dawn', 'deprecated'],
                                          disabled_tags=['native'])
            additional_params = compute_wire_params(params_dawn_wire,
                                                    wire_json)

            wire_params = [
                RENDER_PARAMS_BASE, params_dawn_wire, {
                    'as_wireType': lambda type : as_wireType(metadata, type),
                    'as_annotated_wireType': \
                        lambda arg: annotated(as_wireType(metadata, arg.type), arg),
                }, additional_params
            ]
            renders.append(
                FileRender('dawn/wire/ObjectType.h',
                           'src/dawn/wire/ObjectType_autogen.h', wire_params))
            renders.append(
                FileRender('dawn/wire/WireCmd.h',
                           'src/dawn/wire/WireCmd_autogen.h', wire_params))
            renders.append(
                FileRender('dawn/wire/WireCmd.cpp',
                           'src/dawn/wire/WireCmd_autogen.cpp', wire_params))
            renders.append(
                FileRender('dawn/wire/client/ApiObjects.h',
                           'src/dawn/wire/client/ApiObjects_autogen.h',
                           wire_params))
            renders.append(
                FileRender('dawn/wire/client/ApiProcs.cpp',
                           'src/dawn/wire/client/ApiProcs_autogen.cpp',
                           wire_params))
            renders.append(
                FileRender('dawn/wire/client/ClientBase.h',
                           'src/dawn/wire/client/ClientBase_autogen.h',
                           wire_params))
            renders.append(
                FileRender('dawn/wire/client/ClientHandlers.cpp',
                           'src/dawn/wire/client/ClientHandlers_autogen.cpp',
                           wire_params))
            renders.append(
                FileRender(
                    'dawn/wire/client/ClientPrototypes.inc',
                    'src/dawn/wire/client/ClientPrototypes_autogen.inc',
                    wire_params))
            renders.append(
                FileRender('dawn/wire/server/ServerBase.h',
                           'src/dawn/wire/server/ServerBase_autogen.h',
                           wire_params))
            renders.append(
                FileRender('dawn/wire/server/ServerDoers.cpp',
                           'src/dawn/wire/server/ServerDoers_autogen.cpp',
                           wire_params))
            renders.append(
                FileRender('dawn/wire/server/ServerHandlers.cpp',
                           'src/dawn/wire/server/ServerHandlers_autogen.cpp',
                           wire_params))
            renders.append(
                FileRender(
                    'dawn/wire/server/ServerPrototypes.inc',
                    'src/dawn/wire/server/ServerPrototypes_autogen.inc',
                    wire_params))

        if 'dawn_lpmfuzz_proto' in targets:
            params_dawn_wire = parse_json(loaded_json,
                                          enabled_tags=['dawn', 'deprecated'],
                                          disabled_tags=['native'])
            api_and_wire_params = compute_wire_params(params_dawn_wire,
                                                      wire_json)

            fuzzer_params = compute_lpm_params(api_and_wire_params, lpm_json)

            lpm_params = [
                RENDER_PARAMS_BASE, params_dawn_wire, {
                    'as_protobufTypeLPM': as_protobufTypeLPM,
                    'as_protobufNameLPM': as_protobufNameLPM,
                    'unreachable': unreachable_code
                }, api_and_wire_params, fuzzer_params
            ]

            renders.append(
                FileRender('dawn/fuzzers/lpmfuzz/dawn_lpm.proto',
                           'src/dawn/fuzzers/lpmfuzz/dawn_lpm_autogen.proto',
                           lpm_params))

            renders.append(
                FileRender(
                    'dawn/fuzzers/lpmfuzz/dawn_object_types_lpm.proto',
                    'src/dawn/fuzzers/lpmfuzz/dawn_object_types_lpm_autogen.proto',
                    lpm_params))

        if 'dawn_lpmfuzz_cpp' in targets:
            params_dawn_wire = parse_json(loaded_json,
                                          enabled_tags=['dawn', 'deprecated'],
                                          disabled_tags=['native'])
            api_and_wire_params = compute_wire_params(params_dawn_wire,
                                                      wire_json)

            fuzzer_params = compute_lpm_params(api_and_wire_params, lpm_json)

            lpm_params = [
                RENDER_PARAMS_BASE, params_dawn_wire, {
                    'as_protobufMemberName': as_protobufMemberNameLPM,
                    'unreachable_code': unreachable_code
                }, api_and_wire_params, fuzzer_params
            ]

            renders.append(
                FileRender(
                    'dawn/fuzzers/lpmfuzz/DawnLPMSerializer.cpp',
                    'src/dawn/fuzzers/lpmfuzz/DawnLPMSerializer_autogen.cpp',
                    lpm_params))

            renders.append(
                FileRender(
                    'dawn/fuzzers/lpmfuzz/DawnLPMSerializer.h',
                    'src/dawn/fuzzers/lpmfuzz/DawnLPMSerializer_autogen.h',
                    lpm_params))

            renders.append(
                FileRender(
                    'dawn/fuzzers/lpmfuzz/DawnLPMConstants.h',
                    'src/dawn/fuzzers/lpmfuzz/DawnLPMConstants_autogen.h',
                    lpm_params))

        return renders

    def get_dependencies(self, args):
        deps = [os.path.abspath(args.dawn_json)]
        if args.wire_json != None:
            deps += [os.path.abspath(args.wire_json)]
        if args.lpm_json != None:
            deps += [os.path.abspath(args.lpm_json)]
        return deps


if __name__ == '__main__':
    sys.exit(run_generator(MultiGeneratorFromDawnJSON()))
