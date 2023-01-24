# Intermediate Representation

As Tint has grown the number of transforms on the AST has grown. This
growth has lead to several issues:

1. Transforms rebuild the AST and SEM which causes slowness
1. Transforming in AST can be difficult as the AST is hard to work with

In order to address these goals, an IR is being introduced into Tint.
The IR is mutable, it holds the needed state in order to be transformed.
The IR is also translatable back into AST. It will be possible to
generate an AST, convert to IR, transform, and then rebuild a new AST.
This round-trip ability provides a few features:

1. Easy to integrate into current system by replacing AST transforms
   piecemeal
1. Easier to test as the resulting AST can be emitted as WGSL and
   compared.

The IR helps with the complexity of the AST transforms by limiting the
representations seen in the IR form. For example, instead of `for`,
`while` and `loop` constructs there is a single `loop` construct.
`alias` and `const_assert` nodes are not emitted into IR. Dead code is
eliminated during the IR construction.

As the IR can convert into AST, we could potentially simplify the
SPIRV-Reader by generating IR directly. The IR is closer to what SPIR-V
looks like, so maybe a simpler transform.

## Design

The IR breaks down into two fundamental pieces, the control flow and the
expression lists. While these can be thought of as separate pieces they
are linked in that the control flow blocks contain the expression lists.
A control flow block may use the result of an expression as the
condition.

The IR works together with the AST/SEM. There is an underlying
assumption that the source `Program` will live as long as the IR. The IR
holds pointers to data from the `Program`. This includes things like SEM
types, variables, statements, etc.

Transforming from AST to IR and back to AST is a lossy operation.
The resulting AST when converting back will not be the same as the
AST being provided. (e.g. all `for`, `while` and `loop` constructs coming
in will become `while` loops going out). This is intentional as it
greatly simplifies the number of things to consider in the IR. For
instance:

* No `alias` nodes
* No `const_assert` nodes
* All loops become `while` loops
* `if` statements may all become `if/else`

### Code Structure
The code is contained in the `src/tint/ir` folder and is broken down
into several classes. Note, the IR is a Tint _internal_ representation
and these files should _never_ appear in the public API.

#### Builder
The `Builder` class provides useful helper routines for creating IR
content. The Builder owns an `ir::Module`, it can be created with an
existing Module by moving it into the builder. The Module is moved from
the builder when it is complete.

#### Module
The top level of the IR is the `Module`. The module stores a list of
`functions`, `entry_points`, allocators and various other bits of
information needed by the IR. The `Module` also contains a pointer to
the `Program` which the IR was created from. The `Program` must outlive
the `Module`.

The `Module` provides two methods from moving two and from a `Program`.
The `Module::FromProgram` static method will take a `Program` and
construct an `ir::Module` from the contents. The resulting module class
then has a `ToProgram` method which will construct a new `Program` from
the `Module` contents.

#### BuilderImpl
The `BuilderImpl` is internally used by the `Module` to do the
conversion from a `Program` to a `Module`. This class should not be used
outside the `src/tint/ir` folder.

### Transforms
Similar to the AST a transform system is available for IR. The transform
has the same setup as the AST (and inherits from the same base transform
class.)

Note, not written yet.

### Scoping
The IR flattens scopes. This also means that the IR will rename shadow
variables to be uniquely named in the larger scoped block.

For an example of flattening:

```
{
  var x = 1;
  {
    var y = 2;
  }
}
```

becomes:

```
{
  var x = 1;
  var y = 2;
}
```

For an example of shadowing:

```
{
  var x = 1;
  if (true) {
    var x = 2;
  }
}
```

becomes:

```
{
  var x = 1;
  if true {
    var x_1 = 2;
  }
}
```

### Control Flow Blocks

At the top level, the AST is broken into a series of control flow nodes.
There are a limited set of flow nodes as compared to AST:

1. Block
1. Function
1. If statement
1. Loop statement
1. Switch statement
1. Terminator

As the IR is built a stack of control flow blocks is maintained. The
stack contains `function`, `loop`, `if` and `switch` control flow
blocks. A `function` is always the bottom element in the flow control
stack.

The current instruction block is tracked. The tracking is reset to
`nullptr` when a branch happens. This is used in the statement processing
in order to eliminate dead code. If the current block does not exist, or
has a branch target, then no further instructions can be added, which
means all control flow has branched and any subsequent statements can be
disregarded.

Note, this does have the effect that the inspector _must_ be run to
retrieve the module interface before converting to IR. This is because
phony assignments in dead code add variables into the interface.

```
var<storage> b;

fn a() {
  return;
  _ = b;   // This pulls b into the module interface but would be
           // dropped due to dead code removal.
}
```

#### Control Flow Block
A block is the simplest control flow node. It contains the instruction
lists for a given linear section of codes. A block only has one branch
statement which always happens at the end of the block. Note, the branch
statement is implicit, it doesn't show up in the expression list but is
encoded in the `branch_target`.

In almost every case a block does not branch to another block. It will
always branch to another control flow node. The exception to this rule
is blocks branching to the function end block.

#### Control Flow Function
A function control flow block has two targets associated with it, the
`start_target` and the `end_target`. Function flow starts at the
`start_target` and ends just before the `end_target`. The `end_target`
is always a terminator, it just marks the end of the function
(a return is a branch to the function `end_target`).

#### Control Flow If
The if flow node is an `if-else` structure. There are no `else-if`
entries, they get moved into the `else` of the `if`. The if control flow
node has three targets, the `true_target`, `false_target` and possibly a
`merge_target`.

The `merge_target` is possibly `nullptr`. This can happen if both
branches of the `if` call `return` for instance as the internal branches
would jump to the function `end_target`.

