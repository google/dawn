# Chromium Experimental Primitive ID

The `chromium_experimental_primitive_id` is an experimental extension that allows the use of the
`primitive_id` builtin in fragment shaders in WGSL.

# Status

This extension is experimental and the syntax is being discussed. No official WebGPU specification
has been written yet.

# Availability

The usage is restricted to `fragment` shaders.

| Platform | Type | Notes |
|----------|------|-------|
| SPIR-V | 32-bit integer | Requires geometry shaders, mesh shaders, or raytracing. Available as the `PrimitiveID` builtin |
| HLSL | u32 | Requires D3D10. Available as the `SV_PrimitiveID` semantic |
| GLSL | i32 | Requires GLSL 1.50 and later, ESSL 3.2. (ESSL 3.1 with GL_EXT_geometry_shader). Available as the `gl_primitiveID` builtin |
| Metal | u32 | Requires Metal 2.2 on MacOS or Metal 2.3 on iOS. Available as `[[primitive_id]]` |

Due to the limited availability, this will need a `enable` statement to be used. For this
experimental extension it would be `enable chromium_experimental_primitive_id`.

All of the topologies in `GPUPrimitiveTopology` are supported. (Generally, adjacency topologies would
not be supported but WebGPU does not have any adjacency topologies).

| Topology | Primitive |
|----------|-----------|
| point-list | Each vertex is a primitive |
| line-list | Each vertex pair is a primitive |
| line-strip | Each adjacent vertex pair is a primitive |
| triangle-list | Each vertex triplet is a primitive |
| triangle-strip | Each group of 3 adjacent vertices is a primitive |

# Specification

This extension adds a new `builtin_value_name` entry for `primitive_id`.

An entry is added to the _Built-in input and output values_ table:

 * _Name_: `primitive_id`
 * _Stage_: `fragment`
 * _Direction_: `input`
 * _Type_: `u32`
 * _Extension_: `chromium_experimental_primitive_id`

* The index of the first primitive is zero, incrementing by one for each subsequent primitive.
* `primitive_id` resets to zero between each instance drawn.
* If the primitive id overflows (exceeds 2^32 – 1), it wraps to 0. (This is from HLSL, is it true for
  other backends?)
* There is no support for automatically generating a primitive id for adjacent primitives.
  * For an adjacent primitive, the id is only maintained for the internal non-adjacent primitives.
* The `primitive_id` value is uniform across the primitive.
* Primitive restart has no effect on the value of variables decorated with primitive_id.
  * Is this true for HLSL and MSL? I can see it in the spec for GLSL and SPIR-V but haven't found
    reference for HLSL or MSL.

# Example usage

```wgsl
@vertex fn vs_main(@builtin(primitive_id) my_id: u32) -> @builtin(position) vec4f {
    return vec4f(f32(my_id));
}
```

# References
* [GLSL gl_PrimitiveID](https://registry.khronos.org/OpenGL-Refpages/gl4/html/gl_PrimitiveID.xhtml)
  * [GL_EXT_geometry_shader](https://registry.khronos.org/OpenGL/extensions/EXT/EXT_geometry_shader.txt)
* [ESSL gl_PrimitiveID](https://registry.khronos.org/OpenGL-Refpages/es3/html/gl_PrimitiveID.xhtml)
* [HLSL PrimitiveId](https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-input-assembler-stage-using#primitiveid)
* [HLSL SV_PrimitiveId](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-semantics)
  * [HLSL FunctionalSpec](https://microsoft.github.io/DirectX-Specs/d3d/archive/D3D11_3_FunctionalSpec.htm#:~:text=declaration%20for%20Shaders.-,8.17%20PrimitiveID,-PrimitiveID%20is%20a)
* [Metal p.119](https://developer.apple.com/metal/Metal-Shading-Language-Specification.pdf)
* [Vulkan PrimitiveId](https://registry.khronos.org/vulkan/specs/latest/man/html/PrimitiveId.html)
