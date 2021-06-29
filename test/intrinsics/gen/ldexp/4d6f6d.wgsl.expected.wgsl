fn ldexp_4d6f6d() {
  var res : vec4<f32> = ldexp(vec4<f32>(), vec4<u32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  ldexp_4d6f6d();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  ldexp_4d6f6d();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  ldexp_4d6f6d();
}
