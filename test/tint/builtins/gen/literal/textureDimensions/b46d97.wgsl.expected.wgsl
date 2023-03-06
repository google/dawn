@group(1) @binding(0) var arg_0 : texture_1d<i32>;

fn textureDimensions_b46d97() {
  var res : u32 = textureDimensions(arg_0, 1i);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_b46d97();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_b46d97();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_b46d97();
}
