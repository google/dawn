[[override(12)]] let myconst : f32 = 2.5;

fn main_1() {
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
