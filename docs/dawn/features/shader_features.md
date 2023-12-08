# Shader features

New shader features require the extensions to be enabled at device creation in addition to adding a `enable` directive in the WGSL.
This file regroups the documentation for all the Dawn-specific shader-only features.

## ChromiumExperimentalSubgroups

This adds support for the [`chromium_experimental_subgroups`](Link to do) WGSL `enable`.
Currently used to investigate subgroup functionality and not for general use.

It also provides the `wgpu::DawnExperimentalSubgroupLimits` structure used to gather data about the subgroup minimum and maximum size on the device.
(the limit cannot be changed when requesting a device)
`wgpu::DawnExperimentalSubgroupLimits` is populated by chaining it to the `wgpu::SupportedLimits` in the calls to `wgpu::Adapter::GetLimits` and `wgpu::Device::GetLimits`.

## ChromiumExperimentalSubgroupsUniformControlFlow

Used to expose that the device supports `VK_KHR_shader_subgroup_uniform_control_flow`.
This is only for use in the investigation of subgroup functionality.

