# Unorm 16 formats for external textures

Chromium needs to create ExternalTextures on YUV HDR videos for 0-copy interop, using formats such as `wgpu::TextureFormat::R10X6BG10C6Biplanar420Unorm` and friends.
These formats have planes with `R/RG16Unorm` formats that need to work to import `ExternalTextures` from video, even if other texture format extensions haven't been enabled.

This feature loosens validation in two specific places to allow using YUV HDR videos without exposing `R/RG16Unorm` for anything else:

 - Views of planes of YUV HDR textures can be created using the `R/RG16Unorm` formats even if the formats are not otherwise allowed on the device.
 - Validation of `CreateExternalTexture` considers `R/RG16Unorm` to be filterable float, even if they are unfilterable-float (even when enabled with `wgpu::FeatureName::TextureFormatsTier1`).
