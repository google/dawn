@group(0) @binding(0) var t_rgba8unorm : texture_storage_2d<rgba8unorm, write>;
@group(0) @binding(1) var t_rgba8snorm : texture_storage_2d<rgba8snorm, write>;
@group(0) @binding(2) var t_rgba8uint : texture_storage_2d<rgba8uint, write>;
@group(0) @binding(3) var t_rgba8sint : texture_storage_2d<rgba8sint, write>;
@group(0) @binding(4) var t_rgba16uint : texture_storage_2d<rgba16uint, write>;
@group(0) @binding(5) var t_rgba16sint : texture_storage_2d<rgba16sint, write>;
@group(0) @binding(6) var t_rgba16float : texture_storage_2d<rgba16float, write>;
@group(0) @binding(7) var t_r32uint : texture_storage_2d<r32uint, write>;
@group(0) @binding(8) var t_r32sint : texture_storage_2d<r32sint, write>;
@group(0) @binding(9) var t_r32float : texture_storage_2d<r32float, write>;
@group(0) @binding(10) var t_rg32uint : texture_storage_2d<rg32uint, write>;
@group(0) @binding(11) var t_rg32sint : texture_storage_2d<rg32sint, write>;
@group(0) @binding(12) var t_rg32float : texture_storage_2d<rg32float, write>;
@group(0) @binding(13) var t_rgba32uint : texture_storage_2d<rgba32uint, write>;
@group(0) @binding(14) var t_rgba32sint : texture_storage_2d<rgba32sint, write>;
@group(0) @binding(15) var t_rgba32float : texture_storage_2d<rgba32float, write>;

@compute @workgroup_size(1)
fn main() {
  _ = t_rgba8unorm;
  _ = t_rgba8snorm;
  _ = t_rgba8uint;
  _ = t_rgba8sint;
  _ = t_rgba16uint;
  _ = t_rgba16sint;
  _ = t_rgba16float;
  _ = t_r32uint;
  _ = t_r32sint;
  _ = t_r32float;
  _ = t_rg32uint;
  _ = t_rg32sint;
  _ = t_rg32float;
  _ = t_rgba32uint;
  _ = t_rgba32sint;
  _ = t_rgba32float;
}
