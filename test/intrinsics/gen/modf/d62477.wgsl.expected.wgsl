fn modf_d62477() {
  var arg_1 : vec2<f32>;
  var res : vec2<f32> = modf(vec2<f32>(), &(arg_1));
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  modf_d62477();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  modf_d62477();
}

[[stage(compute)]]
fn compute_main() {
  modf_d62477();
}
