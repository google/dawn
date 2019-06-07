#!/usr/bin/env python2
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

import json, os, sys
from collections import namedtuple

import common
from common import Name
from generator_lib import Generator, run_generator, FileRender
import wire_cmd

############################################################
# PARSE
############################################################

def is_native_method(method):
    return method.return_type.category == "natively defined" or \
        any([arg.type.category == "natively defined" for arg in method.arguments])

def link_object(obj, types):
    def make_method(json_data):
        arguments = common.linked_record_members(json_data.get('args', []), types)
        return common.Method(Name(json_data['name']), types[json_data.get('returns', 'void')], arguments)

    methods = [make_method(m) for m in obj.json_data.get('methods', [])]
    obj.methods = [method for method in methods if not is_native_method(method)]
    obj.native_methods = [method for method in methods if is_native_method(method)]

def link_structure(struct, types):
    struct.members = common.linked_record_members(struct.json_data['members'], types)

# Sort structures so that if struct A has struct B as a member, then B is listed before A
# This is a form of topological sort where we try to keep the order reasonably similar to the
# original order (though th sort isn't technically stable).
# It works by computing for each struct type what is the depth of its DAG of dependents, then
# resorting based on that depth using Python's stable sort. This makes a toposort because if
# A depends on B then its depth will be bigger than B's. It is also nice because all nodes
# with the same depth are kept in the input order.
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
                max_dependent_depth = max(max_dependent_depth, compute_depth(member.type) + 1)

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

def parse_json(json):
    category_to_parser = {
        'bitmask': common.BitmaskType,
        'enum': common.EnumType,
        'native': common.NativeType,
        'natively defined': common.NativelyDefined,
        'object': common.ObjectType,
        'structure': common.StructureType,
    }

    types = {}

    by_category = {}
    for name in category_to_parser.keys():
        by_category[name] = []

    for (name, json_data) in json.items():
        if name[0] == '_':
            continue
        category = json_data['category']
        parsed = category_to_parser[category](name, json_data)
        by_category[category].append(parsed)
        types[name] = parsed

    for obj in by_category['object']:
        link_object(obj, types)

    for struct in by_category['structure']:
        link_structure(struct, types)

    for category in by_category.keys():
        by_category[category] = sorted(by_category[category], key=lambda typ: typ.name.canonical_case())

    by_category['structure'] = topo_sort_structure(by_category['structure'])

    for struct in by_category['structure']:
        struct.update_metadata()

    return {
        'types': types,
        'by_category': by_category
    }

#############################################################
# Generator
#############################################################

def as_varName(*names):
    return names[0].camelCase() + ''.join([name.CamelCase() for name in names[1:]])

def as_cType(name):
    if name.native:
        return name.concatcase()
    else:
        return 'Dawn' + name.CamelCase()

def as_cppType(name):
    if name.native:
        return name.concatcase()
    else:
        return name.CamelCase()

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
                    '{}.{}'.format(arg, as_varName(member.name)),
                    indent + 1)
                for member in typ.members]

            converted_members = [(' ' * 4) + m for m in converted_members ]
            converted_members = ',\n'.join(converted_members)

            return as_cppType(typ.name) + ' {\n' + converted_members + '\n}'
        else:
            return 'static_cast<{}>({})'.format(as_cppType(typ.name), arg)
    else:
        return 'reinterpret_cast<{} {}>({})'.format(as_cppType(typ.name), annotation, arg)

def decorate(name, typ, arg):
    if arg.annotation == 'value':
        return typ + ' ' + name
    elif arg.annotation == '*':
        return typ + ' * ' + name
    elif arg.annotation == 'const*':
        return typ + ' const * ' + name
    elif arg.annotation == 'const*const*':
        return 'const ' + typ + '* const * ' + name
    else:
        assert(False)

def annotated(typ, arg):
    name = as_varName(arg.name)
    return decorate(name, typ, arg)

def as_cEnum(type_name, value_name):
    assert(not type_name.native and not value_name.native)
    return 'DAWN' + '_' + type_name.SNAKE_CASE() + '_' + value_name.SNAKE_CASE()

