# YCbCr Vulkan Samplers

The `y-cb-cr-vulkan-samplers` feature allows specification of VkSamplerYcbcrConversionCreateInfo as
part of the static vulkan sampler descriptor. For this purpose, clients
can supply `YCbCrVkDescriptor` instances when creating samplers and
texture views. Clients can also obtain the YCbCr info for a
SharedTextureMemory instance that was created from an AHardwareBuffer by
querying its properties. Most properties will be created directly from the
corresponding buffer format properties on the underlying AHardwareBuffer. The
two exceptions are as follows:

* `vkChromaFilter`: Will be set to FilterMode::Linear iff
  `VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT` is
  present in the AHB format features and FilterMode::Nearest otherwise
* `forceExplicitReconstruction`: will be set to true iff
  `VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT`
  is present in the AHB format features
TODO(crbug.com/dawn/2476): Expand this documentation with examples and
description of semantics (including constraints/validations) as we build out
support
