var<private> I : i32 = 0;

fn main_1() {
  I = 123;
  I = ((100 + 20) + 3);
  return;
}

[[stage(compute)]]
fn main() {
  main_1();
}
