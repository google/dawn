# Tint Architecture

```
                   +--------+                   +------+
                   | SPIR-V |                   | WGSL |
                   +----|---+                   +---|--+
                        |                           |
              +---------|---------------------------|--------+
              |         |          Reader           |        |
              |         |                           |        |
              | +-------|------+             +------|------+ |
              | | SPIRV-Reader |             | WGSL-Reader | |
              | +--------------+             +-------------+ |
              +-----------------------|----------------------+
                                      |
                                   +--|--+      +---------+
                                   | AST |------| IsValid |
                                   +--|--+      +---------+
                                      |
                             +--------|--------+
                             | Type Determiner |
                             +--------|--------+
                                      |
                                +-----|-----+
                                | Validator |
                                +-----|-----+
                                      |
                                +-----|-----+
                                | Inspector |
                                +-----|-----+
                                      |
                               +------|-----+
                               | Transforms |
                               +------|-----+
                                      |
+-------------------------------------|------------------------------------+
|                                  Writer                                  |
|                                                                          |
| +--------------+    +-------------+    +-------------+    +------------+ |
| | SPIRV-Writer |    | WGSL-Writer |    | HLSL-Writer |    | MSL-Writer | |
| +-------|------+    +------|------+    +------|------+    +------|-----+ |
+---------|------------------|------------------|------------------|-------+
          |                  |                  |                  |
     +----|---+          +---|--+           +---|--+            +--|--+
     | SPIR-V |          | WGSL |           | HLSL |            | MSL |
     +--------+          +------+           +------+            +-----+
```

## WGSL Reader

The WGSL reader is a recursive descent parser. It closely follows the WGSL
grammar in the naming of the parse methods.


## AST

The Abstract Syntax Tree is a directed acyclic graph of nodes encoding
the structure of the WGSL program.

Many AST nodes have numeric parameters. For example, the ast::StrideDecoration
node has numeric stride parameter, which is a count of the number of bytes from
the start of one array element to the start of the next.  The AST node itself
does not constrain the set of stride values that you can set, aside from storing
it as an unsigned integer.

After creating the AST for the whole WGSL program, Tint will run an `IsValid`
method which verifies that the produced AST is structurally correct.  This
means that things like `if` statements have a condition and body attached.

A later phase of processing, Validation, will check properties such as whether
array strides satisfy all the requirements in the WGSL specification.

## Type Determiner

The type determination stage assigns a result type to each node in the AST. It
will do the minimal amount of validation required in order to always be
accessing valid nodes, but that doesn't mean the overall AST is valid. (e.g. a
4 component vector type constructor with 5 components will pass type
determination but is invalid.)


## Validation

After the validation step the AST should be a known valid WGSL program.

## Inspector

The inspectors job is to go through the AST and pull out various pieces of
information. The information may be used to pass information into the downstream
compilers (things like specialization constants) or may be used to pass into
transforms to update the AST before generating the resulting code.

## Transforms

There maybe various transforms we want to run over the AST. This is for things
like Vertex Pulling or Robust Buffer Access. These transforms will modify the
AST and must produce valid AST after their execution.

Currently the transforms are run individually but we should consider adding
a transform manager to maintain and execute the list of transforms.


## Writers

The writers should be thread safe, we maybe generating the HLSL and MLS output
at the same time on different threads. (The writer portion is the only one
that needs to be thread safe.)
