var<private> I : i32 = 0;

fn main_1() {
  let x_9 : i32 = I;
  let x_11 : i32 = (x_9 + 1);
  return;
}

[[stage(compute), workgroup_size(1, 1, 1)]]
fn main() {
  main_1();
}
