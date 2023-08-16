# Dual Source Blending

The `dual-source-blending` feature adds additional blend factors and the WGSL @index attribute to allow a fragment shader to blend two color outputs into a single output buffer.

This feature adds the following `wgpu::BlendFactors`:
- `Src1`
- `OneMinusSrc1`
- `Src1Alpha`
- `OneMinusSrc1Alpha`

This feature introduces the @index WGSL attribute. This attribute is added to a fragment output at @location(0) to denote the blending source index. You must use `enable chromium_internal_dual_source_blending` in a shader to use the @index attribute.

Example Fragment Shader:
```
    enable chromium_internal_dual_source_blending;

    struct FragOut {
        @location(0) @index(0) color : vec4<f32>,
        @location(0) @index(1) blend : vec4<f32>,
    }

    @fragment fn main() -> FragOut {
        var output : FragOut;
        output.color = {1.0, 1.0, 1.0, 1.0};
        output.blend = {0.5, 0.5, 0.5, 0.5};
        return output;
    }
```

### Restrictions:
    - Dual source blending must occur on color attachment 0.
    - Dual source blending makes it invalid to render to multiple render targets.

