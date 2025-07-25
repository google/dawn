fn f(x_99 : f32, x_204 : texture_2d<f32>, x_205 : sampler, x_101 : u32) {
  return;
}

fn caller(x_200 : u32, caller_arg_image : texture_2d<f32>, caller_arg_sampler : sampler, x_201 : f32) -> f32 {
  f(x_201, caller_arg_image, caller_arg_sampler, x_200);
  return 0.0f;
}

fn main_1() {
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
