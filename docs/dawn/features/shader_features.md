# Shader features

New shader features require the extensions to be enabled at device creation in addition to adding a `enable` directive in the WGSL.
This file regroups the documentation for all the Dawn-specific shader-only features.

## ChromiumExperimentalSubgroupsUniformControlFlow

Used to expose that the device supports `VK_KHR_shader_subgroup_uniform_control_flow`.
This is only for use in the investigation of subgroup functionality.

