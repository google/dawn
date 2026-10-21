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
from collections import defaultdict
from copy import deepcopy

from generator_lib import Generator, run_generator, FileRender, GeneratorOutput
import dawn_json_parsing
from dawn_json_parsing import (AnnotatedTypedMember, FunctionDeclaration,
                               Method, Name, Record, RecordMember,
                               StructureType, Type, concat_names,
                               linked_record_members, parse_addin)
from webgpu_docs_utility import build_doc_map, load_json_data


def parse_json(json, enabled_tags, disabled_tags=None, addins=None):
    api_params = dawn_json_parsing.parse_json(json,
                                              enabled_tags,
                                              disabled_tags=disabled_tags,
                                              addins=addins)
    api_params.update({
        'c_methods':
        lambda typ: c_methods(api_params, typ),
        'c_methods_sorted_by_parent':
        get_c_methods_sorted_by_parent(api_params),
        'c_methods_sorted_by_name':
        get_c_methods_sorted_by_name(api_params),
        'cpp_methods':
        lambda typ: cpp_methods(api_params, typ),
    })
    return api_params


############################################################
# WIRE STUFF
############################################################


class Command(Record):

    def __init__(self, name, members=None):
        Record.__init__(self, name)
        self.members = members or []
        self.derived_object = None
        self.derived_method = None


# Create wire commands from api methods
def compute_wire_params(api_params, wire_json):
    wire_params = api_params.copy()
    types = wire_params['types']

    commands = []
    return_commands = []
    special_commands = []

    wire_json['special items']['client_handwritten_commands'] += wire_json[
        'special items']['client_side_commands']

    # Generate commands from object methods
    for api_object in wire_params['by_category']['object']:
        # Reference counting functions are generated separately, so label them as "handwritten".
        wire_json['special items']['client_handwritten_commands'] += [
            api_object.name.CamelCase() + 'AddRef',
            api_object.name.CamelCase() + 'Release'
        ]

        for method in api_object.methods:
            command_name = concat_names(api_object.name, method.name)
            command_suffix = Name(command_name).CamelCase()

            # Only object return values, status or void are supported:
            #
            #- "void" is not a return value so commands can just be pushed to the server.
            # - objects use the wire's "promise pipelining" and will be sent associated with the
            #   WireHandle provided by the client.
            # - "status" is used to synchronously return validation errors so the server checks that
            #   they are always a success.
            #
            # Other methods must be handwritten.
            is_object = method.returns and method.returns.type.category == 'object'
            is_status = method.returns and method.returns.type.name.canonical_case(
            ) == 'status'
            is_void = method.returns == None
            if not (is_object or is_status or is_void):
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
            if method.returns and method.returns.type.category == 'object':
                result = RecordMember(Name('result'),
                                      types['ObjectHandle'],
                                      'value', {},
                                      is_return_value=True)
                result.set_handle_type(method.returns.type)
                members.append(result)

            command = Command(command_name, members)
            command.derived_object = api_object
            command.derived_method = method
            commands.append(command)

    # Generate commands from structure methods. Notes that currently this is only FreeMembers.
    for api_struct in wire_params['by_category']['structure']:
        wire_json['special items']['client_handwritten_commands'] += [
            api_struct.name.CamelCase() + 'FreeMembers'
        ]

    # TODO(crbug.com/575961728): Move or remove these linked_record_members usages so that
    # linked_record_members can be made private in dawn_json_parsing.py.
    for (name, json_data) in wire_json['commands'].items():
        commands.append(Command(name, linked_record_members(json_data, types)))

    for (name, json_data) in wire_json['return commands'].items():
        return_commands.append(
            Command(name, linked_record_members(json_data, types)))

    for (name, json_data) in wire_json['special commands'].items():
        special_commands.append(
            Command(name, linked_record_members(json_data, types)))

    wire_params['cmd_records'] = {
        'command': commands,
        'return command': return_commands,
        'special command': special_commands
    }

    for commands in wire_params['cmd_records'].values():
        for command in commands:
            command.update_metadata()
        commands.sort(key=lambda c: c.name.canonical_case())

    wire_params.update(wire_json.get('special items', {}))

    return wire_params


############################################################
# KOTLIN STUFF
############################################################


# Color the structures to determine which converters
# (Kotlin -> Native and Native -> Kotlin) are required.
def analyze_converter_usage(params_kotlin):
    # Initialize flags for both standard structures and callback info structures.
    for struct in params_kotlin['by_category']['structure'] + params_kotlin[
            'by_category']['callback info']:
        struct.needs_n2k = False
        struct.needs_k2n = False

    # Get the map of chained structs.
    chain_children = params_kotlin.get('chain_children', {})

    def mark_n2k(typ):
        # Only proceed if it's a structure and hasn't been marked yet to avoid infinite recursion.
        if isinstance(typ, StructureType) and not typ.needs_n2k:
            typ.needs_n2k = True
            # Recursively mark all members of this structure. kotlin_only
            # members never cross JNI, so their types don't need a converter.
            for member in typ.members:
                if not member.kotlin_only:
                    mark_n2k(member.type)
            # Recursively mark all potential chained children.
            for child in chain_children.get(typ.name.get(), []):
                mark_n2k(child)

    def mark_k2n(typ):
        # Only proceed if it's a structure and hasn't been marked yet to avoid infinite recursion.
        if isinstance(typ, StructureType) and not typ.needs_k2n:
            typ.needs_k2n = True
            # Recursively mark all members of this structure. See mark_n2k for
            # why kotlin_only members are excluded.
            for member in typ.members:
                if not member.kotlin_only:
                    mark_k2n(member.type)
            # Recursively mark all potential chained children.
            for child in chain_children.get(typ.name.get(), []):
                mark_k2n(child)

    # Scan Objects and Methods for roots.
    for obj in params_kotlin['by_category']['object']:
        for method in obj.methods:
            if not params_kotlin['include_method'](method):
                continue

            # Root A: Return values are always Native -> Kotlin.
            if method.returns:
                mark_n2k(method.returns.type)

            # Root B: Output parameters (mutable pointers '*') are Native -> Kotlin.
            for arg in method.arguments:
                if arg.annotation == '*':
                    mark_n2k(arg.type)
                else:
                    mark_k2n(arg.type)

    # Scan Callback Functions for roots.
    for cb in params_kotlin['by_category']['callback function']:
        for arg in cb.arguments:
            mark_n2k(arg.type)

    # Scan global functions if API exposes them.
    for func in params_kotlin['by_category']['function']:
        if func.returns:
            mark_n2k(func.returns.type)
        for arg in func.arguments:
            if arg.annotation == '*':
                mark_n2k(arg.type)
            else:
                mark_k2n(arg.type)


