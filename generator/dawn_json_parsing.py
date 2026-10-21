# Copyright 2026 The Dawn & Tint Authors
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

import json

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

    def __lt__(self, other):
        return self.concatcase().lower() < other.concatcase().lower()

    def get(self):
        return self.name

    def CamelChunk(self, chunk):
        return chunk[0].upper() + chunk[1:]

    def canonical_case(self):
        return ' '.join(self.chunks)

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


def _validate_and_get_tags(json_data):
    allowed_tags = {
        'dawn',
        'emscripten',
        'native',
        'deprecated',
        'art',
        'art_experimental',
    }

    tags = json_data.get('tags')
    if tags != None:
        for tag in tags:
            assert tag in allowed_tags, f'unrecognized tag "{tag}"'
    return tags


def _item_is_enabled(enabled_tags, json_data):
    tags = _validate_and_get_tags(json_data)
    if tags is None: return True

    # Strip 'art_experimental' for non-Art targets so it doesn't cause
    # the item to be excluded from C++/Emscripten builds.
    if not ('art_experimental' in enabled_tags):
        original_tags_empty = not tags
        tags = [tag for tag in tags if tag not in ('art_experimental')]
        # NOTE: If an item is tagged ONLY with 'art_experimental', it is disabled
        # for non-Art targets.
        if not tags and not original_tags_empty:
            return False
        if not tags:
            return True

    return any(tag in enabled_tags for tag in tags)


def _item_is_disabled(disabled_tags, json_data):
    if disabled_tags is None: return False
    tags = _validate_and_get_tags(json_data)
    if tags is None: return False

    return any(tag in disabled_tags for tag in tags)


class Type:

    def __init__(self, name, json_data, native=False):
        self.json_data = json_data
        self.dict_name = name
        self.name = Name(name, native=native)
        self.category = json_data['category']
        self.is_nullable_pointer = json_data.get('is nullable pointer',
                                                 self.category == 'object')
        self.is_wire_transparent = False

    def __lt__(self, other):
        return self.name < other.name

    def get_child_map(self):
        return {}


class EnumValue:

    def __init__(self, name, value, valid, json_data):
        self.name = name
        self.value = value
        self.valid = valid
        self.json_data = json_data

    def get_child_map(self):
        return {}


class EnumType(Type):

    def __init__(self, is_enabled, name, json_data):
        Type.__init__(self, name, json_data)

        self.values = []
        self.hasUndefined = False
        self.contiguous = True
        self.startValue = None
        self.allowConflict = False
        lastValue = None
        for m in self.json_data['values']:
            if not is_enabled(m):
                continue
            value = m['value']
            value_name = m['name']
            tags = _validate_and_get_tags(m)
            if tags == None:
                tags = []

            prefix = 0

            if 'dawn' in tags:
                # Dawn-only or Dawn+Emscripten
                assert prefix == 0
                prefix = 0x0005_0000
            elif 'emscripten' in tags:
                # Emscripten-only
                assert prefix == 0
                prefix = 0x0004_0000

            if prefix == 0 and 'native' in tags:
                prefix = 0x0001_0000

            if 'deprecated' not in tags:
                # Emscripten implements some Dawn extensions, and some upstream things that
                # aren't in Dawn yet.
                if 'emscripten' in tags and 'dawn' not in tags:
                    assert value_name.startswith('emscripten'), name
                else:
                    assert not value_name.startswith('emscripten'), name

            value += prefix

            if value_name == "undefined":
                self.hasUndefined = True
            if lastValue == None:
                self.startValue = value
            elif value != lastValue + 1:
                self.contiguous = False
            lastValue = value
            self.values.append(
                EnumValue(Name(value_name), value, m.get('valid', True), m))
            self.allowConflict |= m.get('enum_value_conflict', False)

        # Assert that all values except those with "enum_value_conflict": true are unique in enums
        all_values = set()
        for value in self.values:
            if value.value in all_values:
                if not value.json_data.get('enum_value_conflict', False):
                    raise Exception(
                        "Duplicate value {} for '{}' in enum '{}'".format(
                            hex(value.value), value.name.get(), name))
            all_values.add(value.value)
        self.is_wire_transparent = True

    def get_child_map(self):
        return {value.name.get(): value for value in self.values}


