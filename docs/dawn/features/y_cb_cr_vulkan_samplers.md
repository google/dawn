# YCbCr Vulkan Samplers

The `y-cb-cr-vulkan-samplers` feature allows specification of VkSamplerYcbcrConversionCreateInfo as
part of the static vulkan sampler descriptor. For this purpose, clients
can supply `YCbCrVkDescriptor` instances when creating samplers and
texture views. Clients can also obtain the YCbCr info for a
SharedTextureMemory instance that was created from an AHardwareBuffer by
querying its properties. When obtaining this info, note that some of the
info *must* be populated while other info *may* be populated, corresponding
to which fields of the underlying `VkSamplerYcbcrConversionCreateInfo` are
mandatory vs. optional.

TODO(crbug.com/dawn/2476): Expand this documentation with examples and
description of semantics (including constraints/validations) as we build out
support