def compute_kotlin_params(loaded_json,
                          kotlin_json,
                          kotlin_addins,
                          webgpu_kt_docs_data=None,
                          doc_warn_log_file_path=None):

    params_kotlin = parse_json(loaded_json,
                               enabled_tags=['art', 'art_experimental'],
                               addins=kotlin_addins)
    params_kotlin['kotlin_package'] = kotlin_json['kotlin_package']
    params_kotlin['jni_primitives'] = kotlin_json['jni_primitives']
    params_kotlin['jni_signatures'] = kotlin_json['jni_signatures']
    kt_file_path = params_kotlin['kotlin_package'].replace('.', '/')

    # Among record members, the "omitted" addin is only supported on structure
    # members: the JNI argument struct is still built from the full argument
    # list, so omitting a function/method argument would silently pass a
    # zero-initialized value to the native call.
    all_functions_and_methods = params_kotlin['by_category']['function'] + [
        method for obj in params_kotlin['by_category']['object']
        for method in obj.methods
    ]
    for function in all_functions_and_methods:
        for arg in function.arguments:
            assert not getattr(arg, 'omitted', False), (
                f'"omitted" addin is not supported on arguments: '
                f'"{function.name.get()}::{arg.name.get()}"')

    # Kotlin-only members from the "additional_members" addin are appended to
    # the structure's members so that templates see a single member list. They
    # are marked kotlin_only because they have no native counterpart.
    for struct in params_kotlin['by_category']['structure']:
        for added_member in getattr(struct, 'additional_members', []):
            name = Name(added_member['name'])
            # Default to native for simple types if not specified
            category = added_member.get('category', 'native')
            type_name = added_member['type']
            if type_name in params_kotlin['types']:
                typ = params_kotlin['types'][type_name]
            else:
                typ = Type(type_name, {'category': category})
            struct.members.append(
                RecordMember(name,
                             typ,
                             added_member.get('annotation', 'value'), {},
                             optional=added_member.get('optional', False),
                             default_value=added_member.get(
                                 'default_value', None),
                             kotlin_only=True))

    def kotlin_record_members(members):
        # Members are sorted in the following order.
        # 1. members with no default value (except callbacks).
        # 2. members with default values.
        # 3. callbacks.
        for member in sorted(kotlin_record_members_unsorted(members),
                             key=lambda arg: kotlin_default(arg) is not None):
            yield member

        # Callbacks always go at the end.
        for member in members:
            if member.type.category == 'callback info':
                for callback_info_member in member.type.members:
                    if callback_info_member.type.category == 'callback function':
                        # We give the callback function a new name based on the callback info.
                        name = member.name.get().removesuffix(' info')
                        function_member = deepcopy(callback_info_member)
                        function_member.name = Name(name)
                        yield function_member
                    continue

    def kotlin_record_members_unsorted(members):
        for member in members:
            # length parameters are omitted because Kotlin containers have 'length'.
            if member in [m.length for m in members]:
                continue

            # userdata parameter omitted because Kotlin clients can achieve the same with closures.
            if member.name.get() == 'userdata':
                continue

            # Dawn sometimes uses 'annotation = *' for output parameters, for example to return
            # arrays. We convert the return type and strip out the parameters.
            if member.annotation == '*' and member.length == 'constant':
                continue

            # We replace the callback info with an executor here.
            if member.type.category == 'callback info':
                name = member.name.get().removesuffix(' info')
                yield RecordMember(
                    Name(name + ' executor'),
                    Type('java.util.concurrent.Executor',
                         {'category': 'kotlin type'}), None, {})
                continue

            if getattr(member, 'omitted', False):
                continue

            yield member

    # Calculate if we should, and can, provide a Kotlin default value for a given argument.
    # This will affect its order in the method parameter and structure field lists.
    def kotlin_default(arg):
        # Optional and non-optional container parameters are defaulted to empty containers to match
        # the behavior of the JavaScript API.
        if arg.length and arg.length != 'constant' and arg.type.name.get(
        ) != 'void':
            if arg.type.category in [
                    'callback function', 'callback info', 'function pointer',
                    'object', 'structure'
            ] or arg.type.name.get() == 'char':
                return 'arrayOf()'
            if arg.type.name.get() == 'float':
                return 'floatArrayOf()'
            return 'intArrayOf()'

        # All other optional types default to 'null'.
        if arg.optional:
            return 'null'

        # Non-optional structures are defaulted to a defaulted structure if we can construct one.
        # This is to match the behavior of the JavaScript API which lets clients pass undefined
        # structure values even for non-optional fields.
        if arg.type.category in [
                'structure', 'callback info'
        ] and arg.type.name.get() != 'string view' and all(
                kotlin_default(member) is not None
                for member in arg.type.members):
            constructor_args = []
            # default_value = zero is a special defaulting variation from C we have to emulate.
            if arg.default_value == 'zero':
                constructor_args = [
                    f"{member.name.camelCase()} = {member.type.name.CamelCase()}.{as_ktName(value.name.CamelCase())}"
                    for member in kotlin_record_members(arg.type.members)
                    if member.type.category in ['bitmask', 'enum']
                    for value in member.type.values if value.value == 0
                ]
            return f"{kotlin_name(arg.type)}({', '.join(constructor_args)})"

        # For bitmasks/enums we insert the full type of the default. No default value doesn't mean
        # no default in the bindings, because it should match the bitmask/enum labeled 'undefined'.
        if arg.type.category in ['bitmask', 'enum']:
            for value in arg.type.values:
                if value.name.name == (arg.default_value or 'undefined'):
                    return f"{arg.type.name.CamelCase()}.{as_ktName(value.name.CamelCase())}"
            return arg.default_value

        # Everything remaining requires a default value in the dawn.json.
        if arg.default_value in [None, 'nullptr']:
            return None

        if arg.type.category == 'native':
            # Is this a Dawn named constant that can be matched with the global definition?
            constant = find_by_name(by_category["constant"], arg.default_value)
            if constant:
                return 'Constants.' + as_ktName(constant.name.SNAKE_CASE())

            # Convert double/floats to the Kotlin format.
            if arg.type.name.get() in ['double', 'float']:
                return "%.1ff" % float(arg.default_value.rstrip('fF'))

            # Java doesn't have unsigned 32 bit / 64 bit variables so the cleanest workaround is
            # to insert the bitwise equivalent of a signed number.
            if arg.type.name.get() in ['int', 'int32_t', 'uint32_t'
                                       ] and arg.default_value == '0xFFFFFFFF':
                return '-1'
            if arg.type.name.get() in [
                    'int64_t', 'uint64_t', 'size_t'
            ] and arg.default_value == '0xFFFFFFFFFFFFFFFF':
                return '-1'

            # In all remaining cases the default as specified in dawn.json will work verbatim in
            # Kotlin.
            return arg.default_value

        unreachable_code(
            f"no logic to default '{arg.type.name.get()}' in category '{arg.type.category}'"
        )
        return None

    def kotlin_name(item):
        if isinstance(item, (FunctionDeclaration, Method)):
            # The "kotlin_name" addin lets dawn_kotlin.json rename a function
            # or method in the Kotlin API.
            return getattr(item, 'kotlin_name', item.name.camelCase())
        return f"{'GPU' if item.category in ('object', 'structure') else ''}{item.name.CamelCase()}"

    def kotlin_return(method):
        for argument in method.arguments:
            if argument.annotation == '*':
                if method.returns and method.returns.type.name.get(
                ) == 'size_t':
                    unreachable_code("Returning containers is not supported")
                if ((method.returns == None
                     or method.returns.type.name.get() == 'status')
                        and argument.type.category == 'structure'):
                    return argument

        # Check for "status-only" return.
        if (method.returns
                and method.returns.type.name.canonical_case() == 'status'):
            # This is a function like GPUBuffer.readMappedRange(). Its C return is a status,
            # but it has no "out" parameters. The idiomatic Kotlin function
            # should return Unit and throw an exception on failure.
            return None

        # If the function should return an omitted structure, we return nothing instead.
        if method.returns and method.returns.type.category == 'structure' and not include_structure(
                method.returns.type):
            return None

        # Return values are not treated as optional to keep the Kotlin API simple.
        # Methods are expected to return an object if declared. If they can't, dawn may raise an
        # error (converted to a Kotlin exception); otherwise JNI will throw NullPointerException.
        # In either case the optional type is redundant.
        return AnnotatedTypedMember(
            method.returns.type, method.returns.annotation, False,
            method.json_data) if method.returns else None

    def include_method(method):
        if method.returns and method.returns.type.category == 'function pointer':
            # Kotlin doesn't support returning functions.
            return False

        # Is the method marked omitted in dawn_kotlin.json?
        return not getattr(method, 'omitted', False)

    def include_structure(structure):
        if structure.name.canonical_case() == "string view":
            return False
        # Is the structure marked omitted in dawn_kotlin.json?
        return not getattr(structure, 'omitted', False)

    def include_enum(enum):
        return not getattr(enum, 'omitted', False)

    def include_callback(function):
        if getattr(function, 'omitted', False):
            return False

        structures = params_kotlin['by_category']['structure']
        function_pointers = params_kotlin['by_category']['function pointer']
        if any(member.name.get() == function.name.get()
               for member in function_pointers):
            return True

        included_callbacks = list()
        for struct in structures:
            if include_structure(struct):
                for member in kotlin_record_members(struct.members):
                    if member.type.category == 'callback function':
                        included_callbacks.append(member.name.get())

        return function.name.get() in included_callbacks

    def jni_name(type, category=None):
        if type.category == 'kotlin type':
            # Standard library Kotlin class (with namespace) just needs converting.
            return type.name.get().replace('.', '/')
        return f"{kt_file_path}/{kotlin_name(type)}"

    # A structure may need to know which other structures listed it as a chain root, e.g.
    # to know whether to mark the generated class 'open'.
    chain_children = defaultdict(list)
    by_category = params_kotlin['by_category']
    for structure in by_category['structure']:
        for chain_root in structure.chain_roots:
            chain_children[chain_root.name.get()].append(structure)

    kdocs_params = {
        'language': 'kotlin',
        'kdocs_blocklist': kotlin_json['kdocs_blocklist'],
        'kdocs_replacements': kotlin_json['kdocs_replacements'],
        'doc_warn_log_filepath': doc_warn_log_file_path,
    }
    params_kotlin['kdocs'] = build_doc_map(by_category=by_category,
                                           json_data=webgpu_kt_docs_data,
                                           params=kdocs_params)
    params_kotlin['chain_children'] = chain_children
    params_kotlin['kotlin_default'] = kotlin_default
    params_kotlin['kotlin_return'] = kotlin_return
    params_kotlin['kotlin_name'] = kotlin_name
    params_kotlin['include_method'] = include_method
    params_kotlin['include_structure'] = include_structure
    params_kotlin['include_enum'] = include_enum
    params_kotlin['kotlin_record_members'] = kotlin_record_members
    params_kotlin['jni_name'] = jni_name
    params_kotlin['include_callback'] = include_callback

    def check_jvm_overload_usage(functions):
        """Checks functions to see if they have default parameters.

        Sets a `has_default` flag on each function, which is used to add @JvmOverloads.
        """
        for func in functions:
            func.has_default = False
            for arg in func.arguments:
                if kotlin_default(arg) is not None:
                    func.has_default = True
                    break

    check_jvm_overload_usage(params_kotlin['by_category']['function'])

    params_kotlin['has_kotlin_classes'] = (
        [
            callback for callback in by_category['callback function'] +
            by_category['function pointer'] if include_callback(callback)
        ] + [enum for enum in by_category['enum'] if include_enum(enum)] +
        by_category['object'] + [
            structure for structure in by_category['structure']
            if include_structure(structure)
        ])

    analyze_converter_usage(params_kotlin)

    return params_kotlin


