fn main_1() {
  var x_1 : u32;
  let x_2 : ptr<function, u32> = &(x_1);
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
