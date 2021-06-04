[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_2d_array<rgba8uint>;

fn textureDimensions_f7e436() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_f7e436();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_f7e436();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_f7e436();
}
