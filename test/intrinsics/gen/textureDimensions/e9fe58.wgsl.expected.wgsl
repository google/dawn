[[group(1), binding(0)]] var arg_0 : texture_storage_2d_array<r32uint, read>;

fn textureDimensions_e9fe58() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_e9fe58();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_e9fe58();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureDimensions_e9fe58();
}
