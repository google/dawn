# IR Extensions

The IR can be extended by the various backends as needed. This can be done through
custom transforms, capabilities, types, intrinsics or through subclasses of other values.

## Transforms
In a few cases we need to make transforms on the IR which are specific to a given backend.
In these cases the transforms live in the `writer/raise` folder for the backend (e.g.
`lang/msl/writer/raise/`). These transforms are then added into the `raise.cc` file to run the
transform when necessary.

Each transform defines its own configuration if necessary. This is typically done by defining a
`struct` in the transform `.h` file with a naming matching the transform with a  `Config` suffix.
(e.g. `ModuleConstant` transform has a `ModuleConstantConfig` structure). When the transform is
called in `raise` the configuration will be created from the generator configuration.

Transforms have a set of `Capabilities` they support. Often the list of supported capabilities is
added to the `.h` file so they can be shared with a fuzzer for the transform. The set of
capabilities is typically named `k` + transform name + `Capabilities` (e.g. `ModuleConstant` has
`kModuleConstantCapabilities`). The set of capabilities should be kept as minimal as possible for
the transform. The capabilities are used to control what extra features the IR validator will allow.

Transforms all follow a similar pattern of a free function named after the transform and then a
`State` struct with a `Process` method. The free function will run the validator if necessary and
then call the `State::Process` method.

## Intrinsics
Each backend has a `.def` file listing the intrinsics for that backend (e.g. `lang/msl/msl.def`).
These custom intrinsics can be types or instructions. See
[intrinsic_definition_files.md](intrinsic_defintion_files.md).

### Instructions
The intrinsic instructions typically match the signature of the backend, so may take values which
don't make sense coming from core IR or WGSL. (For instance, the `GroupNonUniform` SPIR-V
instructions take a `scope` parameter which doesn't exist, and isn't used, but is there to match the
API on the SPIR-V side.

The naming of the intrinsic is set to match the casing of the backend instruction. So,
`group_non_uniform_s_min` turns into `spirv::BuiltinFn::kGroupNonUniformSMin` which matches to the
`OpGroupNonUniformSMin` instruction in SPIR-V.

As instructions are added they need to be added to the `printer.cc` file for the backend in order to
emit the intrinsic. The instruction also needs to be added to the `builtin_fn.cc.tmpl` file for the
backend to add to the `GetSideEffects` switch. This lists if there are side effects to the
instruction which need to be accounted for when doing instruction inlining.

When these instructions are added to the IR they are done using the `BuiltinCall` class for that
backend. For example, to call the `kGroupNonUniformBroadcast` intrinsic in the SPIR-V backend we
call:

```
b_.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kGroupNonUniformBroadcast, Vector{id}))
```

### Types
There are a few places to modify to create a new type. First, create the type `.h` and `.cc` files
in the backend `type/` folder. The type will inherit from `CastableBase<NewType, Type>` where
`NewType` is the name of the new type. At a minimum the class needs to override:

```
bool Equals(const UniqueNode& other) const override;

/// @returns the friendly name for this type
std::string FriendlyName() const override;

/// @param ctx the clone context
/// @returns a clone of this type
NewType* Clone(core::type::CloneContext& ctx) const override;
```

The class can also override:

```
/// @returns the size in bytes of the type. This may include tail padding.
/// @note opaque types will return a size of 0.
virtual uint32_t Size() const;

/// @returns the alignment in bytes of the type. This may include tail
/// padding.
/// @note opaque types will return a size of 0.
virtual uint32_t Align() const;
```

The `Align` and `Size` overrides are only necessary if the default of size 0 and align 0 do not work
for the new type.

The type can then be added to the `.def` file for the backend as `type new_type`. In order to use
the type in the type matches the `type_matches.h` file for the backend needs to be updated to have a
`MatchNewType` method and a `BuildNewType` method. The arguments end up matching the values passed
to the type implicit template parameters.

The new types can be created in the type manager by calling `ty.Get<spirv::types::NewType>()`,
providing any needed arguments for the type constructor.

### Enums
When defining custom enums in a given backend an extra attribute is needed to set the correct
namespace for the enum, otherwise it ends up in the core IR namespace. In the SPIR-V backend we want
the namespace to be `tint::spirv::type` so we declare enums as:

```
@ns("spirv::type") enum arrayed {
  NonArrayed
  Arrayed
}
```

The `@ns("spirv::type")` will put the enum into the correct namespace when emitted.

