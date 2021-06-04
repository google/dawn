fn frexp_2a171c() {
  var arg_1 : vec2<u32>;
  var res : vec2<f32> = frexp(vec2<f32>(), &(arg_1));
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  frexp_2a171c();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  frexp_2a171c();
}

[[stage(compute)]]
fn compute_main() {
  frexp_2a171c();
}
