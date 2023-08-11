enable chromium_experimental_read_write_storage_texture;

@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rgba32uint, read_write>;

fn textureNumLayers_2a48dc() {
  var res : u32 = textureNumLayers(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureNumLayers_2a48dc();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureNumLayers_2a48dc();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureNumLayers_2a48dc();
}
