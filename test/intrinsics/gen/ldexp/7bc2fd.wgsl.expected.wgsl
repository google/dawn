fn ldexp_7bc2fd() {
  var res : vec2<f32> = ldexp(vec2<f32>(), vec2<u32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  ldexp_7bc2fd();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  ldexp_7bc2fd();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  ldexp_7bc2fd();
}
