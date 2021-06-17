fn func(value : i32, pointer : ptr<function, i32>) {
  *(pointer) = value;
  return;
}

fn main_1() {
  var i : i32 = 0;
  i = 123;
  func(123, &(i));
  return;
}

[[stage(compute)]]
fn main() {
  main_1();
}
