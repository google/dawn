var<private> arg_1 : vec4<u32>;

fn frexp_d14b16() {
  var res : vec4<f32> = frexp(vec4<f32>(), &(arg_1));
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  frexp_d14b16();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  frexp_d14b16();
}

[[stage(compute)]]
fn compute_main() {
  frexp_d14b16();
}
