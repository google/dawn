# Dawn Errors

Dawn produces errors for several reasons. The most common is validation errors, indicating that a
given descriptor, configuration, state, or action is not valid according to the WebGPU spec. Errors
can also be produced during exceptional circumstances such as the system running out of GPU memory
or the device being lost.

The messages attached to these errors will frequently be one of the primary tools developers use to
debug problems their applications, so it is important that the messages Dawn returns are useful.

Following the guidelines in document will help ensure that Dawn's errors are clear, informative, and
consistent.

## Returning Errors

Since errors are expected to be an exceptional case, it's important that code that produces an error
doesn't adversely impact the performance of the error-free path. The best way to ensure that is to
make sure that all errors are returned from within an `if` statement that uses the `DAWN_UNLIKELY()`
macro to indicate that the expression is not expected to evaluate to true. For example:

```C++
if (DAWN_UNLIKELY(offset > buffer.size)) {
  return DAWN_VALIDATION_ERROR("Offset (%u) is larger than the size (%u) of %s."
    offset, buffer.size, buffer);
}
```

To simplify producing validation errors, it's strongly suggested that the `DAWN_INVALID_IF()` macro
is used, which will wrap the expression in the `DAWN_UNLIKELY()` macro for you:

```C++
// This is equivalent to the previous example.
DAWN_INVALID_IF(offset > buffer.size, "Offset (%u) is larger than the size (%u) of %s."
    offset, buffer.size, buffer);
```

// TODO: Cover `MaybeError`, `ResultOrError<T>`, `DAWN_TRY(_ASSIGN)`, `DAWN_TRY_CONTEXT`, etc...

## Error message formatting

Errors returned from `DAWN_INVALID_IF()` or `DAWN_VALIDATION_ERROR()` should follow these guidelines:

**Write error messages as complete sentences. (First word capitalized, ends with a period, etc.)**
 * Example: `Command encoding has already finished.`
 * Instead of: `encoder finished`

**Error messages should be in the present tense.**
 * Example: `Buffer is not large enough...`
 * Instead of: `Buffer was not large enough...`

**When possible any values mentioned should be immediately followed in parentheses by the given value.**
 * Example: `("Array stride (%u) is not...", stride)`
 * Output: `Array stride (16) is not...`

**When possible any object or descriptors should be represented by the object formatted as a string.**
 * Example: `("The %s size (%s) is...", buffer, buffer.size)`
 * Output: `The [Buffer] size (512) is...` or `The [Buffer "Label"] size (512) is...`

**Enum and bitmask values should be formatted as strings rather than integers or hex values.**
 * Example: `("The %s format (%s) is...", texture, texture.format)`
 * Output: `The [Texture "Label"] format (TextureFormat::RGBA8Unorm) is...`

**When possible state both the given value and the expected value or limit.**
 * Example: `("Offset (%u) is larger than the size (%u) of %s.", offset, buffer.size, buffer)`
 * Output: `Offset (256) is larger than the size (144) of [Buffer "Label"].`

**State errors in terms of what failed, rather than how to satisfy the rule.**
 * Example: `Binding size (3) is less than the minimum binding size (32).`
 * Instead of: `Binding size (3) must not be less than the minimum binding size (32).`

**Don't repeat information given in context.**
 * See next section for details

## Error Context

When calling functions that perform validation consider if calling `DAWN_TRY_CONTEXT()` rather than
`DAWN_TRY()` is appropriate. Context messages, when provided, will be appended to any validation
errors as a type of human readable "callstack". An error with context messages appears will be
formatted as:

```
<Primary error message.>
 - While <context message lvl 2>
 - While <context message lvl 1>
 - While <context message lvl 0>
```

For example, if a validation error occurs while validating the creation of a BindGroup, the message
may be:

```
Binding size (256) is larger than the size (80) of [Buffer "View Matrix"].
 - While validating entries[1] as a Buffer
 - While validating [BindGroupDescriptor "Frame Bind Group"] against [BindGroupLayout]
 - While calling CreateBindGroup
```

// TODO: Guidelines about when to include context

## Context message formatting

Context messages should follow these guidelines:

**Begin with the action being taken, starting with a lower case. `- While ` will be appended by Dawn.**
 * Example: `("validating primitive state")`
 * Output: `- While validating primitive state`

**When looping through arrays, indicate the array name and index.**
 * Example: `("validating buffers[%u]", i)`
 * Output: `- While validating buffers[2]`

**Indicate which descriptors or objects are being examined in as high-level a context as possible.**
 * Example: `("validating % against %", descriptor, descriptor->layout)`
 * Output: `- While validating [BindGroupDescriptor "Label"] against [BindGroupLayout]`

**When possible, indicate the function call being made as the top-level context, as well as the parameters passed.**
 * Example: `("calling %s.CreatePipelineLayout(%s).", this, descriptor)`
 * Output: `- While calling [Device].CreatePipelineLayout([PipelineLayoutDescriptor]).`

# Internal Error Classes
There are multiple levels of validation and error handling which happen in Dawn. There is WebGPU API
validation which directly validates the user input. There is backend validation to make sure the
provided user input can execute on a given backend, there is backend error handling (e.g. when a
request to the Vulkan API fails due to Out of Memory).

