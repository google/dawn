fn f(x_100 : u32, x_101 : f32) {
  return;
}

fn caller() -> f32 {
  let caller_arg = 0u;
  f(caller_arg, 0.0f);
  return 0.0f;
}

fn main_1() {
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
