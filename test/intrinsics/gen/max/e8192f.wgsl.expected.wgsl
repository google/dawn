fn max_e8192f() {
  var res : vec2<i32> = max(vec2<i32>(), vec2<i32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  max_e8192f();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  max_e8192f();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  max_e8192f();
}
