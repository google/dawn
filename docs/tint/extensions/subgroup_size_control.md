# Subgroup Size Control

The `subgroup_size_control` is an extension that allows the use of the
`subgroup_size` attribute in compute shaders in WGSL. When the attribute `subgroup_size` is declared, the compute shader will only be executed with the specified subgroup size, and the built-in value `subgroup_size` will be the the value of the attribute `subgroup_size`. Otherwise, the compute shader may be executed with a subgroup size chosen by the GPU driver, which is unknown before the execution of the compute shader.

# Status

This extension has been accepted as an extension in the [WebGPU specification](https://gpuweb.github.io/gpuweb/#dom-gpufeaturename-subgroup-size-control).

# Availability

The usage is restricted to `compute` shaders.

|Backend|Implementation|Requirements|
|---|---|---|
|D3D12|`[WaveSize(<numLanes>)]` in HLSL|Shader Model 6.6 and Wave Operations support|
|Vulkan|`requiredSubgroupSize` in `VkPipelineShaderStageRequiredSubgroupSizeCreateInfo` on the API side|`subgroupSizeControl` and `computeFullSubgroups` in `VK_EXT_subgroup_size_control` or Vulkan 1.3|
|Metal|Enabled when `mSubgroupMinSize == mSubgroupMaxSize` (only available on Apple GPUs)|`mSubgroupMinSize == mSubgroupMaxSize`|

- On D3D12, `SubgroupSizeControl` is enabled when wave operations are supported and the highest supported shader model is at least 6.6 (which provides the `[WaveSize]` attribute in HLSL).
- On Vulkan, `SubgroupSizeControl` is enabled when `VkPhysicalDeviceSubgroupSizeControlFeatures.subgroupSizeControl` and `VkPhysicalDeviceSubgroupSizeControlFeatures.computeFullSubgroups` are both `VK_TRUE`.
- On Metal, `SubgroupSizeControl` is enabled when `SubgroupMinSize == SubgroupMaxSize`. Although Metal doesn't support selecting a subgroup size, currently all the Apple GPUs have a fixed subgroup size of 32, so controlling the subgroup size is trivially satisfied. Non-Apple Metal GPUs (e.g. Intel/AMD Macs) have different min/max sizes and do not support this feature.

# Specification

This extension adds a new `subgroup_size` attribute for `compute` shaders.

Enabling this extension on the API side will implicitly enable the `subgroups` extension.

```
subgroup_size_attr :
 '@' 'subgroup_size' '(' expression ',' ? ')'
```

Below are the validation rules against the extension `subgroup_size_control` and the attribute `subgroup_size`:
- Enabling `subgroup_size_control` must also enable `subgroups`.
- `subgroup_size` cannot be used when `subgroup_size_control` is not enabled.
- `subgroup_size` can only be used in the compute stage.
- `subgroup_size` must be a const-expression or an override-expression that resolves to an `i32` or `u32`.
- `subgroup_size` must be a power-of-two.
- `subgroup_size` must be greater than or equal to `SubgroupMinSize` on the current `adapter`.
- `subgroup_size` must be less than or equal to `SubgroupMaxSize` on the current `adapter`.
- The total compute invocations per workgroup must be a multiple of the declared `subgroup_size`.

# Example usage

```wgsl
enable subgroups;
enable subgroup_size_control;

@subgroup_size(32)
@compute @workgroup_size(64, 1, 1)
fn main(@builtin(subgroup_invocation_id) sg_id : u32,
        @builtin(subgroup_size) sg_size : u32) {

}
```

# Notes

1. `maxComputeWorkgroupSubgroups` on Vulkan

`maxComputeWorkgroupSubgroups` is a limitation defined in the Vulkan extension `VK_EXT_subgroup_size_control` and defined in `VkPhysicalDeviceSubgroupSizeControlProperties.maxComputeWorkgroupSubgroups`. Note that there is no such limitation on D3D12.

| Limit name | Type | Limit class | Default | Compatibility Mode Default |
|------------|------|-------------|---------|----------------------------|
| `maxComputeWorkgroupSubgroups` | `GPUSize32` | `maximum` | - | Not Supported |

- On Vulkan the `workgroup_size` attribute, `subgroup_size` attribute and `maxComputeWorkgroupSubgroups` limit must follow below requirement:
```
workgroup_size.x * workgroup_size.y * workgroup_size.z <= subgroup_size * maxComputeWorkgroupSubgroups
```

We decide not to expose `maxComputeWorkgroupSubgroups` in Dawn because it is not a limit on D3D12 and it is not a limit on Metal. Instead, we will validate the `subgroup_size` attribute against the `workgroup_size` attribute and the `maxComputeWorkgroupSubgroups` limit on Vulkan.

2. `SubgroupMinSize` and `SubgroupMaxSize` (in `wgpu::AdapterInfo`)

The `subgroup_size` attribute is validated against `SubgroupMinSize` and `SubgroupMaxSize` directly. The separate `minExplicitComputeSubgroupSize` and `maxExplicitComputeSubgroupSize` limits are no longer needed.

- On D3D12, `SubgroupMinSize` and `SubgroupMaxSize` are sourced from `D3D12_FEATURE_DATA_D3D12_OPTIONS1::waveLaneCountMin` and `waveLaneCountMax`. Microsoft has [clarified](https://github.com/microsoft/DirectXShaderCompiler/issues/8535) that `WaveLaneCountMax` is exactly the maximum wave lane count that can be used with the HLSL `[WaveSize]` attribute.
  - On some Intel GPUs, pixel shaders may run with wave lane count 8 while `waveLaneCountMin` reports 16. This is treated as a driver issue.
- On Vulkan, `SubgroupMinSize` and `SubgroupMaxSize` are sourced from `VkPhysicalDeviceSubgroupSizeControlPropertiesEXT::minSubgroupSize` and `maxSubgroupSize`.

# References

* [DirectX Specs HLSL Wave Size](https://microsoft.github.io/DirectX-Specs/d3d/HLSL_SM_6_6_WaveSize.html#allowed-wave-sizes)
* [Vulkan VK_EXT_subgroup_size_control](https://docs.vulkan.org/refpages/latest/refpages/source/VK_EXT_subgroup_size_control.html)
