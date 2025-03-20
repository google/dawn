fn f(x_113 : texture_2d<f32>, x_114 : sampler) {
  return;
}

fn caller(caller_arg_image : texture_2d<f32>, caller_arg_sampler : sampler) -> f32 {
  f(caller_arg_image, caller_arg_sampler);
  return 0.0f;
}

fn main_1() {
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
