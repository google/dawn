enable chromium_experimental_read_write_storage_texture;

@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rgba8uint, read>;

fn textureNumLayers_d3e21f() {
  var res : u32 = textureNumLayers(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureNumLayers_d3e21f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureNumLayers_d3e21f();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureNumLayers_d3e21f();
}
