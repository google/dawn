var<private> x_1 : array<i32, 1>;

fn main_1() {
  x_1[0] = 12;
  return;
}

struct main_out {
  [[builtin(sample_mask)]]
  x_1_1 : u32;
};

[[stage(fragment)]]
fn main() -> main_out {
  main_1();
  return main_out(bitcast<u32>(x_1[0]));
}
