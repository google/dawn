fn func(value : i32, pointer : ptr<function, i32>) -> i32 {
  return (value + *(pointer));
}

@compute @workgroup_size(1u, 1u, 1u)
fn main() {
  var i : i32 = 0i;
  i = 123i;
  func(i, &(i));
}
