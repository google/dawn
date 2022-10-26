@group(1) @binding(0) var arg_0 : texture_cube<f32>;

fn textureDimensions_9b223b() {
  var res : vec2<u32> = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_9b223b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_9b223b();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_9b223b();
}
