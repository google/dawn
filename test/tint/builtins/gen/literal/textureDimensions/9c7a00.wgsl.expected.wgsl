@group(1) @binding(0) var arg_0 : texture_1d<u32>;

fn textureDimensions_9c7a00() {
  var res : u32 = textureDimensions(arg_0, 1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_9c7a00();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_9c7a00();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_9c7a00();
}
