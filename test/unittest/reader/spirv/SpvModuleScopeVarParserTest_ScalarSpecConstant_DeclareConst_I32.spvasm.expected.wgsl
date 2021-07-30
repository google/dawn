[[override(12)]] let myconst : i32 = 42;

fn main_1() {
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
