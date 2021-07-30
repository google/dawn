fn main_1() {
  var x_1 : u32 = 42u;
  let x_2 : u32 = x_1;
  let x_3 : u32 = x_1;
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
