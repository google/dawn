fn ldexp_abd718() {
  var res : vec2<f32> = ldexp(vec2<f32>(), vec2<i32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  ldexp_abd718();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  ldexp_abd718();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  ldexp_abd718();
}
