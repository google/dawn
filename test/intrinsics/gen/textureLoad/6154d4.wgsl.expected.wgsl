[[group(1), binding(0)]] var arg_0 : texture_2d<u32>;

fn textureLoad_6154d4() {
  var res : vec4<u32> = textureLoad(arg_0, vec2<i32>(), 0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureLoad_6154d4();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_6154d4();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureLoad_6154d4();
}
