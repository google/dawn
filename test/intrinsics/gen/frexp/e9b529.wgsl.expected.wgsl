fn frexp_e9b529() {
  var arg_1 : vec3<u32>;
  var res : vec3<f32> = frexp(vec3<f32>(), &(arg_1));
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  frexp_e9b529();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  frexp_e9b529();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  frexp_e9b529();
}
