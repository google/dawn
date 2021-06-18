var<private> arg_1 : vec2<u32>;

fn frexp_e06acd() {
  var res : vec2<f32> = frexp(vec2<f32>(), &(arg_1));
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  frexp_e06acd();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  frexp_e06acd();
}

[[stage(compute)]]
fn compute_main() {
  frexp_e06acd();
}
