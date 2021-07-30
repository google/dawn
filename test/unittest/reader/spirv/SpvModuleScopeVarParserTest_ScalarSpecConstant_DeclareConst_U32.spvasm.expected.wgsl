[[override(12)]] let myconst : u32 = 42u;

fn main_1() {
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
