var<private> x_1 : array<u32, 1> = array<u32, 1>(0u);

fn main_1() {
  return;
}

struct main_out {
  [[builtin(sample_mask)]]
  x_1_1 : u32;
};

[[stage(fragment)]]
fn main() -> main_out {
  main_1();
  return main_out(x_1[0]);
}
