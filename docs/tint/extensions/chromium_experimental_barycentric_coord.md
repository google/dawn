# Chromium Experimental Barycentric Coordinates

The `chromium_experimental_barycentric_coord` is an experimental extension that allows the use of the
`barycentric_coord` builtin in fragment shaders in WGSL.

# Status

This extension is experimental and the syntax is being discussed. No official WebGPU specification
has been written yet.

# Availability

The usage is restricted to `fragment` shaders.

| SPIR-V | `vec3f` | `VK_KHR_fragment_shader_barycentric`. Available as the `BaryCoordKHR` builtin |
| HLSL | `vec3f` | Requires D3D12, HLSL 6.1. Available as the `SV_Barycentrics` semantic |
| GLSL | `vec3f` | `GLSL_EXT_fragment_shader_barycentric` (Requires GLSL 4.50 and later, ESSL 3.2 and later). Available as the `gl_BaryCoordEXT` builtin |
| Metal | `f32`, `vec2f`, or `vec3f` | Requires Metal 2.2 on MacOS or Metal 2.3 on iOS. Available as `[[barycentric_coord]]`, can test with `supportsShaderBarycentricCoordinates` |

Vulkan support unfortunately [looks pretty low](https://vulkan.gpuinfo.org/displayextensiondetail.php?extension=VK_KHR_fragment_shader_barycentric) with 10.76%	of devices on GPUInfo.org reporting support. The relevant GL extension doesn't even appear on the site.

Due to the limited availability, this will need a `enable` statement to be used. For this
experimental extension it would be `enable chromium_experimental_barycentric_coord`.

Additionally, anywhere that barycentrics are supported they support perspective corrected and non-perspective correct
values are available. Most backends support multiple interpolation modes, but only centroid is available everywhere.

| SPIR-V | `BaryCoordKHR`, `BaryCoordNoPerspKHR`, both can be decorated with the `Centroid` or `Sample` interpolation qualifiers |
| HLSL | `float3 b : SV_Barycentrics` (default, perspective correct), `noperspective float3 b : SV_Barycentrics`. Interpolation modes like `linear`, `centroid`, and `sample` are supported |
| GLSL | `gl_BaryCoordEXT`, `gl_BaryCoordNoPerspEXT`, both can be decorated with the centroid or sample interpolation qualifiers |
| Metal | `[[barycentric_coord, center_perspective]]` (default, can be omitted), ` [[barycentric_coord, center_no_perspective]]` |

# Specification

This extension adds a new `builtin_value_name` entry for `barycentric_coord`.

An entry is added to the _Built-in input and output values_ table:

| Name | `barycentric_coord` |
| Stage | `fragment` |
| Direction | `input` |
| Type | `vec3` |
| Extension | `chromium_experimental_barycentric_coord` |

* The value contains the relative weights for each vertex in the primitive.
* Fragments in the corners of triangle primitives will have weights of (1,0,0), (0,1,0), and (0,0,1)
* Fragments at the endpoints of line primitives will have weights of (1,0,0), and (0,1,0). The z component will always be 0.
* Support for point primitives is unclear (Metal mentions that they will have yz components of (0, 0), HLSL and GLSL make no mention of points.)
* The value of the three components should add up to approximately 1, but strictly adding up to 1 is not guaranteed.

# Example usage

```wgsl
@fragment fn fs_main(@builtin(barycentric_coord) bary_coord: vec3f) -> @builtin(color) vec4f {
    return vec4f(bary_coord, 1.0);
}
```

# References

* [DirectXShaderCompiler SV_Barycentrics](https://github.com/microsoft/DirectXShaderCompiler/wiki/SV_Barycentrics)
  * [Shader Model 6.1](https://github.com/microsoft/DirectXShaderCompiler/wiki/Shader-Model-6.1)
* [Metal p.119](https://developer.apple.com/metal/Metal-Shading-Language-Specification.pdf)
* [MTLDevice supportsShaderBarycentricCoordinates](https://developer.apple.com/documentation/metal/mtldevice/supportsshaderbarycentriccoordinates)
* [Vulkan VK_KHR_fragment_shader_barycentric](https://registry.khronos.org/vulkan/specs/latest/man/html/VK_KHR_fragment_shader_barycentric.html)
  * [SPIR-V VK_KHR_fragment_shader_barycentric](https://github.khronos.org/SPIRV-Registry/extensions/KHR/SPV_KHR_fragment_shader_barycentric.html)
* [GLSL_EXT_fragment_shader_barycentric](https://github.com/KhronosGroup/GLSL/blob/main/extensions/ext/GLSL_EXT_fragment_shader_barycentric.txt)
