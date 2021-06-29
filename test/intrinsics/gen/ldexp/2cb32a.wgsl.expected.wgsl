fn ldexp_2cb32a() {
  var res : vec3<f32> = ldexp(vec3<f32>(), vec3<u32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  ldexp_2cb32a();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  ldexp_2cb32a();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  ldexp_2cb32a();
}
