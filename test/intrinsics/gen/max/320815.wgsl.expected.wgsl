fn max_320815() {
  var res : vec2<u32> = max(vec2<u32>(), vec2<u32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  max_320815();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  max_320815();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  max_320815();
}
