fn frexp_a0eb3b() {
  var res = frexp(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  frexp_a0eb3b();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  frexp_a0eb3b();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  frexp_a0eb3b();
}
