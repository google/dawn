@group(1) @binding(0) var arg_0 : texture_2d_array<f32>;

fn textureDimensions_46f0fc() {
  var res : vec2<u32> = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_46f0fc();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_46f0fc();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_46f0fc();
}
