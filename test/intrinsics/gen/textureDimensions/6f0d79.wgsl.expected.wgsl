[[group(1), binding(0)]] var arg_0 : texture_storage_2d_array<rgba8snorm, write>;

fn textureDimensions_6f0d79() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_6f0d79();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_6f0d79();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureDimensions_6f0d79();
}