#############################################################
# Generator
#############################################################


def as_varName(*names):
    return names[0].camelCase() + ''.join(
        [name.CamelCase() for name in names[1:]])


def as_cType(c_prefix, name, spanify=False):
    # Special case for 'bool' because it has a typedef for compatibility.
    if name.get() == 'void' and spanify:
        return 'std::byte'
    elif name.native and name.get() != 'bool':
        return name.concatcase()
    else:
        return c_prefix + name.CamelCase()


def as_cppType(name):
    # Special case for 'bool' because it has a typedef for compatibility.
    if name.native and name.get() != 'bool':
        return name.concatcase()
    else:
        return name.CamelCase()


def as_ktName(name):
    return '_' + name if '0' <= name[0] <= '9' else name


def as_jsEnumValue(value):
    if 'jsrepr' in value.json_data: return value.json_data['jsrepr']
    return "'" + value.name.js_enum_case() + "'"


def has_wasmType(return_type, args):
    return all(map(lambda x: len(as_wasmType(x)) == 1, [return_type] + args))


# Returns a single character wasm type (v/p/i/j/f/d) if valid, a "(longer string)" if not
def as_wasmType(x):
    if x is None:
        return 'v'  # void return type

    if isinstance(x, AnnotatedTypedMember):
        if x.annotation == 'value':
            x = x.type
        elif '*' in x.annotation:
            return 'p'
        else:
            x = x.type

    if isinstance(x, Type):
        if x.category == 'enum':
            return 'i'
        elif x.category == 'bitmask':
            return 'j'
        elif x.category in ['object', 'function pointer']:
            return 'p'
        elif x.category == 'native':
            return x.json_data.get('wasm type', f'({x.name.name})')
        elif x.category in ['structure', 'callback info']:
            return f'({x.name.name})'  # Invalid
        else:
            assert False, 'Type -> ' + x.category


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


