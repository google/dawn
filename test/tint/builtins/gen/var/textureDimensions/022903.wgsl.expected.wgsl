@group(1) @binding(0) var arg_0 : texture_1d<i32>;

fn textureDimensions_022903() {
  var arg_1 = 1u;
  var res : u32 = textureDimensions(arg_0, arg_1);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_022903();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_022903();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_022903();
}
