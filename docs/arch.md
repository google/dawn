# Tint Architecture

```
                   ┏━━━━━━━━┓                   ┏━━━━━━┓
                   ┃ SPIR━V ┃                   ┃ WGSL ┃
                   ┗━━━━┃━━━┛                   ┗━━━┃━━┛
                        ▼                           ▼
              ┏━━━━━━━━━┃━━━━━━━━━━━━━━━━━━━━━━━━━━━┃━━━━━━━━┓
              ┃         ┃          Reader           ┃        ┃
              ┃         ┃                           ┃        ┃
              ┃ ┏━━━━━━━┻━━━━━━┓             ┏━━━━━━┻━━━━━━┓ ┃
              ┃ ┃ SPIRV-Reader ┃             ┃ WGSL-Reader ┃ ┃
              ┃ ┗━━━━━━━━━━━━━━┛             ┗━━━━━━━━━━━━━┛ ┃
              ┗━━━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━━┛
                                      ▼
                    ┏━━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━┓
                    ┃           ProgramBuilder          ┃
                    ┃             (mutable)             ┃
      ┏━━━━━━━━━━━━►┫   ┏━━━━━┓ ┏━━━━━━━┓ ┏━━━━━━━━━┓   ┃
      ┃             ┃   ┃ AST ┃ ┃ Types ┃ ┃ Symbols ┃   ┃
      ┃             ┃   ┗━━━━━┛ ┗━━━━━━━┛ ┗━━━━━━━━━┛   ┃
      ┃             ┗━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━┛
      ┃                               ▼
      ┃             ┌┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┃┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┐
      ▲             ┆ Build           ▼                ┆
  ┏━━━┻━━━┓         ┆        ┏━━━━━━━━┻━━━━━━━━┓       ┆
  ┃ Clone ┃         ┆        ┃    Resolver     ┃       ┆
  ┗━━━┳━━━┛         ┆        ┗━━━━━━━━━━━━━━━━━┛       ┆
      ▲             └┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┃┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┘
      ┃                               ▼
      ┃       ┏━━━━━━━━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━━━━┓
      ┃       ┃                    Program                   ┃
      ┃       ┃                  (immutable)                 ┃
      ┣━━━━━━◄┫  ┏━━━━━┓ ┏━━━━━━━┓ ┏━━━━━━━━━━┓ ┏━━━━━━━━━┓  ┃
      ┃       ┃  ┃ AST ┃ ┃ Types ┃ ┃ Semantic ┃ ┃ Symbols ┃  ┃
      ┃       ┃  ┗━━━━━┛ ┗━━━━━━━┛ ┗━━━━━━━━━━┛ ┗━━━━━━━━━┛  ┃
      ┃       ┗━━━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━━┛
      ▲                               ▼
┏━━━━━┻━━━━━┓                         ┃             ┏━━━━━━━━━━━┓
┃ Transform ┃◄━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━►┃ Inspector ┃
┗━━━━━━━━━━━┛                         ┃             ┗━━━━━━━━━━━┛
                                      ▼
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                  Writer                                  ┃
┃                                                                          ┃
┃ ┏━━━━━━━━━━━━━━┓    ┏━━━━━━━━━━━━━┓    ┏━━━━━━━━━━━━━┓    ┏━━━━━━━━━━━━┓ ┃
┃ ┃ SPIRV-Writer ┃    ┃ WGSL-Writer ┃    ┃ HLSL-Writer ┃    ┃ MSL-Writer ┃ ┃
┃ ┗━━━━━━━┳━━━━━━┛    ┗━━━━━━┳━━━━━━┛    ┗━━━━━━┳━━━━━━┛    ┗━━━━━━┳━━━━━┛ ┃
┗━━━━━━━━━┃━━━━━━━━━━━━━━━━━━┃━━━━━━━━━━━━━━━━━━┃━━━━━━━━━━━━━━━━━━┃━━━━━━━┛
          ▼                  ▼                  ▼                  ▼
     ┏━━━━┻━━━┓          ┏━━━┻━━┓           ┏━━━┻━━┓            ┏━━┻━━┓
     ┃ SPIR-V ┃          ┃ WGSL ┃           ┃ HLSL ┃            ┃ MSL ┃
     ┗━━━━━━━━┛          ┗━━━━━━┛           ┗━━━━━━┛            ┗━━━━━┛
```

## Reader

Readers are responsible for parsing a shader program and populating a
`ProgramBuilder` with the parsed AST, type and symbol information.

The WGSL reader is a recursive descent parser. It closely follows the WGSL
grammar in the naming of the parse methods.

## ProgramBuilder

