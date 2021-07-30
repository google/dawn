var<private> x_2 : u32;

fn main_1() {
  var x_1 : u32;
  x_2 = x_1;
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
