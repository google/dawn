var<private> x_1 : u32;

fn main_1() {
  x_1 = 42u;
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