A `ProgramBuilder` is the primary interface to construct an immutable `Program`.
There are a number of methods exposed which make creating of the `Program`
simpler. A `ProgramBuilder` can only be used once, and must be discarded after
the `Program` is constructed.

A `Program` is built from the `ProgramBuilder` by `std::move()`ing the
`ProgramBuilder` to a new `Program` object. When built, resolution is performed
so the produced `Program` will contain all the needed semantic information.

At any time before building the `Program`, `ProgramBuilder::IsValid()` may be
called to ensure the AST is **structurally** correct. This checks that things
like `if` statements have a condition and body attached.

If further changes to the `Program` are needed (say via a `Transform`) then a
new `ProgramBuilder` can be produced by cloning the `Program` into a new
`ProgramBuilder`.

Unlike `Program`s, `ProgramBuilder`s are not part of the public Tint API.

## AST

The Abstract Syntax Tree is a directed acyclic graph of `ast::Node`s which
encode the syntactic structure of the WGSL program.

The root of the AST is the `ast::Module` class which holds each of the declared
functions, variables and user defined types (type aliases and structures).

Each `ast::Node` represents a **single** part of the program's source, and so
`ast::Node`s are not shared.

The AST does not perform any verification of its content. For example, the
`ast::StrideDecoration` node has numeric stride parameter, which is a count of
the number of bytes from the start of one array element to the start of the
next. The AST node itself does not constrain the set of stride values that you
can set, aside from storing it as an unsigned integer.

## Types

Types are constructed during the Reader and resolution phases, and are
held by the `Program` or `ProgramBuilder`. AST and semantic nodes can both
reference types.

Each `type::Type` node **uniquely** represents a particular spelling of a WGSL
type within the program, so you can compare `type::Type*` pointers to check for
equivalence of type expressions.
For example, there is only one `type::Type` node for the `i32` type, no matter
how many times it is mentioned in the source program.
However, if `MyI32` is a type alias for `i32`, then they will have two different
type nodes.

## Semantic information

Semantic information is held by `sem::Node`s which describe the program at
a higher / more abstract level than the AST. This includes information such as
the resolved type of each expression, the resolved overload of an intrinsic
function call, and the module scoped variables used by each function.

Semantic information is generated by the `Resolver` when the `Program`
is built from a `ProgramBuilder`.

The `sem::Info` class holds a map of `ast::Node`s to `sem::Node`s.
This map is **many-to-one** - i.e. while a AST node might have a single
corresponding semantic node, the reverse may not be true. For example:
many `ast::IdentifierExpression` nodes may map to a single `sem::Variable`,
and so the `sem::Variable` does not have a single corresponding
`ast::Node`.

Unlike `ast::Node`s, semantic nodes may not necessarily form a directed acyclic
graph, and the semantic graph may contain diamonds.

## Symbols

Symbols represent a unique string identifier in the source program. These string
identifiers are transformed into symbols within the `Reader`s.

During the Writer phase, symbols may be emitted as strings using a `Namer`.
A `Namer` may output the symbol in any form that preserves the uniqueness of
that symbol.

## Resolver

The `Resolver` will automatically run when a `Program` is built.
A `Resolver` creates the `Program`s semantic information by analyzing the
`Program`s AST and type information.

The `Resolver` will validate to make sure the generated `Program` is
semantically valid.

## Program

A `Program` holds an immutable version of the information from the
`ProgramBuilder` along with semantic information generated by the
`Resolver`.

Like `ProgramBuilder`, `Program::IsValid()` may be called to ensure the AST is
structurally correct and semantically valid, and that the `Resolver` did not
report any errors.

Unlike the `ProgramBuilder`, a `Program` is fully immutable, and is part of the
public Tint API. The immutable nature of `Program`s make these entirely safe
to share between multiple threads without the use of synchronization primitives.

## Inspector

The inspectors job is to go through the `Program` and pull out various pieces of
information. The information may be used to pass information into the downstream
compilers (things like specialization constants) or may be used to pass into
transforms to update the AST before generating the resulting code.

The input `Program` to the inspector must be valid (pass validation).

## Transforms

There maybe various transforms we want to run over the `Program`.
This is for things like Vertex Pulling or Robust Buffer Access.

A transform operates by cloning the input `Program` into a new `ProgramBuilder`,
applying the required changes, and then finally building and returning a new
output `Program`. As the resolver is always run when a `Program` is built,
Transforms will always emit a `Program` with semantic information.

The input `Program` to a transform must be valid (pass validation).
If the input `Program` of a transform is valid then the transform must guarantee
that the output program is also valid.

## Writers

A writer is responsible for writing the `Program` in the target shader language.

The input `Program` to a writer must be valid (pass validation).