In all cases, the if node will have a `true_target` and a
`false_target`, the target block maybe just a branch to the
`merge_target` in the case where that branch of the if was empty.

#### Control Flow Loop
All of the loop structures in AST merge down to a single loop control
flow node. The loop contains the `start_target`, `continuing_target` and
a `merge_target`.

In the case of a loop, the `merge_target` always exists, but may
actually not exist in the control flow. The target is created in order
to have a branch for `continue` to branch too, but if the loop body does
a `return` then control flow may jump over that block completely.

The chain of blocks from the `start_target`, as long as it does not
`break` or `return` will branch to the `continuing_target`. The
`continuing_target` will possibly branch to the `merge_target` and will
branch to the `start_target` for the loop.

A while loop is decomposed as listed in the WGSL spec:

```
while (a < b) {
  c += 1;
}
```

becomes:

```
loop {
  if (!(a < b)) {
    break;
  }
  c += 1;
}
```

A for loop is decomposed as listed in the WGSL spec:
```
for (var i = 0; i < 10; i++) {
  c += 1;
}
```

becomes:

```
var i = 0;
loop {
  if (!(i < 10)) {
    break;
  }

  c += 1;

  continuing {
    i++;
  }
}
```

#### Control Flow Switch
The switch control flow has a target block for each of the
`case/default` labels along with a `merge_target`. The `merge_target`
while existing, maybe outside the control flow if all of the `case`
branches `return`. The target exists in order to provide a `break`
target.

#### Control Flow Terminator
The terminator control flow is only used as the `end_target` of a
function. It does not contain instructions and is only used as a marker
for the exit of a function.

### Expression Lists.
Note, this section isn't fully formed as this has not been written at
this point.

The expression lists are all in SSA form. The SSA variables will keep
pointers back to the source AST variables in order for us to not require
PHI nodes and to make it easier to move back out of SSA form.

#### Expressions
All expressions in IR are single operations. There are no complex
expressions. Any complex expression in the AST is broke apart into the
simpler single operation components.

```
var a = b + c - (4 * k);
```

becomes:

```
%t0 = b + c
%t1 = 4 * k
%v0 = %t0 - %t1
```

This also means that many of the short forms `i += 1`, `i++` get
expanded into the longer form of `i = i + 1`.

##### Short-Circuit Expressions
The short-circuit expressions (e.g. `a && b`) will be convert into an
`if` structure control flow.

```
let c = a() && b()
```

becomes

```
let c = a();
if (c) {
  c = b();
}
```

#### Registers
There are several types of registers used in the SSA form.

1. Constant Register
1. Temporary Register
1. Variable Register
1. Return Register
1. Function Argument Register

##### Constant Register
The constant register `%c` holds a constant value. All values in IR are
concrete, there are no abstract values as materialization has already
happened. Each constant register holds a single constant value (e.g.
`3.14`) and a pointee to the type (maybe? If needed.)

##### Temporary Register
The temporary register `%t` hold the results of a simple operation. The
temporaries are created as complex expressions are broken down into
pieces. The temporary register tracks the usage count for the register.
This allows a portion of a calculation to be pulled out when rebuilding
AST as a common calculation. If the temporary is used once it can be
re-combine back into a large expression.

##### Variable Register
The variable register `%v` potentially holds a pointer back to source
variables. So, while each value is written only once, if the pointer
back to an AST variable exists we can rebuild the variable that value
was originally created from and can assign back when converting to AST.

##### Return Register
Each function has a return register `%r` where the return value will be
stored before the final block branches to the `end_target`.

##### Function Argument Register
The function argument registers `%a` are used to store the values being
passed into a function call.

#### Type Information
The IR shares type information with the SEM. The types are the same, but
they may exist in different block allocations. The SEM types will be
re-used if they exist, but if the IR needs to create a new type it will
be created in the IRs type block allocator.

#### Loads / Stores and Deref
Note, have not thought about this. We should probably have explicit
load/store operations injected in the right spot, but don't know yet.

## Alternatives
Instead of going to a custom IR there are several possible other roads
that could be travelled.

### Mutable AST
Tint originally contained a mutable AST. This was converted to immutable
in order to allow processing over multiple threads and for safety
properties. Those desires still hold, the AST is public API, and we want
it to be as safe as possible, so keeping it immutable provides that
guarantee.

### Multiple Transforms With One Program Builder
Instead of generating an immutable AST after each transform, running
multiple transforms on the single program builder would remove some of
the performance penalties of going to and from immutable AST. While this
is true, the transforms use a combination of AST and SEM information.
When they transform they _do not_ create new SEM information. That
means, after a given transform, the SEM is out of date. In order to
re-generate the SEM the resolver needs to be rerun. Supporting this
would require being very careful on what transforms run together and
how they modify the AST.

### Adopt An Existing IR
There are already several IRs in the while, Mesa has NIR, LLVM has
LLVM IR. There are others, adopting one of those would remove the
requirements of writing and maintaining our own IR. While that is true,
there are several downsides to this re-use. The IRs are internal to the
library, so the API isn't public, LLVM IR changes with each iteration of
LLVM. This would require us to adapt the AST -> IR -> AST transform for
each modification of the IR.

They also end up being lower level then is strictly useful for us. While
the IR in Tint is a simplified form, we still have to be able to go back
to the high level structured form in order to emit the resulting HLSL,
MSL, GLSL, etc. (Only SPIR-V is a good match for the lowered IR form).
This transformation back is not a direction other IRs maybe interested
in so may have lost information, or require re-determining (determining
variables from SSA and PHI nodes for example).

Other technical reasons are the maintenance of BUILD.gn and CMake files
in order to integrate into our build systems, along with resulting
binary size questions from pulling in external systems.