def decorate(typ, arg, *, with_nullability):
    s = typ
    if arg.annotation != 'value' or arg.type.is_nullable_pointer:
        if arg.annotation == '*':
            s = typ + ' *'
        elif arg.annotation == 'const*':
            s = typ + ' const *'
        elif arg.annotation == 'const*const*':
            s = 'const ' + typ + '* const *'
        if with_nullability:
            nullability = 'WGPU_NULLABLE ' if arg.optional else ''
            s = nullability + s
    return s


def annotate(typ, arg, *, make_const_member=False, with_nullability=False):
    result = decorate(typ, arg, with_nullability=with_nullability)
    if isinstance(arg, RecordMember):
        if make_const_member:
            result += ' const'
        result += ' ' + as_varName(arg.name)
    return result


def as_cppEnum(value_name):
    assert not value_name.native
    if value_name.concatcase()[0].isdigit():
        return "e" + value_name.CamelCase()
    return value_name.CamelCase()


def as_MethodSuffix(type_name, method_name):
    c_method = ''
    if type_name:
        assert not type_name.native
        c_method += type_name.CamelCase()
    assert not method_name.native
    c_method += method_name.CamelCase()
    return c_method


def as_CppMethodSuffix(type_name, method_name):
    assert not type_name.native and not method_name.native
    original_method_name_str = method_name.CamelCase()
    if method_name.chunks[-1] == 'f':
        return type_name.CamelCase() + original_method_name_str[:-1]
    return type_name.CamelCase() + original_method_name_str


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


def c_methods(params, typ):
    if typ.category == 'object':
        return typ.methods + [
            Method(Name('add ref'), None, [], False, {}),
            Method(Name('release'), None, [], False, {}),
        ]
    elif typ.category == 'structure':
        if typ.has_free_members_function:
            return [Method(Name('free members'), None, [], False, {})]
        return []
    else:
        assert False, "c_methods only valid on objects and structure"


def cpp_methods(params, typ):
    if typ.category == 'structure':
        methods = []
        for member in typ.members:
            if member.type.category == 'callback info':
                methods.append(
                    Method(Name(" ".join(["set"] + member.name.chunks[:-1])),
                           None, [member], False, {}))
        return methods
    else:
        assert False, "cpp_methods only valid on structures"

def get_c_methods_sorted_by_parent(api_params):
    return sorted([(typ, c_methods(api_params, typ))
                   for typ in (api_params['by_category']['object'] +
                               api_params['by_category']['structure'])
                   if len(c_methods(api_params, typ)) > 0])


def get_c_methods_sorted_by_name(api_params):
    unsorted = [(as_MethodSuffix(typ.name, method.name), typ, method)
                for (typ,
                     methods) in get_c_methods_sorted_by_parent(api_params)
                for method in methods]
    unsorted += [(function.name.CamelCase(), None, function)
                 for function in api_params['by_category']['function']]
    return [(typ, method) for (_, typ, method) in sorted(unsorted)]


def find_by_name(members, name):
    for member in members:
        if member.name.get() == name:
            return member
    return None


def has_callback_arguments(method):
    return any(arg.type.category == 'function pointer'
               for arg in method.arguments)


def has_callbackInfoStruct(args):
    return any(arg.type.category == 'callback info' for arg in args)


def is_wire_serializable(type):
    # Function pointers, callback functions, and "void *" types (i.e. userdata) cannot
    # be serialized.
    return (type.category != 'function pointer'
            and type.category != 'callback info'
            and type.category != 'callback function'
            and type.name.get() != 'void *')


def is_enum_value_proxy(value):
    conflicts = value.json_data.get('enum_value_conflict', False)
    is_proxy = 'deprecated' in value.json_data.get(
        'tags', []) or value.json_data.get('is_proxy', False)
    return conflicts and is_proxy


def unreachable_code(msg="unreachable_code"):
    assert False, msg


