fn main_1() {
  var x_1 : bool = true;
  let x_2 : bool = x_1;
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
