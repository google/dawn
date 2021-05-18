fn func(value : i32, pointer : ptr<function, i32>) -> i32 {
  return (value + *(pointer));
}

[[stage(compute)]]
fn main() {
  var i : i32 = 123;
  let r : i32 = func(i, &(i));
}
