var<private> x_1 : array<u32, 1>;

fn main_1() {
  return;
}

[[stage(fragment)]]
fn main([[builtin(sample_mask)]] x_1_param : u32) {
  x_1[0] = x_1_param;
  main_1();
}