class BitmaskValue:

    def __init__(self, name, value, json_data):
        self.name = name
        self.value = value
        self.json_data = json_data

    def get_child_map(self):
        return {}


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

    def get_child_map(self):
        return {value.name.get(): value for value in self.values}


class CallbackFunctionType(Type):

    def __init__(self, is_enabled, name, json_data):
        Type.__init__(self, name, json_data)
        self.returns = None
        self.arguments = []


class FunctionPointerType(Type):

    def __init__(self, is_enabled, name, json_data):
        Type.__init__(self, name, json_data)
        self.returns = None
        self.arguments = []


class TypedefType(Type):

    def __init__(self, is_enabled, name, json_data):
        Type.__init__(self, name, json_data)
        self.type = None


class NativeType(Type):

    def __init__(self, is_enabled, name, json_data):
        Type.__init__(self, name, json_data, native=True)
        self.is_wire_transparent = json_data.get('wire transparent', True)


# Method/function argument, method/function return value, or struct member.
class AnnotatedTypedMember:

    def __init__(self, typ, annotation, optional, json_data):
        self.type = typ
        self.annotation = annotation
        self.optional = optional
        self.json_data = json_data
        self.length = None
        self.constant_length = None
        self.is_length = False

    def get_child_map(self):
        return {}


# Methods and structures are both "records", so record members correspond to
# method arguments or structure members.
class RecordMember(AnnotatedTypedMember):

    def __init__(self,
                 name,
                 typ,
                 annotation,
                 json_data,
                 optional=False,
                 array_element_optional=False,
                 is_return_value=False,
                 default_value=None,
                 skip_serialize=False,
                 kotlin_only=False):
        super().__init__(typ, annotation, optional, json_data)
        self.name = name
        self.array_element_optional = array_element_optional
        if array_element_optional:
            assert annotation == 'const*', 'array_element_optional can only be used on array types'
        self.is_return_value = is_return_value
        self.handle_type = None
        self.id_type = None
        self.default_value = default_value
        self.skip_serialize = skip_serialize
        self.kotlin_only = kotlin_only

    def set_handle_type(self, handle_type):
        assert self.type.dict_name == "ObjectHandle"
        self.handle_type = handle_type

    def set_id_type(self, id_type):
        assert self.type.dict_name == "ObjectId"
        self.id_type = id_type

    @property
    def requires_struct_defaulting(self):
        if self.annotation != "value":
            return False

        if self.type.category == "structure":
            return self.type.any_member_requires_struct_defaulting
        elif self.type.category == "enum":
            return (self.type.hasUndefined
                    and self.default_value not in [None, "undefined"])
        else:
            return False


class Method():

    def __init__(self, name, returns, arguments, autolock, json_data):
        self.name = name
        self.returns = returns
        self.arguments = arguments
        self.autolock = autolock
        self.json_data = json_data

    def get_child_map(self):
        return {arg.name.get(): arg for arg in self.arguments}


class ObjectType(Type):

    def __init__(self, is_enabled, name, json_data):
        json_data_override = {'methods': []}
        if 'methods' in json_data:
            json_data_override['methods'] = [
                m for m in json_data['methods'] if is_enabled(m)
            ]
        Type.__init__(self, name, dict(json_data, **json_data_override))

    def get_child_map(self):
        return {method.name.get(): method for method in self.methods}


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
        tags = _validate_and_get_tags(json_data)
        if tags == ['emscripten']:
            if name != 'INTERNAL_HAVE_EMDAWNWEBGPU_HEADER':
                assert name.startswith('emscripten'), name
        else:
            assert not name.startswith('emscripten'), name

        Record.__init__(self, name)
        json_data_override = {}
        if 'members' in json_data:
            json_data_override['members'] = [
                m for m in json_data['members'] if is_enabled(m)
            ]
        Type.__init__(self, name, dict(json_data, **json_data_override))
        self.out = json_data.get('out', False)
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

    def get_child_map(self):
        return {member.name.get(): member for member in self.members}

    @property
    def output(self):
        # self.out is a temporary way to express that this is an output structure
        # without also making it extensible. See
        # https://dawn-review.googlesource.com/c/dawn/+/212174/comment/2271690b_1fd82ea9/
        return self.chained == "out" or self.extensible == "out" or self.out

    @property
    def has_free_members_function(self):
        if not self.output:
            return False
        for m in self.members:
            if m.annotation != 'value' \
                or m.type.name.canonical_case() == 'string view':
                return True
        return False

    @property
    def any_member_requires_struct_defaulting(self):
        return any(member.requires_struct_defaulting
                   for member in self.members)


