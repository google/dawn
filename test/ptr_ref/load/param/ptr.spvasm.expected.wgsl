fn func(value : i32, pointer : ptr<function, i32>) -> i32 {
  let x_9 : i32 = *(pointer);
  return (value + x_9);
}

[[stage(compute)]]
fn main() {
  var i : i32 = 0;
  i = 123;
  let x_19 : i32 = i;
  let x_18 : i32 = func(x_19, &(i));
  return;
}
