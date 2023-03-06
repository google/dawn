@group(1) @binding(0) var arg_0 : texture_storage_1d<rg32uint, write>;

fn textureDimensions_ea25bc() {
  var res : u32 = textureDimensions(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_ea25bc();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_ea25bc();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_ea25bc();
}
