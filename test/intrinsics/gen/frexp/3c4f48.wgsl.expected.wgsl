fn frexp_3c4f48() {
  var res = frexp(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  frexp_3c4f48();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  frexp_3c4f48();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  frexp_3c4f48();
}
