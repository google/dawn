# Chromium Experimental Subgroup Size Control

The `chromium_experimental_subgroup_size_control` is an experimental extension that allows the use of the
`subgroup_size` attribute in compute shaders in WGSL. When the attribute `subgroup_size` is declared, the compute shader will only be executed with the specified subgroup size, and the built-in value `subgroup_size` will be the the value of the attribute `subgroup_size`. Otherwise, the compute shader may be executed with a subgroup size chosen by the GPU driver, which is unknown before the execution of the compute shader.

# Status

This extension is experimental and the syntax is being discussed. No official WebGPU specification
has been written yet.

The status can be tracked at [463721943](https://issues.chromium.org/issues/463721943).

# Availability

The usage is restricted to `compute` shaders.

|Backend|Implementation|Requirements|
|---|---|---|
|D3D12|`[WaveSize(<numLanes>)]` in HLSL|Shader Model 6.6|
|Vulkan|`requiredSubgroupSize` in `VkPipelineShaderStageRequiredSubgroupSizeCreateInfo` on the API side|`subgroupSizeControl` in `VK_EXT_subgroup_size_control` or Vulkan 1.3|
|Metal|Not Supported|Not Supported|

- Currently `VK_EXT_subgroup_control` is required to enable feature `Subgroups` so we can always enable `chromium_experimental_subgroup_size_control` when `Subgroups` can be enabled.
- About [71.5%](https://vulkan.gpuinfo.org/listfeaturesextensions.php?extension=VK_EXT_subgroup_size_control) devices support `subgroupSizeControl` according to the report on GPUInfo.org.
- This extension cannot be supported on Metal.
- Due to the limited availability, this will need a `enable` statement to be used. For this
experimental extension it would be `enable chromium_experimental_subgroup_size_control`.

# Specification

This extension adds a new `subgroup_size` attribute for `compute` shaders.

Enabling this extension on the API side will implicitly enable the `subgroups` extension.

```
subgroup_size_attr :
 '@' 'subgroup_size' '(' expression ',' ? ')'
```
- The parameter must be a const-expression or an override-expression that resolves to an `i32` or `u32`.
- The parameter must be must be a power-of-two.
- The parameter must be greater than or equal to the `minExplicitComputeSubgroupSize` (described later) on the current `adapter`.
- The parameter must be less than or equal to the `maxExplicitComputeSubgroupSize` (described later) on the current `adapter`.
- The total compute invocations per workgroup must be a multiple of the declared `subgroup_size`.

# Example usage

```wgsl
enable subgroups;
enable chromium_experimental_subgroup_size_control;

@subgroup_size(32)
@compute @workgroup_size(64, 1, 1)
fn main(@builtin(subgroup_invocation_id) sg_id : u32,
        @builtin(subgroup_size) sg_size : u32) {

}
```

This extension also adds below limits in Dawn:

1. `maxComputeWorkgroupSubgroups`

Required by Vulkan `VK_EXT_subgroup_size_control` and defined in `VkPhysicalDeviceSubgroupSizeControlProperties.maxComputeWorkgroupSubgroups`. Note that there is no such limitation on D3D12.

| Limit name | Type | Limit class | Default | Compatibility Mode Default |
|------------|------|-------------|---------|----------------------------|
| `maxComputeWorkgroupSubgroups` | `GPUSize32` | `maximum` | - | Not Supported |

- The `workgroup_size` attribute, `subgroup_size` attribute and `maxComputeWorkgroupSubgroups` limit must follow below requirement:
```
workgroup_size.x * workgroup_size.y * workgroup_size.z <= subgroup_size * maxComputeWorkgroupSubgroups
```

2. `minExplicitComputeSubgroupSize` and `maxExplicitComputeSubgroupSize` (in structure `wgpu::AdapterPropertiesExplicitComputeSubgroupSizeConfigs` that can be chained in `wgpu::AdapterInfo`)

Required by both Vulkan and D3D12:
- On D3D12 we should use `D3D12_FEATURE_DATA_D3D12_OPTIONS1::waveLaneCountMin` and `D3D12_FEATURE_DATA_D3D12_OPTIONS1::waveLaneCountMax`, which are not always used as `wgpu::AdapterInfo::subgroupMinSize` or `wgpu::AdapterInfo::subgroupMaxSize` in Dawn.
  For example:
  - On some Intel GPUs, it is possible to run some pixel shaders with wave lane count 8, while on that platform `waveLaneCountMin` is 16, meaning in compute shaders the wave lane count will always be at least 16 (A toggle ["d3d12_relax_min_subgroup_size_to_8"](https://issues.chromium.org/issues/381969450) has been added for this issue).
  - Now Dawn always uses `128` as `SubgroupMaxSize` because in D3D12 document ["the WaveLaneCountMax queried from D3D12 API is not reliable and the meaning is unclear](https://github.com/Microsoft/DirectXShaderCompiler/wiki/Wave-Intrinsics#:~:text=UINT%20WaveLaneCountMax), while `waveLaneCountMax` is actually the maximum value that can be used as HLSL attribute `[WaveSize]`.
- On Vulkan we should use `VkPhysicalDeviceSubgroupSizeControlPropertiesEXT::minSubgroupSize` and `VkPhysicalDeviceSubgroupSizeControlPropertiesEXT::maxSubgroupSize`.

| Limit name | Type | Limit class | Default | Compatibility Mode Default |
|------------|------|-------------|---------|----------------------------|
| `minExplicitComputeSubgroupSize` | `GPUSize32` | `maximum` | - | Not Supported |
| `maxExplicitComputeSubgroupSize` | `GPUSize32` | `maximum` | - | Not Supported |

# References

* [DirectX Specs HLSL Wave Size](https://microsoft.github.io/DirectX-Specs/d3d/HLSL_SM_6_6_WaveSize.html#allowed-wave-sizes)
* [Vulkan VK_EXT_subgroup_size_control](https://docs.vulkan.org/refpages/latest/refpages/source/VK_EXT_subgroup_size_control.html)
