fn func(value : i32, pointer : ptr<function, i32>) {
  *(pointer) = value;
}

[[stage(compute)]]
fn main() {
  var i : i32 = 123;
  func(123, &(i));
}
