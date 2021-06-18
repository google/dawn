var<private> arg_1 : u32;

fn frexp_b7bcbb() {
  var res : f32 = frexp(1.0, &(arg_1));
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  frexp_b7bcbb();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  frexp_b7bcbb();
}

[[stage(compute)]]
fn compute_main() {
  frexp_b7bcbb();
}