def as_cppEnum(value_name):
    assert(not value_name.native)
    if value_name.concatcase()[0].isdigit():
        return "e" + value_name.CamelCase()
    return value_name.CamelCase()

def as_cMethod(type_name, method_name):
    assert(not type_name.native and not method_name.native)
    return 'dawn' + type_name.CamelCase() + method_name.CamelCase()

def as_MethodSuffix(type_name, method_name):
    assert(not type_name.native and not method_name.native)
    return type_name.CamelCase() + method_name.CamelCase()

def as_cProc(type_name, method_name):
    assert(not type_name.native and not method_name.native)
    return 'Dawn' + 'Proc' + type_name.CamelCase() + method_name.CamelCase()

def as_frontendType(typ):
    if typ.category == 'object':
        return typ.name.CamelCase() + 'Base*'
    elif typ.category in ['bitmask', 'enum']:
        return 'dawn::' + typ.name.CamelCase()
    elif typ.category == 'structure':
        return as_cppType(typ.name)
    else:
        return as_cType(typ.name)

def cpp_native_methods(types, typ):
    return typ.methods + typ.native_methods

def c_native_methods(types, typ):
    return cpp_native_methods(types, typ) + [
        common.Method(Name('reference'), types['void'], []),
        common.Method(Name('release'), types['void'], []),
    ]

def js_native_methods(types, typ):
    return cpp_native_methods(types, typ)

def debug(text):
    print(text)

def do_assert(expr):
    assert expr
    return ''

