@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rgba16snorm, write>;

fn textureNumLayers_1f6da6() -> u32 {
  var res : u32 = textureNumLayers(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureNumLayers_1f6da6();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureNumLayers_1f6da6();
}
