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

############################################################
# COMMON
############################################################
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
        self.is_builder = self.name.canonical_case().endswith(" builder")

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
    def __init__(self, name, typ, annotation, optional):
        self.name = name
        self.type = typ
        self.annotation = annotation
        self.length = None
        self.optional = optional

Method = namedtuple('Method', ['name', 'return_type', 'arguments'])
class ObjectType(Type):
    def __init__(self, name, json_data):
        Type.__init__(self, name, json_data)
        self.methods = []
        self.native_methods = []
        self.built_type = None

class StructureType(Type):
    def __init__(self, name, json_data):
        Type.__init__(self, name, json_data)
        self.extensible = json_data.get("extensible", False)
        self.members = []

############################################################
# PARSE
############################################################
import json

def is_native_method(method):
    return method.return_type.category == "natively defined" or \
        any([arg.type.category == "natively defined" for arg in method.arguments])

def linked_record_members(json_data, types):
    members = []
    members_by_name = {}
    for m in json_data:
        member = RecordMember(Name(m['name']), types[m['type']],
                              m.get('annotation', 'value'), m.get('optional', False))
        members.append(member)
        members_by_name[member.name.canonical_case()] = member

    for (member, m) in zip(members, json_data):
        if member.annotation != 'value':
            if not 'length' in m:
                if member.type.category == 'structure':
                    member.length = "constant"
                    member.constant_length = 1
                else:
                    assert(False)
            elif m['length'] == 'strlen':
                member.length = 'strlen'
            else:
                member.length = members_by_name[m['length']]

    return members


def link_object(obj, types):
    def make_method(json_data):
        arguments = linked_record_members(json_data.get('args', []), types)
        return Method(Name(json_data['name']), types[json_data.get('returns', 'void')], arguments)

    methods = [make_method(m) for m in obj.json_data.get('methods', [])]
    obj.methods = [method for method in methods if not is_native_method(method)]
    obj.native_methods = [method for method in methods if is_native_method(method)]

    # Compute the built object type for builders
    if obj.is_builder:
        for method in obj.methods:
            if method.name.canonical_case() == "get result":
                obj.built_type = method.return_type
                break
        assert(obj.built_type != None)

def link_structure(struct, types):
    struct.members = linked_record_members(struct.json_data['members'], types)

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
        'bitmask': BitmaskType,
        'enum': EnumType,
        'native': NativeType,
        'natively defined': NativelyDefined,
        'object': ObjectType,
        'structure': StructureType,
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

    return {
        'types': types,
        'by_category': by_category
    }

#############################################################
# OUTPUT
#############################################################
import re, os, sys
from collections import OrderedDict

kExtraPythonPath = '--extra-python-path'

# Try using an additional python path from the arguments if present. This
# isn't done through the regular argparse because PreprocessingLoader uses
# jinja2 in the global scope before "main" gets to run.
if kExtraPythonPath in sys.argv:
    path = sys.argv[sys.argv.index(kExtraPythonPath) + 1]
    sys.path.insert(1, path)

import jinja2

# A custom Jinja2 template loader that removes the extra indentation
# of the template blocks so that the output is correctly indented
class PreprocessingLoader(jinja2.BaseLoader):
    def __init__(self, path):
        self.path = path

    def get_source(self, environment, template):
        path = os.path.join(self.path, template)
        if not os.path.exists(path):
            raise jinja2.TemplateNotFound(template)
        mtime = os.path.getmtime(path)
        with open(path) as f:
            source = self.preprocess(f.read())
        return source, path, lambda: mtime == os.path.getmtime(path)

    blockstart = re.compile('{%-?\s*(if|for|block)[^}]*%}')
    blockend = re.compile('{%-?\s*end(if|for|block)[^}]*%}')

    def preprocess(self, source):
        lines = source.split('\n')

        # Compute the current indentation level of the template blocks and remove their indentation
        result = []
        indentation_level = 0

        for line in lines:
            # The capture in the regex adds one element per block start or end so we divide by two
            # there is also an extra line chunk corresponding to the line end, so we substract it.
            numends = (len(self.blockend.split(line)) - 1) // 2
            indentation_level -= numends

            line = self.remove_indentation(line, indentation_level)

            # Manually perform the lstrip_blocks jinja2 env options as it available starting from 2.7
            # and Travis only has Jinja 2.6
            if line.lstrip().startswith('{%'):
                line = line.lstrip()

            result.append(line)

            numstarts = (len(self.blockstart.split(line)) - 1) // 2
            indentation_level += numstarts

        return '\n'.join(result) + '\n'

    def remove_indentation(self, line, n):
        for _ in range(n):
            if line.startswith(' '):
                line = line[4:]
            elif line.startswith('\t'):
                line = line[1:]
            else:
                assert(line.strip() == '')
        return line

FileRender = namedtuple('FileRender', ['template', 'output', 'params_dicts'])

FileOutput = namedtuple('FileOutput', ['name', 'content'])

def do_renders(renders, template_dir):
    env = jinja2.Environment(loader=PreprocessingLoader(template_dir), trim_blocks=True, line_comment_prefix='//*')

    outputs = []
    for render in renders:
        params = {}
        for param_dict in render.params_dicts:
            params.update(param_dict)
        content = env.get_template(render.template).render(**params)
        outputs.append(FileOutput(render.output, content))

    return outputs

#############################################################
# MAIN SOMETHING WHATEVER
#############################################################
import argparse, sys

def as_varName(*names):
    return names[0].camelCase() + ''.join([name.CamelCase() for name in names[1:]])

def as_cType(name):
    if name.native:
        return name.concatcase()
    else:
        return 'dawn' + name.CamelCase()

def as_cppType(name):
    if name.native:
        return name.concatcase()
    else:
        return name.CamelCase()

