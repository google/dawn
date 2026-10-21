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
"""Resolves an intrinsic definition AST into a semantic model (Sem)."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Optional

import def_ast as ast
import sem
import tok


class ResolveError(Exception):
    """Raised when semantic resolution fails."""


def resolve(a: ast.AST) -> sem.Sem:
    """Processes the AST and returns the resolved Sem."""
    r = _Resolver(a)
    r.resolve()
    return r.sem


@dataclass
class _ObjectAndSource:
    object: sem.Node
    source: tok.Source


class _Scope:
    """A hierarchical name-to-object symbol table."""

    def __init__(self, parent: Optional[_Scope] = None):
        self.objects: dict[str, _ObjectAndSource] = {}
        self.parent = parent

    def lookup(self, name: str) -> Optional[_ObjectAndSource]:
        if name in self.objects:
            return self.objects[name]
        if self.parent is None:
            return None
        return self.parent.lookup(name)

    def declare(self, obj: sem.Node, source: tok.Source) -> None:
        name = obj.get_name()
        existing = self.lookup(name)
        if existing is not None:
            raise ResolveError(
                f"{source} '{name}' already declared\nFirst declared here: {existing.source}"
            )
        self.objects[name] = _ObjectAndSource(object=obj, source=source)


class _Resolver:

    def __init__(self, ast: ast.AST):
        self.ast = ast
        self.sem = sem.Sem()
        self.globals = _Scope()
        self.builtins: dict[str, sem.Intrinsic] = {}
        self.unary_operators: dict[str, sem.Intrinsic] = {}
        self.binary_operators: dict[str, sem.Intrinsic] = {}
        self.constructors_and_converters: dict[str, sem.Intrinsic] = {}

    def resolve(self) -> None:
        usages = ast.EnumDecl()

        for enum in self.ast.enums:
            self._enum(enum)
            if enum.name == "usages":
                usages = enum
        for type in self.ast.types:
            self._ty(type)
        for matcher in self.ast.matchers:
            self._matcher(matcher)
        for builtin in self.ast.builtins:
            self._intrinsic(builtin, self.builtins, self.sem.builtins)
        for op in self.ast.operators:
            if len(op.parameters) == 1:
                self._intrinsic(op, self.unary_operators,
                                self.sem.unary_operators)
            elif len(op.parameters) == 2:
                self._intrinsic(op, self.binary_operators,
                                self.sem.binary_operators)
            else:
                raise ResolveError(
                    f"{op.source} operators must have either 1 or 2 parameters"
                )
        for construct in self.ast.constructors:
            self._intrinsic(
                construct,
                self.constructors_and_converters,
                self.sem.constructors_and_converters,
            )
        for convert in self.ast.converters:
            if len(convert.parameters) != 1:
                raise ResolveError(
                    f"{convert.source} conversions must have a single parameter"
                )
            self._intrinsic(
                convert,
                self.constructors_and_converters,
                self.sem.constructors_and_converters,
            )

        self.sem.unique_parameter_names = self._calculate_unique_parameter_names(
            usages)

    def _enum(self, e: ast.EnumDecl) -> None:
        attrs = ast.Attributes(e.attributes)
        ns = "core"
        ns_attr = attrs.take("ns")
        if ns_attr is not None:
            ns = str(ns_attr.values[0])
        if attrs:
            raise ResolveError(
                f"{attrs[0].source} unknown attribute: {attrs[0].values[0]}")

        enum = sem.Enum(decl=e, name=e.name, ns=ns)
        self.sem.enums.append(enum)
        self.globals.declare(enum, e.source)

        names: set[str] = set()
        for ast_entry in e.entries:
            entry_attrs = ast.Attributes(ast_entry.attributes)
            entry = sem.EnumEntry(enum, name=ast_entry.name)
            internal = entry_attrs.take("internal")
            if internal is not None:
                entry.is_internal = True
                if internal.values:
                    raise ResolveError(
                        f"{ast_entry.source} unexpected value for internal attribute"
                    )
            if entry_attrs:
                raise ResolveError(
                    f"{entry_attrs[0].source} unknown attribute")
            enum.entries.append(entry)
            if ast_entry.name in names:
                raise ResolveError(
                    f"{ast_entry.source} duplicate enum entry '{ast_entry.name}'"
                )
            names.add(ast_entry.name)

        enum.entries.sort(key=lambda entry: entry.name)

    def _ty(self, a: ast.TypeDecl) -> None:
        attrs = ast.Attributes(a.attributes)
        type = sem.Type(decl=a, name=a.name)
        self.sem.types.append(type)
        self.globals.declare(type, a.source)

        scope = _Scope(self.globals)
        type.template_params = self._template_params(scope, a.template_params)

        display_attr = attrs.take("display")
        if display_attr is not None:
            if len(display_attr.values) != 1:
                raise ResolveError(
                    f"{display_attr.source} expected a single value for 'display' attribute"
                )
            type.display_name = str(display_attr.values[0])

        prec_attr = attrs.take("precedence")
        if prec_attr is not None:
            if len(prec_attr.values) != 1:
                raise ResolveError(
                    f"{prec_attr.source} expected a single integer value for 'precedence' attribute"
                )
            val = prec_attr.values[0]
            if not isinstance(val, int) or isinstance(val, bool):
                raise ResolveError(
                    f"{prec_attr.source} @precedence value must be an integer")
            type.precedence = val

        if attrs:
            raise ResolveError(f"{attrs[0].source} unknown attribute")

    def _matcher(self, a: ast.MatcherDecl) -> None:
        is_type_matcher = len(a.options.types) != 0
        is_enum_matcher = len(a.options.enumerants) != 0
        if is_type_matcher and is_enum_matcher:
            raise ResolveError(
                f"{a.source} matchers cannot mix enums and types")

        if is_type_matcher:
            options: dict[sem.Node, tok.Source] = {}
            matcher = sem.TypeMatcher(decl=a, name=a.name)
            self.sem.type_matchers.append(matcher)
            self.globals.declare(matcher, a.source)

            for opt in matcher.decl.options.types:
                ty = self._lookup_type(self.globals, opt)
                matcher.types.append(ty)
                if ty in options:
                    raise ResolveError(
                        f"{opt.source} duplicate option '{opt.name}' in matcher\nFirst declared here: {options[ty]}"
                    )
                options[ty] = opt.source
            return

        if is_enum_matcher:
            owners = {enum_opt.owner for enum_opt in a.options.enumerants}
            if len(owners) > 1:
                sorted_owners = sorted(owners)
                raise ResolveError(
                    f"{a.source} cannot mix enums ({sorted_owners}) in type matcher"
                )
            enum_name = a.options.enumerants[0].owner
            lookup = self.globals.lookup(enum_name)
            if lookup is None or not isinstance(lookup.object, sem.Enum):
                raise ResolveError(
                    f"{a.source} cannot resolve enum '{enum_name}'")
            enum_obj = lookup.object

            matcher = sem.EnumMatcher(decl=a, name=a.name, enum=enum_obj)
            self.sem.enum_matchers.append(matcher)
            self.globals.declare(matcher, a.source)

            for opt in matcher.decl.options.enumerants:
                entry = enum_obj.find_entry(opt.member)
                if entry is None:
                    raise ResolveError(
                        f"{opt.source} enum '{enum_obj.name}' does not contain '{opt.member}'"
                    )
                matcher.options.append(entry)
            return

        raise ResolveError(f"{a.source} matcher cannot be empty")

    def _intrinsic(
        self,
        a: ast.IntrinsicDecl,
        intrinsics_by_name: dict[str, sem.Intrinsic],
        sem_intrinsics: list[sem.Intrinsic],
    ) -> None:
        attrs = ast.Attributes(a.attributes)
        intrinsic = intrinsics_by_name.get(a.name)
        if intrinsic is None:
            intrinsic = sem.Intrinsic(name=a.name)
            intrinsics_by_name[a.name] = intrinsic
            sem_intrinsics.append(intrinsic)

        scope = _Scope(self.globals)
        overload = sem.Overload(decl=a, intrinsic=intrinsic)

        overload.implicit_templates = sem.IntrinsicTemplates(
            self._template_params(scope, a.implicit_template_params))
        overload.explicit_templates = sem.IntrinsicTemplates(
            self._template_params(scope, a.explicit_template_params))

        stage_deco = attrs.take("stage")
        if stage_deco is not None:
            for stage in stage_deco.values:
                if stage == "vertex":
                    overload.can_be_used_in_stage.vertex = True
                elif stage == "fragment":
                    overload.can_be_used_in_stage.fragment = True
                elif stage == "compute":
                    overload.can_be_used_in_stage.compute = True
                else:
                    raise ResolveError(
                        f"{stage_deco.source} unknown stage '{stage}'")
        else:
            overload.can_be_used_in_stage = sem.StageUses(
                vertex=True,
                fragment=True,
                compute=True,
            )

        must_use = attrs.take("must_use")
        if must_use is not None:
            if must_use.values:
                raise ResolveError(
                    f"{must_use.source} @must_use does not accept any arguments"
                )
            if a.return_type is None:
                raise ResolveError(
                    f"{must_use.source} @must_use can only be used on a function with a return type"
                )
            overload.must_use = True

        const_eval_fn = attrs.take("const")
        if const_eval_fn is not None:
            if len(const_eval_fn.values) == 0:
                if overload.decl.kind in (
                        ast.IntrinsicKind.Builtin,
                        ast.IntrinsicKind.Operator,
                ):
                    overload.const_eval_function = overload.decl.name
                elif overload.decl.kind == ast.IntrinsicKind.Constructor:
                    overload.const_eval_function = "Ctor"
                elif overload.decl.kind == ast.IntrinsicKind.Converter:
                    overload.const_eval_function = "Conv"
            elif len(const_eval_fn.values) == 1:
                fn_val = const_eval_fn.values[0]
                if not isinstance(fn_val, str):
                    raise ResolveError(
                        f"{const_eval_fn.source} optional @const value must be a string"
                    )
                overload.const_eval_function = fn_val
            else:
                raise ResolveError(
                    f"{const_eval_fn.source} too many values for @const attribute"
                )

        member_function = attrs.take("member_function")
        if member_function is not None:
            overload.member_function = True
            if member_function.values:
                raise ResolveError(
                    f"{member_function.source} unexpected value for member_function attribute"
                )
            if len(a.parameters) < 1:
                raise ResolveError(
                    f"{member_function.source} @member_function can only be used on a function with at least one parameter"
                )

        deprecated = attrs.take("deprecated")
        if deprecated is not None:
            overload.is_deprecated = True
            if deprecated.values:
                raise ResolveError(
                    f"{deprecated.source} unexpected value for deprecated attribute"
                )

        if attrs:
            raise ResolveError(f"{attrs[0].source} unknown attribute")

        intrinsic.overloads.append(overload)

        num_templates = len(overload.all_templates())
        if self.sem.max_templates < num_templates:
            self.sem.max_templates = num_templates

        for p in a.parameters:
            p_attrs = ast.Attributes(p.attributes)
            usage = self._fully_qualified_name(scope, p.type)
            is_const = False
            if p_attrs.take("const") is not None:
                is_const = True
                if not p.name:
                    raise ResolveError(
                        f"{p.source} @const parameters must be named")
            test_value = 1.0
            test_val_attr = p_attrs.take("test_value")
            if test_val_attr is not None:
                val = test_val_attr.values[0]
                if isinstance(val, (int, float)) and not isinstance(val, bool):
                    test_value = float(val)
                else:
                    raise ResolveError(
                        f"{p_attrs[0].source if p_attrs else test_val_attr.source} @test_value must be an integer or float"
                    )
            if p_attrs:
                raise ResolveError(f"{p_attrs[0].source} unknown attribute")
            overload.parameters.append(
                sem.Parameter(
                    name=p.name,
                    type=usage,
                    is_const=is_const,
                    test_value=test_value,
                ))

        if a.return_type is not None:
            usage = self._fully_qualified_name(scope, a.return_type)
            if isinstance(usage.target, (sem.Type, sem.TemplateTypeParam)):
                overload.return_type = usage
            else:
                raise ResolveError(
                    f"{a.return_type.source} cannot use '{a.return_type.name}' as return type. Must be a type or template type"
                )

    def _fully_qualified_name(
        self,
        scope: _Scope,
        arg: ast.TemplatedName,
    ) -> sem.FullyQualifiedName:
        target = self._lookup_named(scope, arg)
        template_args = [
            self._fully_qualified_name(scope, a) for a in arg.template_args
        ]
        return sem.FullyQualifiedName(
            target=target,
            template_arguments=template_args,
        )

    def _template_params(
        self,
        scope: _Scope,
        params: list[ast.TemplateParam],
    ) -> list[sem.TemplateParam]:
        out: list[sem.TemplateParam] = []
        for a in params:
            param = self._template_param(scope, a)
            scope.declare(param, a.source)
            out.append(param)
        return out

    def _template_param(
        self,
        scope: _Scope,
        a: ast.TemplateParam,
    ) -> sem.TemplateParam:
        if a.type.name == "num":
            return sem.TemplateNumberParam(name=a.name, ast_param=a)

        if a.type.name != "":
            resolved = self._fully_qualified_name(scope, a.type)
            target = resolved.target
            if isinstance(target, sem.Enum):
                return sem.TemplateEnumParam(
                    name=a.name,
                    ast_param=a,
                    enum=target,
                )
            if isinstance(target, sem.EnumMatcher):
                return sem.TemplateEnumParam(
                    name=a.name,
                    ast_param=a,
                    enum=target.enum,
                    matcher=target,
                )
            if isinstance(target, (sem.TypeMatcher, sem.Type)):
                return sem.TemplateTypeParam(
                    name=a.name,
                    ast_param=a,
                    type=resolved,
                )
            raise ResolveError(
                f"{a.source} invalid template parameter type '{a.type.name}'")

        return sem.TemplateTypeParam(name=a.name)

    def _lookup_type(
        self,
        scope: _Scope,
        a: ast.TemplatedName,
    ) -> sem.Type:
        resolved = self._lookup_named(scope, a)
        if isinstance(resolved, sem.Type):
            return resolved
        raise ResolveError(
            f"{a.source} '{a.name}' resolves to {_describe(resolved)} but type is expected"
        )

    def _lookup_named(
        self,
        scope: _Scope,
        a: ast.TemplatedName,
    ) -> sem.Node:
        target = scope.lookup(a.name)
        if target is None:
            raise ResolveError(f"{a.source} cannot resolve '{a.name}'")

        obj = target.object
        params: list[sem.TemplateParam] = []
        if isinstance(obj, sem.Type):
            params = obj.template_params
        elif isinstance(obj, sem.TypeMatcher):
            params = []
        elif isinstance(
                obj,
            (
                sem.TemplateTypeParam,
                sem.TemplateEnumParam,
                sem.TemplateEnumEntryParam,
                sem.TemplateNumberParam,
            ),
        ):
            if a.template_args:
                raise ResolveError(
                    f"{a.source} '{a.name}' template parameters do not accept template arguments"
                )
            return obj
        else:
            return obj

        args = a.template_args
        if len(params) != len(args):
            raise ResolveError(
                f"{a.source} '{a.name}' requires {len(params)} template arguments, but {len(args)} were provided"
            )

        for param, arg_ast in zip(params, args):
            arg_obj = self._lookup_named(scope, arg_ast)
            try:
                _check_compatible(arg_obj, param)
            except ResolveError as e:
                raise ResolveError(f"{arg_ast.source} {e}") from e

        return obj

    def _calculate_unique_parameter_names(
        self,
        usages: ast.EnumDecl,
    ) -> list[str]:
        seen: set[str] = {""}
        names: list[str] = []

        for e in usages.entries:
            seen.add(e.name)
            names.append(e.name)

        for intrinsics in (
                self.sem.builtins,
                self.sem.unary_operators,
                self.sem.binary_operators,
                self.sem.constructors_and_converters,
        ):
            for i in intrinsics:
                for o in i.overloads:
                    for p in o.parameters:
                        if p.name not in seen:
                            seen.add(p.name)
                            names.append(p.name)

        names.sort()
        return names


def _describe(n: sem.Node) -> str:
    if isinstance(n, sem.Type):
        return f"type '{n.name}'"
    if isinstance(n, sem.TypeMatcher):
        return f"type matcher '{n.name}'"
    if isinstance(n, sem.Enum):
        return f"enum '{n.name}'"
    if isinstance(n, sem.EnumMatcher):
        return f"enum matcher '{n.name}'"
    if isinstance(n, sem.TemplateTypeParam):
        return "template type"
    if isinstance(n, sem.TemplateEnumParam):
        enum_name = n.enum.name if n.enum is not None else ""
        return f"template enum '{enum_name}'"
    if isinstance(n, sem.EnumEntry):
        enum_name = n.enum.name if n.enum is not None else ""
        return f"enum entry '{enum_name}.{n.name}'"
    if isinstance(n, sem.TemplateNumberParam):
        return "template number"
    raise TypeError(f"unhandled type {type(n)}")


_ANY_TYPE = sem.Type()


def _check_compatible(arg: sem.Node, param: sem.Node) -> None:

    def as_enum(n: sem.Node) -> Optional[sem.Enum]:
        if isinstance(n,
                      (sem.EnumMatcher, sem.TemplateEnumParam, sem.EnumEntry)):
            return n.enum
        return None

    arg_enum = as_enum(arg)
    if arg_enum is not None:
        if arg_enum == as_enum(param):
            return

    if isinstance(arg, sem.TemplateNumberParam) and isinstance(
            param, sem.TemplateNumberParam):
        return

    def as_resolvable_type(n: sem.Node) -> Optional[sem.ResolvableType]:
        if isinstance(n, sem.TemplateTypeParam):
            if n.type is not None:
                return n.type.target  # type: ignore[return-value]
            return _ANY_TYPE
        if isinstance(n, (sem.Type, sem.TypeMatcher)):
            return n
        return None

    arg_ty = as_resolvable_type(arg)
    if arg_ty is not None:
        param_ty = as_resolvable_type(param)
        if arg_ty == param_ty or param_ty == _ANY_TYPE:
            return

    raise ResolveError(f"cannot use {_describe(arg)} as {_describe(param)}")
