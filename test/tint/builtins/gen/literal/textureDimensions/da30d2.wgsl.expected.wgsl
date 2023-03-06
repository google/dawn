@group(1) @binding(0) var arg_0 : texture_storage_1d<rgba32float, write>;

fn textureDimensions_da30d2() {
  var res : u32 = textureDimensions(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_da30d2();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_da30d2();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_da30d2();
}
