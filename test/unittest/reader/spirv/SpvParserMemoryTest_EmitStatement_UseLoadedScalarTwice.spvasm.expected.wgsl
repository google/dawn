fn main_1() {
  var x_1 : u32 = 42u;
  let x_2 : u32 = x_1;
  x_1 = x_2;
  x_1 = x_2;
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
