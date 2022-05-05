var<private> I : i32 = 0i;

fn main_1() {
  let x_9 : i32 = I;
  let x_11 : i32 = (x_9 + 1i);
  return;
}

@stage(compute) @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
