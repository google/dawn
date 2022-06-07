@group(1) @binding(0) var arg_0 : texture_1d<u32>;

fn textureDimensions_79df87() {
  var res : i32 = textureDimensions(arg_0, 0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_79df87();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_79df87();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_79df87();
}
