var<private> I : i32 = 0;

fn main_1() {
  I = 123;
  I = ((100 + 20) + 3);
  return;
}

[[stage(compute), workgroup_size(1, 1, 1)]]
fn main() {
  main_1();
}
