fn main_1() {
  var i : i32 = 0;
  i = 123;
  i = 123;
  i = ((100 + 20) + 3);
  return;
}

[[stage(compute)]]
fn main() {
  main_1();
}
