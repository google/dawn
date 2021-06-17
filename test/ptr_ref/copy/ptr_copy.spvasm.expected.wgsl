fn main_1() {
  var x_10 : u32;
  let x_1 : ptr<function, u32> = &(x_10);
  let x_2 : ptr<function, u32> = x_1;
  return;
}

[[stage(compute), workgroup_size(1, 1, 1)]]
fn main() {
  main_1();
}
