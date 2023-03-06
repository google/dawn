@group(1) @binding(0) var arg_0 : texture_3d<u32>;

fn textureNumLevels_60d9b8() {
  var res : u32 = textureNumLevels(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureNumLevels_60d9b8();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureNumLevels_60d9b8();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureNumLevels_60d9b8();
}