Within all of this, Dawn may need to execute backend functionality in service of a user API call.
For example, when uploading a texture, Dawn internally may need to allocate a transfer buffer, copy
the user data into that buffer and then upload. (This is what is referred to as reentrant in Dawn).
Any validation error in this reentrant behaviour *must* be upgraded to a DeviceLost because the system internally is in an invalid state.

## Error Types

Internally Dawn currently supports 6 types of error, they are all listed in `src/dawn/native/Error.h`
inside the `InternalErrorType` enumeration.

1. **None** -- no error generated
2. **Validation** -- An API validation error has occurred
3. **BackendDeviceLost** -- A call in the backend has caused the device to be lost. The device is
                            now gone, there is no cleanup to do and the device must not be
                            used anymore.
4. **Unrecoverable** -- There was an error that will cause a device lost. Internal resources which
                        use the device will be cleaned up before we intentionally lose the device.
5. **PipelineUncategorized** -- An error occurred in the backend (this could be the shader compiler
                                unable to complete due to lack of registers) that is not device
                                fatal, but the users request could not be handled. This does not
                                need to cause a device loss.
6. **OutOfMemory** -- There was an out-of-memory caused by the request. This should cause a device
                      lost as we are in an inconsistent state, modulo a few specific spec permitted
                      places (e.g. Createbuffer/Texture/QueueSet/ResourceTable).

## Classes

### ErrorData, UnrecoverableError, ValidationError, UnknownError

The content of an error is stored in a primary `ErrorData` class. That class is then wrapped by the
error classes, `UnknownError`, `UnrecoverableError`, `ValidationError`.

The `ErrorData` class stores all the actual error information, the type, the message, any other
content. Each of the `Error` classes stores a `unique_ptr` to an `ErrorData`.

In general, code preferably produces a `Validation` or `Unrecoverable` error. In the case where we
have both validation and unrecoverable errors being produced, the `Unknown` error is used.
`UnknownError` stores both **Validation** and other types of error types. In general using
`UnknownError` should be used as a last resort. The code *should* be structured in such a way that
validation is followed by non-validation errors. There are cases, like in the backends where we
have a "validation sandwich" where we must validate, perform backend operation, validate, perform
operation, and these require `UnknownError` usage.

The `UnknownError` provides a way to retrieve the error data as a `Validation` or `Unrecoverable`
error but the `InternalErrorType` must match. It *must* be **Validation** to retrieve a `Validation`
error and it must be something other than **Validation** for an `Unrecoverable` error.

An `UnknownError` can be initialized with either a `Validation` or `Unrecoverable` error.

An `UnrecoverableError` can be initialized with an `Unknown` error, but if that error was
**Validation** it will become **Unrecoverable**.

### MaybeValError, MaybeError, MaybeUnknownError && ResultOrValError, ResultOrError, ResultOrUnknownError

The `Maybe` classes are `Result<void, E>` types, the `ResultOr` classes are `Result<T, E>` types
which wrap the matching error type related to the name. They allow returning errors from methods and
work with the various macros to pass errors up the system.

There are certain conversion restrictions between the result types.

* A `Validation` error and a `UnrecoverableError` can be placed in a `MaybeUnknown` or
  `ResultOrUnknown` error.
* A `Validation` error can be placed into a `MaybeError` or `ResultOrError` but it will convert to
  **Unrecoverable** and no longer be a validation error.
* A `Unrecoverable` error *cannot* be placed into a `Validation` error. This will trigger a
  compile error.

### Error Adapter

The `ErrorAdapter` is used to convert between various error types. The adapter will be created
templated on one of the `Error` classes. This template is then used to determine which of the
conversion methods are available. (The adapter has an `operator <type>` overload for each of the
`Result`, `Maybe` entries and the `unique_ptr` for each `Error` type.

## Macros

Most of the work with the error classes is done through macros in the `src/dawn/native/Error.h` file.
Some of the more interesting macros are:

* `DAWN_VALIDATION_ERROR` -- Creates a **Validation** `Validation` error.
* `DAWN_PIPELINE_UNCATEGORIZED_ERROR` -- Makes a **PipelineUncategorized** `Unrecoverable` error.
* `DAWN_BACKEND_DEVICE_LOST_ERROR` -- Makes a **BackendDeviceLost** `Unrecoverable` error.
* `DAWN_UNRECOVERABLE_ERROR` -- Makes an **Unrecoverable** `Unrecoverable` error.
* `DAWN_OUT_OF_MEMORY_ERROR` -- Makes an **OutOfMemory** `Unrecoverable` error.

* `DAWN_INVALID_IF`  -- Creates a validation error if the given condition is false
* `DAWN_PIPELINE_UNCATEGORIZED_IF` -- Creates the pipeline error if the condition is false
* `DAWN_UNRECOVERABLE_ERROR_IF` -- Creates an unrecoverable error if the condition is false

* `DAWN_TRY` -- Will execute the given command. If the command returns a `Result` set to an error
                it will use the `ErrorAdapter` to return the correct type for the parent method.
* `DAWN_TRY_ASSIGN` -- Similar to the above, but will move the success value out of the `Result`
                       and into the given variable.

## Error Sink

Once an error has propagated, the `ErrorSink` methods are used to consume the error. (The `Device`,
`Instance` and `EncoderContext` are all `ErrorSink` subclasses). Each of the subclass implements a
`ConsumeErrorImpl` method which takes an `UnknownError` and emits it in some fashion.