def make_base_render_params(metadata):
    c_prefix = metadata.c_prefix

    def as_cTypeEnumSpecialCase(typ):
        return as_cType(c_prefix, typ.name)

    def as_cEnum(type_name, value_name):
        assert not type_name.native and not value_name.native
        return c_prefix + type_name.CamelCase() + '_' + value_name.CamelCase()

    def as_cMethodNamespaced(type_name, method_name, namespace=None):
        c_method = c_prefix.lower()
        if namespace is not None:
            c_method += namespace.CamelCase()
        if type_name:
            assert not type_name.native
            c_method += type_name.CamelCase()
        assert not method_name.native
        c_method += method_name.CamelCase()
        return c_method

    def as_cMethod(type_name, method_name):
        return as_cMethodNamespaced(type_name, method_name)

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
            'as_nullability_annotated_cType': \
                lambda arg: 'void' if arg is None else annotate(as_cTypeEnumSpecialCase(arg.type), arg, with_nullability=True),
            'as_annotated_cType': \
                lambda arg: 'void' if arg is None else annotate(as_cTypeEnumSpecialCase(arg.type), arg),
            'as_annotated_cppType': \
                lambda arg, make_const_member=False: 'void' if arg is None else annotate(as_cppType(arg.type.name), arg, make_const_member=make_const_member),
            'as_cEnum': as_cEnum,
            'as_cppEnum': as_cppEnum,
            'as_cMethod': as_cMethod,
            'as_cMethodNamespaced': as_cMethodNamespaced,
            'as_MethodSuffix': as_MethodSuffix,
            'as_CppMethodSuffix': as_CppMethodSuffix,
            'as_cProc': as_cProc,
            'as_cType': lambda name, spanify=False: as_cType(c_prefix, name, spanify),
            'as_cppType': as_cppType,
            'as_jsEnumValue': as_jsEnumValue,
            'has_wasmType': has_wasmType,
            'as_wasmType': as_wasmType,
            'convert_cType_to_cppType': convert_cType_to_cppType,
            'as_varName': as_varName,
            'decorate': lambda typ, arg: decorate(typ, arg, with_nullability=False),
            'as_ktName': as_ktName,
            'has_callbackInfoStruct': has_callbackInfoStruct,
            'find_by_name': find_by_name,
            'print': print,
            'unreachable_code': unreachable_code,
            'is_enum_value_proxy': is_enum_value_proxy,
        }


