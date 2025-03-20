fn f(x_100 : texture_2d<f32>, x_101 : sampler, x_102 : texture_2d<f32>, x_103 : sampler) -> f32 {
  return 0.0f;
}

fn main_1() {
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
