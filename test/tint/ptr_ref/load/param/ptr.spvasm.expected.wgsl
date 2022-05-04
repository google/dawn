fn func(value : i32, pointer : ptr<function, i32>) -> i32 {
  let x_9 : i32 = *(pointer);
  return (value + x_9);
}

fn main_1() {
  var i : i32 = 0i;
  i = 123i;
  let x_19 : i32 = i;
  let x_18 : i32 = func(x_19, &(i));
  return;
}

@stage(compute) @workgroup_size(1, 1, 1)
fn main() {
  main_1();
}
