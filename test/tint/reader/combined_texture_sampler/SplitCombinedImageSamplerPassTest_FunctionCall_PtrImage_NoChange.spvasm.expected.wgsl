fn f(x_100 : texture_2d<f32>) {
  return;
}

fn caller(caller_arg : texture_2d<f32>) -> f32 {
  f(caller_arg);
  return 0.0f;
}

fn main_1() {
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