def decorate(name, typ, arg):
    if arg.annotation == 'value':
        return typ + ' ' + name
    elif arg.annotation == 'const*':
        return typ + ' const * ' + name
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
    return 'dawn' + 'Proc' + type_name.CamelCase() + method_name.CamelCase()

def as_frontendType(typ):
    if typ.category == 'object':
        if typ.is_builder:
            return typ.name.CamelCase() + '*'
        else:
            return typ.name.CamelCase() + 'Base*'
    elif typ.category in ['bitmask', 'enum']:
        return 'dawn::' + typ.name.CamelCase()
    elif typ.category == 'structure':
        return as_cppType(typ.name)
    else:
        return as_cType(typ.name)

def cpp_native_methods(types, typ):
    methods = typ.methods + typ.native_methods

    if typ.is_builder:
        methods.append(Method(Name('set error callback'), types['void'], [
            RecordMember(Name('callback'), types['builder error callback'], 'value', False),
            RecordMember(Name('userdata1'), types['callback userdata'], 'value', False),
            RecordMember(Name('userdata2'), types['callback userdata'], 'value', False),
        ]))

    return methods

def c_native_methods(types, typ):
    return cpp_native_methods(types, typ) + [
        Method(Name('reference'), types['void'], []),
        Method(Name('release'), types['void'], []),
    ]

def js_native_methods(types, typ):
    return cpp_native_methods(types, typ)

def debug(text):
    print(text)

def get_renders_for_targets(api_params, targets):
    base_params = {
        'enumerate': enumerate,
        'format': format,
        'len': len,
        'debug': debug,

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
        'as_varName': as_varName,
        'decorate': decorate,
    }

    renders = []

    c_params = {'native_methods': lambda typ: c_native_methods(api_params['types'], typ)}
    cpp_params = {'native_methods': lambda typ: cpp_native_methods(api_params['types'], typ)}

    if 'dawn_headers' in targets:
        renders.append(FileRender('api.h', 'dawn/dawn.h', [base_params, api_params, c_params]))
        renders.append(FileRender('apicpp.h', 'dawn/dawncpp.h', [base_params, api_params, cpp_params]))
        renders.append(FileRender('apicpp_traits.h', 'dawn/dawncpp_traits.h', [base_params, api_params, cpp_params]))

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
        wire_params = [
            base_params,
            api_params,
            c_params,
            {
                'as_wireType': lambda typ: typ.name.CamelCase() + '*' if typ.category == 'object' else as_cppType(typ.name)
            }
        ]
        renders.append(FileRender('dawn_wire/WireCmd.h', 'dawn_wire/WireCmd_autogen.h', wire_params))
        renders.append(FileRender('dawn_wire/WireCmd.cpp', 'dawn_wire/WireCmd_autogen.cpp', wire_params))
        renders.append(FileRender('dawn_wire/WireClient.cpp', 'dawn_wire/WireClient.cpp', wire_params))
        renders.append(FileRender('dawn_wire/WireServer.cpp', 'dawn_wire/WireServer.cpp', wire_params))

    return renders

def output_to_json(outputs, output_json):
    json_root = {}
    for output in outputs:
        json_root[output.name] = output.content

    with open(output_json, 'w') as f:
        f.write(json.dumps(json_root))

def output_depfile(depfile, output, dependencies):
    with open(depfile, 'w') as f:
        f.write(output + ": " + " ".join(dependencies))

def main():
    allowed_targets = ['dawn_headers', 'libdawn', 'mock_dawn', 'dawn_wire', "dawn_native_utils"]

    parser = argparse.ArgumentParser(
        description = 'Generates code for various target for Dawn.',
        formatter_class = argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument('json', metavar='DAWN_JSON', nargs=1, type=str, help ='The DAWN JSON definition to use.')
    parser.add_argument('-t', '--template-dir', default='templates', type=str, help='Directory with template files.')
    parser.add_argument('-T', '--targets', required=True, type=str, help='Comma-separated subset of targets to output. Available targets: ' + ', '.join(allowed_targets))
    parser.add_argument(kExtraPythonPath, default=None, type=str, help='Additional python path to set before loading Jinja2')
    parser.add_argument('--output-json-tarball', default=None, type=str, help='Name of the "JSON tarball" to create (tar is too annoying to use in python).')
    parser.add_argument('--depfile', default=None, type=str, help='Name of the Ninja depfile to create for the JSON tarball')
    parser.add_argument('--expected-outputs-file', default=None, type=str, help="File to compare outputs with and fail if it doesn't match")

    args = parser.parse_args()

    # Load and parse the API json file
    with open(args.json[0]) as f:
        loaded_json = json.loads(f.read())
    api_params = parse_json(loaded_json)

    targets = args.targets.split(',')
    renders = get_renders_for_targets(api_params, targets)

    # The caller wants to assert that the outputs are what it expects.
    # Load the file and compare with our renders.
    if args.expected_outputs_file != None:
        with open(args.expected_outputs_file) as f:
            expected = set([line.strip() for line in f.readlines()])

        actual = set()
        actual.update([render.output for render in renders])

        if actual != expected:
            print("Wrong expected outputs, caller expected:\n    " + repr(list(expected)))
            print("Actual output:\n    " + repr(list(actual)))
            return 1

    outputs = do_renders(renders, args.template_dir)

    # Output the tarball and its depfile
    if args.output_json_tarball != None:
        output_to_json(outputs, args.output_json_tarball)

        dependencies = [args.template_dir + os.path.sep + render.template for render in renders]
        dependencies.append(args.json[0])
        output_depfile(args.depfile, args.output_json_tarball, dependencies)

if __name__ == '__main__':
    sys.exit(main())
