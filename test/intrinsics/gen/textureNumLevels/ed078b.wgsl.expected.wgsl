[[group(1), binding(0)]] var arg_0 : texture_cube<u32>;

fn textureNumLevels_ed078b() {
  var res : i32 = textureNumLevels(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureNumLevels_ed078b();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLevels_ed078b();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureNumLevels_ed078b();
}
