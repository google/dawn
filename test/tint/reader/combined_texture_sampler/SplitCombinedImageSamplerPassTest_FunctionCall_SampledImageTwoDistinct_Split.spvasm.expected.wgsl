fn f(x_204 : texture_2d<f32>, x_205 : sampler, x_206 : texture_2d<f32>, x_207 : sampler) {
  return;
}

fn caller(caller_arg_image : texture_2d<f32>, caller_arg_sampler : sampler, x_210 : texture_2d<f32>, x_211 : sampler) -> f32 {
  f(caller_arg_image, caller_arg_sampler, x_210, x_211);
  return 0.0f;
}

fn main_1() {
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
