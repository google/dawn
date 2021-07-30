var<private> x_1 : u32;

fn main_1() {
  let x_11 : ptr<private, u32> = &(x_1);
  let x_2 : u32 = *(x_11);
  return;
}

[[stage(fragment)]]
fn main([[builtin(sample_index)]] x_1_param : u32) {
  x_1 = x_1_param;
  main_1();
}
