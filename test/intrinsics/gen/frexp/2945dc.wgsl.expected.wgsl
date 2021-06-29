fn frexp_2945dc() {
  var arg_1 : vec2<u32>;
  var res : vec2<f32> = frexp(vec2<f32>(), &(arg_1));
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  frexp_2945dc();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  frexp_2945dc();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  frexp_2945dc();
}