class MultiGeneratorFromDawnJSON(Generator):

    def get_description(self):
        return 'Generates code for various target from Dawn.json.'

    def add_commandline_arguments(self, parser):
        allowed_targets = [
            'dawn_headers', 'cpp_headers', 'cpp', 'proc', 'mock_api', 'wire',
            'native_utils', 'kotlin'
        ]

        parser.add_argument('--dawn-json',
                            required=True,
                            type=str,
                            help='The DAWN JSON definition to use.')
        parser.add_argument('--wire-json',
                            default=None,
                            type=str,
                            help='The DAWN WIRE JSON definition to use.')
        parser.add_argument('--native-json',
                            default=None,
                            type=str,
                            help='The DAWN NATIVE JSON definition to use.')
        parser.add_argument('--kotlin-json',
                            default=None,
                            type=str,
                            help='The KOTLIN JSON definition to use.')
        parser.add_argument('--webgpu-kt-docs',
                            default=None,
                            type=str,
                            help='The WebGPU Kotlin API documentation to use.')
        parser.add_argument(
            '--targets',
            required=True,
            type=str,
            help=
            'Comma-separated subset of targets to output. Available targets: '
            + ', '.join(allowed_targets))

        parser.add_argument(
            '--doc-warn-log-file',
            default=None,
            type=str,
            help=
            'Path to output file for documentation warnings; ignored if not set.',
        )

    def get_outputs(self, args):
        with open(args.dawn_json) as f:
            loaded_json = json.loads(f.read())

        targets = args.targets.split(',')

        wire_json = None
        wire_addins = []
        if args.wire_json:
            with open(args.wire_json) as f:
                wire_json = json.loads(f.read())
            wire_addins = [parse_addin(args.wire_json)]

        native_addins = []
        if args.native_json:
            native_addins = [parse_addin(args.native_json)]

        kotlin_json = None
        kotlin_addins = []
        if args.kotlin_json:
            with open(args.kotlin_json) as f:
                kotlin_json = json.loads(f.read())
            kotlin_addins = [parse_addin(args.kotlin_json)]

        webgpu_kt_docs_data = None
        if args.webgpu_kt_docs:
            webgpu_kt_docs_data = load_json_data(args.webgpu_kt_docs)

        doc_warn_log_file_path = args.doc_warn_log_file

        renders = []
        imported_templates = []

        params_dawn = parse_json(loaded_json,
                                 enabled_tags=['dawn', 'native', 'deprecated'])

        params_all = parse_json(
            loaded_json,
            enabled_tags=['dawn', 'emscripten', 'native', 'deprecated'])

        metadata = params_dawn['metadata']
        RENDER_PARAMS_BASE = make_base_render_params(metadata)

        api = metadata.api.lower()
        prefix = metadata.proc_table_prefix.lower()
        if 'headers' in targets:
            imported_templates.append('BSD_LICENSE')
            renders.append(
                FileRender('api.h', 'include/dawn/' + api + '.h',
                           [RENDER_PARAMS_BASE, params_all]))
            renders.append(
                FileRender('dawn/wire/client/api.h',
                           'include/dawn/wire/client/' + api + '.h',
                           [RENDER_PARAMS_BASE, params_dawn]))
            renders.append(
                FileRender('dawn_proc_table.h',
                           'include/dawn/' + prefix + '_proc_table.h',
                           [RENDER_PARAMS_BASE, params_dawn]))

        if 'cpp_headers' in targets:
            imported_templates += [
                "dawn/cpp_macros.tmpl",
            ]

            renders.append(
                FileRender('api_cpp.h', 'include/dawn/' + api + '_cpp.h', [
                    RENDER_PARAMS_BASE, params_all, {
                        'c_header': api + '/' + api + '.h',
                        'c_namespace': None,
                    }
                ]))

            renders.append(
                FileRender(
                    'api_cpp.h', 'include/dawn/wire/client/' + api + '_cpp.h',
                    [
                        RENDER_PARAMS_BASE, params_dawn, {
                            'c_header': 'dawn/wire/client/' + api + '.h',
                            'c_namespace': Name('dawn wire client'),
                        }
                    ]))

            renders.append(
                FileRender('api_cpp_print.h',
                           'include/dawn/' + api + '_cpp_print.h', [
                               RENDER_PARAMS_BASE, params_dawn, {
                                   'cpp_header': api + '/' + api + '_cpp.h',
                                   'c_namespace': None,
                               }
                           ]))

            renders.append(
                FileRender(
                    'api_cpp_print.h',
                    'include/dawn/wire/client/' + api + '_cpp_print.h', [
                        RENDER_PARAMS_BASE, params_dawn, {
                            'cpp_header': 'dawn/wire/client/' + api + '_cpp.h',
                            'c_namespace': Name('dawn wire client'),
                        }
                    ]))

            renders.append(
                FileRender('api_cpp_chained_struct.h',
                           'include/webgpu/' + api + '_cpp_chained_struct.h',
                           [RENDER_PARAMS_BASE, params_dawn]))

        if 'cpp_modules' in targets:
            renders.append(
                FileRender('api_cpp.ixx', 'include/dawn/' + api + '.ixx', [
                    RENDER_PARAMS_BASE, params_dawn, {
                        'cpp_header': api + '/' + api + '_cpp.h',
                    }
                ]))

        if 'proc' in targets:
            renders.append(
                FileRender('dawn_proc.cpp', 'src/dawn/' + prefix + '_proc.cpp',
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

        if 'webgpu_headers' in targets:
            imported_templates += [
                "BSD_LICENSE",
                "dawn/cpp_macros.tmpl",
            ]

            params_upstream = parse_json(loaded_json,
                                         enabled_tags=['native'],
                                         disabled_tags=['dawn'])
            renders.append(
                FileRender('api.h', 'webgpu-headers/' + api + '.h',
                           [RENDER_PARAMS_BASE, params_upstream]))

            upstream_cpp = 'include/webgpu_upstream/' + api + '/' + api
            renders.append(
                FileRender('api_cpp.h', upstream_cpp + '_cpp.h', [
                    RENDER_PARAMS_BASE, params_upstream, {
                        'c_header': api + '/' + api + '.h',
                        'c_namespace': None,
                    }
                ]))
            renders.append(
                FileRender('api_cpp_chained_struct.h',
                           upstream_cpp + '_cpp_chained_struct.h',
                           [RENDER_PARAMS_BASE, params_upstream]))
            renders.append(
                FileRender('api_cpp_print.h', upstream_cpp + '_cpp_print.h', [
                    RENDER_PARAMS_BASE, params_upstream, {
                        'cpp_header': api + '/' + api + '_cpp.h',
                        'c_namespace': None,
                    }
                ]))

        if 'emdawnwebgpu_headers' in targets:
            imported_templates += [
                "dawn/cpp_macros.tmpl",
            ]

            assert api == 'webgpu'
            params_emscripten = parse_json(loaded_json,
                                           enabled_tags=['emscripten'])
            # system/include/webgpu
            imported_templates.append('BSD_LICENSE')
            renders.append(
                FileRender('api.h', 'src/emdawnwebgpu/include/webgpu/webgpu.h',
                           [RENDER_PARAMS_BASE, params_emscripten]))
            renders.append(
                FileRender('api_cpp.h',
                           'src/emdawnwebgpu/include/webgpu/webgpu_cpp.h', [
                               RENDER_PARAMS_BASE, params_emscripten, {
                                   'c_header': api + '/' + api + '.h',
                                   'c_namespace': None,
                               }
                           ]))
            renders.append(
                FileRender(
                    'api_cpp_chained_struct.h',
                    'src/emdawnwebgpu/include/webgpu/webgpu_cpp_chained_struct.h',
                    [RENDER_PARAMS_BASE, params_emscripten]))
            renders.append(
                FileRender('api_cpp_print.h',
                           'src/emdawnwebgpu/include/dawn/webgpu_cpp_print.h',
                           [
                               RENDER_PARAMS_BASE, params_emscripten, {
                                   'cpp_header': api + '/' + api + '_cpp.h',
                                   'c_namespace': None,
                               }
                           ]))

        if 'emdawnwebgpu_modules' in targets:
            assert api == 'webgpu'
            params_emscripten = parse_json(loaded_json,
                                           enabled_tags=['emscripten'])
            renders.append(
                FileRender('api_cpp.ixx', 'include/dawn/' + api + '.ixx', [
                    RENDER_PARAMS_BASE, params_emscripten, {
                        'cpp_header': api + '/' + api + '_cpp.h',
                    }
                ]))

        if 'emdawnwebgpu_js' in targets:
            assert api == 'webgpu'
            params_emscripten = parse_json(loaded_json,
                                           enabled_tags=['emscripten'])
            renders.append(
                FileRender('emdawnwebgpu/struct_info_webgpu.json',
                           'src/emdawnwebgpu/struct_info_webgpu.json',
                           [RENDER_PARAMS_BASE, params_emscripten]))
            renders.append(
                FileRender('emdawnwebgpu/library_webgpu_enum_tables.js',
                           'src/emdawnwebgpu/library_webgpu_enum_tables.js',
                           [RENDER_PARAMS_BASE, params_emscripten]))
            renders.append(
                FileRender(
                    'emdawnwebgpu/library_webgpu_generated_sig_info.js',
                    'src/emdawnwebgpu/library_webgpu_generated_sig_info.js',
                    [RENDER_PARAMS_BASE, params_emscripten]))

        if 'emdawnwebgpu_link_test_cpp' in targets:
            assert api == 'webgpu'
            params_emscripten = parse_json(loaded_json,
                                           enabled_tags=['emscripten'])
            renders.append(
                FileRender('emdawnwebgpu/LinkTest.cpp',
                           'src/emdawnwebgpu/LinkTest.cpp',
                           [RENDER_PARAMS_BASE, params_emscripten]))

        if 'mock_api' in targets:
            mock_params = [
                RENDER_PARAMS_BASE, params_dawn, {
                    'has_callback_arguments': has_callback_arguments,
                }
            ]
            renders.append(
                FileRender('mock_api.h', 'src/dawn/mock_' + api + '.h',
                           mock_params))
            renders.append(
                FileRender('mock_api.cpp', 'src/dawn/mock_' + api + '.cpp',
                           mock_params))

        if 'native_utils' in targets:
            params_dawn_native = parse_json(
                loaded_json,
                enabled_tags=['dawn', 'native', 'deprecated'],
                addins=native_addins)
            frontend_params = [
                RENDER_PARAMS_BASE,
                params_dawn_native,
                {
                    # TODO: as_frontendType and co. take a Type, not a Name :(
                    'as_frontendType':
                    lambda typ: as_frontendType(metadata, typ),
                },
            ]

            imported_templates += [
                "dawn/api_structs.h.tmpl",
                "dawn/api_structs.cpp.tmpl",
                "dawn/cpp_macros.tmpl",
                "dawn/dawn_platform.h.tmpl",
            ]

            impl_dir = metadata.impl_dir + '/' if metadata.impl_dir else ''
            native_dir = impl_dir + Name(metadata.native_namespace).Dirs()
            namespace = metadata.namespace
            renders.append(
                FileRender('dawn/native/ValidationUtils.h',
                           native_dir + '/ValidationUtils_autogen.h',
                           frontend_params))
            renders.append(
                FileRender('dawn/native/ValidationUtils.cpp',
                           native_dir + '/ValidationUtils_autogen.cpp',
                           frontend_params))
            renders.append(
                FileRender('dawn/native/dawn_platform.h',
                           native_dir + '/' + prefix + '_platform_autogen.h',
                           frontend_params))
            renders.append(
                FileRender('dawn/native/api_structs.h',
                           native_dir + '/' + namespace + '_structs_autogen.h',
                           frontend_params))
            renders.append(
                FileRender(
                    'dawn/native/api_structs.cpp',
                    native_dir + '/' + namespace + '_structs_autogen.cpp',
                    frontend_params))
            renders.append(
                FileRender(
                    'dawn/native/api_structs_defaults.h', native_dir + '/' +
                    namespace + '_structs_defaults_autogen.h',
                    frontend_params))
            renders.append(
                FileRender(
                    'dawn/native/api_structs_defaults.cpp', native_dir + '/' +
                    namespace + '_structs_defaults_autogen.cpp',
                    frontend_params))
            renders.append(
                FileRender('dawn/native/ProcTable.cpp',
                           native_dir + '/ProcTable.cpp', frontend_params))
            renders.append(
                FileRender('dawn/native/ChainUtils.h',
                           native_dir + '/ChainUtils_autogen.h',
                           frontend_params))
            renders.append(
                FileRender('dawn/native/ChainUtils.cpp',
                           native_dir + '/ChainUtils_autogen.cpp',
                           frontend_params))
            renders.append(
                FileRender('dawn/native/Features.h',
                           native_dir + '/Features_autogen.h',
                           frontend_params))
            renders.append(
                FileRender('dawn/native/Features.inl',
                           native_dir + '/Features_autogen.inl',
                           frontend_params))
            renders.append(
                FileRender('dawn/native/api_absl_format.h',
                           native_dir + '/' + api + '_absl_format_autogen.h',
                           frontend_params))
            renders.append(
                FileRender('dawn/native/api_absl_format.cpp',
                           native_dir + '/' + api + '_absl_format_autogen.cpp',
                           frontend_params))
            renders.append(
                FileRender('dawn/native/api_StreamImpl.cpp',
                           native_dir + '/' + api + '_StreamImpl_autogen.cpp',
                           frontend_params))
            renders.append(
                FileRender('dawn/native/ObjectType.h',
                           native_dir + '/ObjectType_autogen.h',
                           frontend_params))
            renders.append(
                FileRender('dawn/native/ObjectType.cpp',
                           native_dir + '/ObjectType_autogen.cpp',
                           frontend_params))

        if 'dawn_utils' in targets:
            # Generate ComboLimits without any extensions so it works on
            # all targets (and doesn't chain any experimental stuff like
            # extensions that are output only and produce warnings on input).
            params = parse_json(loaded_json, enabled_tags=[])
            renders.append(
                FileRender('dawn/utils/ComboLimits.h',
                           'src/dawn/utils/ComboLimits.h',
                           [RENDER_PARAMS_BASE, params]))
            renders.append(
                FileRender('dawn/utils/ComboLimits.cpp',
                           'src/dawn/utils/ComboLimits.cpp',
                           [RENDER_PARAMS_BASE, params]))

        if 'wire' in targets:
            params_dawn_wire = parse_json(loaded_json,
                                          enabled_tags=['dawn', 'deprecated'],
                                          disabled_tags=['native'],
                                          addins=wire_addins)
            additional_params = compute_wire_params(params_dawn_wire,
                                                    wire_json)

            imported_templates += [
                "dawn/api_structs.h.tmpl",
                "dawn/api_structs.cpp.tmpl",
                "dawn/cpp_macros.tmpl",
                "dawn/dawn_platform.h.tmpl",
            ]

            wire_params = [
                RENDER_PARAMS_BASE, params_dawn_wire, {
                    'as_wireType': lambda type : as_wireType(metadata, type),
                    'as_annotated_wireType': \
                        lambda arg: annotate(as_wireType(metadata, arg.type), arg),
                    'is_wire_serializable': lambda type : is_wire_serializable(type),
                    'is_wire_data_only': \
                        lambda member: member.type.name.get() in ['void', 'std::byte'] and \
                                       not member.skip_serialize,
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
                FileRender(
                    'dawn/wire/api_structs.h', 'src/dawn/wire/' +
                    metadata.namespace + '_structs_autogen.h', wire_params))
            renders.append(
                FileRender(
                    'dawn/wire/api_structs.cpp', 'src/dawn/wire/' +
                    metadata.namespace + '_structs_autogen.cpp', wire_params))
            renders.append(
                FileRender('dawn/wire/dawn_platform.h',
                           'src/dawn/wire/' + prefix + '_platform.h',
                           wire_params))

            renders.append(
                FileRender('dawn/wire/client/ApiObjects.h',
                           'src/dawn/wire/client/ApiObjects_autogen.h',
                           wire_params))
            renders.append(
                FileRender('dawn/wire/client/ApiProcs.cpp',
                           'src/dawn/wire/client/ApiProcs_autogen.cpp.inc',
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
                FileRender(
                    'dawn/wire/client/api_structs.h', 'src/dawn/wire/client/' +
                    metadata.namespace + '_structs_autogen.h', wire_params))
            renders.append(
                FileRender(
                    'dawn/wire/client/api_structs.cpp',
                    'src/dawn/wire/client/' + metadata.namespace +
                    '_structs_autogen.cpp', wire_params))
            renders.append(
                FileRender('dawn/wire/client/dawn_platform.h',
                           'src/dawn/wire/client/' + prefix + '_platform.h',
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
            renders.append(
                FileRender('dawn/wire/server/WGPUTraits.h',
                           'src/dawn/wire/server/WGPUTraits_autogen.h',
                           wire_params))


        if 'kotlin' in targets:
            params_kotlin = compute_kotlin_params(loaded_json, kotlin_json,
                                                  kotlin_addins,
                                                  webgpu_kt_docs_data,
                                                  doc_warn_log_file_path)
            kt_file_path = params_kotlin['kotlin_package'].replace('.', '/')
            jni_name = params_kotlin['jni_name']

            imported_templates += [
                "art/api_kotlin_async_helpers.kt",
                "art/api_kotlin_types.kt",
            ]

            by_category = params_kotlin['by_category']
            include_callback = params_kotlin['include_callback']

            for structure in by_category['structure']:
                if params_kotlin['include_structure'](structure):
                    renders.append(
                        FileRender(
                            'art/api_kotlin_structure.kt', 'java/' +
                            jni_name(structure, category='structure') + '.kt',
                            [
                                RENDER_PARAMS_BASE, params_kotlin, {
                                    'structure': structure
                                }
                            ]))
            for obj in by_category['object']:
                renders.append(
                    FileRender(
                        'art/api_kotlin_object.kt',
                        'java/' + jni_name(obj, category='object') + '.kt',
                        [RENDER_PARAMS_BASE, params_kotlin, {
                            'obj': obj
                        }]))
            for function_pointer in (by_category['function pointer'] +
                                     by_category['callback function']):
                if include_callback(function_pointer):
                    renders.append(
                        FileRender(
                            'art/api_kotlin_function_pointer.kt', 'java/' +
                            jni_name(function_pointer,
                                     category='function pointer') + '.kt',
                            [
                                RENDER_PARAMS_BASE, params_kotlin, {
                                    'function_pointer': function_pointer
                                }
                            ]))

            renders.append(
                FileRender('art/api_kotlin_exceptions.kt',
                           'java/' + kt_file_path + '/Exceptions.kt',
                           [RENDER_PARAMS_BASE, params_kotlin]))

            renders.append(
                FileRender('art/api_kotlin_functions.kt',
                           'java/' + kt_file_path + '/Functions.kt',
                           [RENDER_PARAMS_BASE, params_kotlin]))
            renders.append(
                FileRender('art/api_kotlin_callback.kt',
                           'java/' + kt_file_path + '/GPURequestCallback.kt',
                           [RENDER_PARAMS_BASE, params_kotlin]))

            for enum in (params_kotlin['by_category']['bitmask'] +
                         params_kotlin['by_category']['enum']):
                if params_kotlin['include_enum'](enum):
                    renders.append(
                        FileRender(
                            'art/api_kotlin_enum.kt',
                            'java/' + jni_name(enum, category='enum') + '.kt',
                            [
                                RENDER_PARAMS_BASE, params_kotlin, {
                                    'enum': enum
                                }
                            ]))

            renders.append(
                FileRender('art/api_kotlin_constants.kt',
                           'java/' + kt_file_path + '/Constants.kt',
                           [RENDER_PARAMS_BASE, params_kotlin]))

        if "jni" in targets:
            params_kotlin = compute_kotlin_params(loaded_json, kotlin_json,
                                                  kotlin_addins,
                                                  webgpu_kt_docs_data,
                                                  doc_warn_log_file_path)

            imported_templates += [
                "art/api_jni_types.cpp",
                "art/kotlin_record_conversion.cpp",
            ]

            renders.append(
                FileRender('art/structures.h', 'cpp/structures.h',
                           [RENDER_PARAMS_BASE, params_kotlin]))
            renders.append(
                FileRender('art/structures.cpp', 'cpp/structures.cpp',
                           [RENDER_PARAMS_BASE, params_kotlin]))
            renders.append(
                FileRender('art/methods.cpp', 'cpp/methods.cpp',
                           [RENDER_PARAMS_BASE, params_kotlin]))
            renders.append(
                FileRender('art/JNIClasses.h', 'cpp/JNIClasses.h',
                           [RENDER_PARAMS_BASE, params_kotlin]))
            renders.append(
                FileRender('art/JNIClasses.cpp', 'cpp/JNIClasses.cpp',
                           [RENDER_PARAMS_BASE, params_kotlin]))
        return GeneratorOutput(renders=renders,
                               imported_templates=imported_templates)

    def get_dependencies(self, args):
        deps = [os.path.abspath(args.dawn_json)]
        if args.wire_json != None:
            deps += [os.path.abspath(args.wire_json)]
        if args.native_json != None:
            deps += [os.path.abspath(args.native_json)]
        if args.kotlin_json != None:
            deps += [os.path.abspath(args.kotlin_json)]
        if args.webgpu_kt_docs != None:
            deps += [os.path.abspath(args.webgpu_kt_docs)]
        return deps


if __name__ == '__main__':
    sys.exit(run_generator(MultiGeneratorFromDawnJSON()))
