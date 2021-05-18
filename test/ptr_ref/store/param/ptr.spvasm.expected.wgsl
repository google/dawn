fn func(value : i32, pointer : ptr<function, i32>) {
  *(pointer) = value;
  return;
}

[[stage(compute)]]
fn main() {
  var i : i32 = 0;
  i = 123;
  func(123, &(i));
  return;
}