class CallbackInfoType(StructureType):

    def __init__(self, is_enabled, name, json_data):
        StructureType.__init__(self, is_enabled, name, json_data)
        self.extensible = 'in'


class ConstantDefinition():

    def __init__(self, is_enabled, name, json_data):
        self.type = None
        self.value = json_data['value']
        self.cpp_value = json_data.get('cpp_value', None)
        self.json_data = json_data
        self.name = Name(name)

    def get_child_map(self):
        return {}


class FunctionDeclaration():

    def __init__(self, is_enabled, name, json_data, no_cpp=False):
        self.returns = None
        self.arguments = []
        self.json_data = json_data
        self.name = Name(name)
        self.no_cpp = no_cpp

    def get_child_map(self):
        return {arg.name.get(): arg for arg in self.arguments}


# TODO(crbug.com/575961728): Make this private (_linked_record_members) if its
# usage for Dawn wire in dawn_json_generator.py can be moved/removed.
def linked_record_members(json_data, types, check_span_regularity=False):
    members = []
    members_by_name = {}
    index_of_member = {}
    for (i, m) in enumerate(json_data):
        member = RecordMember(Name(m['name']),
                              types[m['type']],
                              m.get('annotation', 'value'),
                              m,
                              optional=m.get('optional', False),
                              array_element_optional=m.get(
                                  'array_element_optional', False),
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
        index_of_member[member.name.canonical_case()] = i

    for (member, m) in zip(members, json_data):
        if member.annotation != 'value':
            if not 'length' in m:
                if member.type.category != 'object':
                    member.length = "constant"
                    member.constant_length = 1
                else:
                    assert False
            elif isinstance(m['length'], int):
                assert m['length'] > 0
                member.length = "constant"
                member.constant_length = m['length']
            else:
                # Check that the length member comes just before the `member`
                if check_span_regularity:
                    length_index = index_of_member[m['length']]
                    member_index = index_of_member[
                        member.name.canonical_case()]
                    assert length_index == member_index - 1

                member.length = members_by_name[m['length']]
                member.length.is_length = True

    return members


############################################################
# PARSE
############################################################


def _link_object(obj, types):
    # Disable method's autolock if obj's "no autolock" = True
    obj_scoped_autolock_enabled = not obj.json_data.get('no autolock', False)

    def make_method(json_data):
        autolock_enabled = obj_scoped_autolock_enabled and not json_data.get(
            'no autolock', False)
        # _link_function sets 'returns' and 'arguments'
        method = Method(Name(json_data['name']), None, None, autolock_enabled,
                        json_data)
        _link_function(method, types)
        return method

    obj.methods = [make_method(m) for m in obj.json_data.get('methods', [])]
    obj.methods.sort(key=lambda method: method.name)


def _link_structure(struct, types):
    struct.members = linked_record_members(struct.json_data['members'],
                                           types,
                                           check_span_regularity=True)
    for root in struct.json_data.get('chain roots', []):
        struct.chain_roots.append(types[root])
        types[root].extensions.append(struct)
    struct.chain_roots = [
        types[root] for root in struct.json_data.get('chain roots', [])
    ]
    assert all((root.category == 'structure' for root in struct.chain_roots))


def _link_function_pointer(function_pointer, types):
    _link_function(function_pointer, types)


def _link_typedef(typedef, types):
    typedef.type = types[typedef.json_data['type']]


def _link_constant(constant, types):
    constant.type = types[constant.json_data['type']]
    assert constant.type.name.native


def _link_function(function, types):
    # "returns" may be either a bare type name or a AnnotatedTypedMember-like.
    returns = function.json_data.get('returns')
    assert returns != 'void', '"returns": "void" should be omitted instead'
    if returns:
        if type(returns) == str:
            returns = {'type': returns}
        function.returns = AnnotatedTypedMember(
            types[returns['type']], returns.get('annotation', 'value'),
            returns.get('optional', False), returns)

    function.arguments = linked_record_members(
        function.json_data.get('args', []), types)


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
def _topo_sort_structure(structs):
    for struct in structs:
        struct.visited = False
        struct.subdag_depth = 0
        # String view is special cased to -1 because we purposely fully declare
        # it before all other structs.
        if struct.name.get() == "string view":
            struct.subdag_depth = -1
            struct.visited = True

    def compute_depth(struct):
        if struct.visited:
            return struct.subdag_depth

        max_dependent_depth = 0
        for member in struct.members:
            if member.type.category == 'structure':
                max_dependent_depth = max(max_dependent_depth,
                                          compute_depth(member.type) + 1)
        for extension in struct.extensions:
            max_dependent_depth = max(max_dependent_depth,
                                      compute_depth(extension) + 1)

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


# Sort objects so that if object A has a member function with an optional
# argument of type object B, then B is listed before A. We do this because
# optional object arguments are emitted as 'Func(B const& arg = nullptr)`, and
# this requires that B's definition be known so that the compiler can generate
# the call to B's implicit std::nullptr constructor.
#
# See the comment for _topo_sort_structure for how this sorts topologically.
def _topo_sort_object(objects):
    for object in objects:
        object.visited = False
        object.subdag_depth = 0

    def compute_depth(object):
        if object.visited:
            return object.subdag_depth

        max_dependent_depth = 0
        for method in object.methods:
            for arg in method.arguments:
                if arg.type.category == 'object':
                    if arg.optional:
                        max_dependent_depth = max(max_dependent_depth,
                                                  compute_depth(arg.type) + 1)

        object.subdag_depth = max_dependent_depth
        object.visited = True
        return object.subdag_depth

    for object in objects:
        compute_depth(object)

    result = sorted(objects, key=lambda object: object.subdag_depth)

    for object in objects:
        del object.visited
        del object.subdag_depth

    return result


def _apply_addins(types, addins):

    def get_addin_target(search_map, path):
        while path:
            part = path.pop(0)
            if part not in search_map:
                return None
            target = search_map[part]
            if not path:
                return target
            search_map = target.get_child_map()
        return None

    for key, prop_dict in addins.items():
        path = key.split('::')
        target = get_addin_target(types, path)
        assert target is not None, f'Addin instance "{key}" not found in dawn.json'
        for prop, val in prop_dict.items():
            setattr(target, prop, val)


def parse_addin(json_filepath):
    with open(json_filepath) as f:
        content = f.read().strip()
        if not content:
            return {}
        return json.loads(content).get('metadata', {}).get('addins', {})


def parse_json(json, enabled_tags, disabled_tags=None, addins=None):
    is_enabled = lambda json_data: _item_is_enabled(
        enabled_tags, json_data) and not _item_is_disabled(
            disabled_tags, json_data)
    category_to_parser = {
        'bitmask': BitmaskType,
        'callback function': CallbackFunctionType,
        'callback info': CallbackInfoType,
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
        _link_object(obj, types)

    for struct in by_category['structure']:
        _link_structure(struct, types)

    for callback_info in by_category['callback info']:
        _link_structure(callback_info, types)

    for callback_function in by_category['callback function']:
        _link_function_pointer(callback_function, types)

    for function_pointer in by_category['function pointer']:
        _link_function_pointer(function_pointer, types)

    for typedef in by_category['typedef']:
        _link_typedef(typedef, types)

    for constant in by_category['constant']:
        _link_constant(constant, types)

    for function in by_category['function']:
        _link_function(function, types)

    # Sort everything by name
    for category in by_category.keys():
        by_category[category] = sorted(by_category[category],
                                       key=lambda typ: typ.name)
    # Then sort GetProcAddress last
    by_category['function'].sort(
        key=lambda f: f.name.get() == 'get proc address')

    by_category['structure'] = _topo_sort_structure(by_category['structure'])
    by_category['object'] = _topo_sort_object(by_category['object'])

    for struct in by_category['structure']:
        struct.update_metadata()

    for addin in (addins or []):
        _apply_addins(types, addin)

    return {
        'metadata': Metadata(json['_metadata']),
        'types': types,
        'by_category': by_category,
        'enabled_tags': enabled_tags,
        'disabled_tags': disabled_tags,
    }
