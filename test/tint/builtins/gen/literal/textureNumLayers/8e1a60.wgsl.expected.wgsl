@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rg16float, write>;

fn textureNumLayers_8e1a60() -> u32 {
  var res : u32 = textureNumLayers(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureNumLayers_8e1a60();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureNumLayers_8e1a60();
}
