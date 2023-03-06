@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rgba16float, write>;

fn textureNumLayers_98a9cf() {
  var res : u32 = textureNumLayers(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureNumLayers_98a9cf();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureNumLayers_98a9cf();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureNumLayers_98a9cf();
}