class MultiGeneratorFromDawnJSON(Generator):
    def get_description(self):
        return 'Generates code for various target from Dawn.json.'

    def add_commandline_arguments(self, parser):
        allowed_targets = ['dawn_headers', 'libdawn', 'mock_dawn', 'dawn_wire', "dawn_native_utils"]

        parser.add_argument('--dawn-json', required=True, type=str, help ='The DAWN JSON definition to use.')
        parser.add_argument('--wire-json', default=None, type=str, help='The DAWN WIRE JSON definition to use.')
        parser.add_argument('-T', '--targets', required=True, type=str, help='Comma-separated subset of targets to output. Available targets: ' + ', '.join(allowed_targets))

    def get_file_renders(self, args):
        with open(args.dawn_json) as f:
            loaded_json = json.loads(f.read())
        api_params = parse_json(loaded_json)

        targets = args.targets.split(',')

        wire_json = None
        if args.wire_json:
            with open(args.wire_json) as f:
                wire_json = json.loads(f.read())

        base_params = {
            'enumerate': enumerate,
            'format': format,
            'len': len,
            'debug': debug,
            'assert': do_assert,

            'Name': lambda name: Name(name),

            'as_annotated_cType': lambda arg: annotated(as_cType(arg.type.name), arg),
            'as_annotated_cppType': lambda arg: annotated(as_cppType(arg.type.name), arg),
            'as_cEnum': as_cEnum,
            'as_cppEnum': as_cppEnum,
            'as_cMethod': as_cMethod,
            'as_MethodSuffix': as_MethodSuffix,
            'as_cProc': as_cProc,
            'as_cType': as_cType,
            'as_cppType': as_cppType,
            'convert_cType_to_cppType': convert_cType_to_cppType,
            'as_varName': as_varName,
            'decorate': decorate,
        }

        renders = []

        c_params = {'native_methods': lambda typ: c_native_methods(api_params['types'], typ)}
        cpp_params = {'native_methods': lambda typ: cpp_native_methods(api_params['types'], typ)}

        if 'dawn_headers' in targets:
            renders.append(FileRender('api.h', 'dawn/dawn.h', [base_params, api_params, c_params]))
            renders.append(FileRender('apicpp.h', 'dawn/dawncpp.h', [base_params, api_params, cpp_params]))

        if 'libdawn' in targets:
            additional_params = {'native_methods': lambda typ: cpp_native_methods(api_params['types'], typ)}
            renders.append(FileRender('api.c', 'dawn/dawn.c', [base_params, api_params, c_params]))
            renders.append(FileRender('apicpp.cpp', 'dawn/dawncpp.cpp', [base_params, api_params, cpp_params]))

        if 'mock_dawn' in targets:
            renders.append(FileRender('mock_api.h', 'mock/mock_dawn.h', [base_params, api_params, c_params]))
            renders.append(FileRender('mock_api.cpp', 'mock/mock_dawn.cpp', [base_params, api_params, c_params]))

        if 'dawn_native_utils' in targets:
            frontend_params = [
                base_params,
                api_params,
                c_params,
                {
                    'as_frontendType': lambda typ: as_frontendType(typ), # TODO as_frontendType and friends take a Type and not a Name :(
                    'as_annotated_frontendType': lambda arg: annotated(as_frontendType(arg.type), arg)
                }
            ]

            renders.append(FileRender('dawn_native/ValidationUtils.h', 'dawn_native/ValidationUtils_autogen.h', frontend_params))
            renders.append(FileRender('dawn_native/ValidationUtils.cpp', 'dawn_native/ValidationUtils_autogen.cpp', frontend_params))
            renders.append(FileRender('dawn_native/api_structs.h', 'dawn_native/dawn_structs_autogen.h', frontend_params))
            renders.append(FileRender('dawn_native/api_structs.cpp', 'dawn_native/dawn_structs_autogen.cpp', frontend_params))
            renders.append(FileRender('dawn_native/ProcTable.cpp', 'dawn_native/ProcTable.cpp', frontend_params))

        if 'dawn_wire' in targets:
            additional_params = wire_cmd.compute_wire_params(api_params, wire_json)

            wire_params = [
                base_params,
                api_params,
                c_params,
                {
                    'as_wireType': lambda typ: typ.name.CamelCase() + '*' if typ.category == 'object' else as_cppType(typ.name)
                },
                additional_params
            ]
            renders.append(FileRender('dawn_wire/WireCmd.h', 'dawn_wire/WireCmd_autogen.h', wire_params))
            renders.append(FileRender('dawn_wire/WireCmd.cpp', 'dawn_wire/WireCmd_autogen.cpp', wire_params))
            renders.append(FileRender('dawn_wire/client/ApiObjects.h', 'dawn_wire/client/ApiObjects_autogen.h', wire_params))
            renders.append(FileRender('dawn_wire/client/ApiProcs.cpp', 'dawn_wire/client/ApiProcs_autogen.cpp', wire_params))
            renders.append(FileRender('dawn_wire/client/ApiProcs.h', 'dawn_wire/client/ApiProcs_autogen.h', wire_params))
            renders.append(FileRender('dawn_wire/client/ClientBase.h', 'dawn_wire/client/ClientBase_autogen.h', wire_params))
            renders.append(FileRender('dawn_wire/client/ClientHandlers.cpp', 'dawn_wire/client/ClientHandlers_autogen.cpp', wire_params))
            renders.append(FileRender('dawn_wire/client/ClientPrototypes.inc', 'dawn_wire/client/ClientPrototypes_autogen.inc', wire_params))
            renders.append(FileRender('dawn_wire/server/ServerBase.h', 'dawn_wire/server/ServerBase_autogen.h', wire_params))
            renders.append(FileRender('dawn_wire/server/ServerDoers.cpp', 'dawn_wire/server/ServerDoers_autogen.cpp', wire_params))
            renders.append(FileRender('dawn_wire/server/ServerHandlers.cpp', 'dawn_wire/server/ServerHandlers_autogen.cpp', wire_params))
            renders.append(FileRender('dawn_wire/server/ServerPrototypes.inc', 'dawn_wire/server/ServerPrototypes_autogen.inc', wire_params))

        return renders

    def get_dependencies(self, args):
        deps = [os.path.abspath(args.dawn_json)]
        if args.wire_json != None:
            deps += [os.path.abspath(args.wire_json)]
        return deps

if __name__ == '__main__':
    sys.exit(run_generator(MultiGeneratorFromDawnJSON()))
