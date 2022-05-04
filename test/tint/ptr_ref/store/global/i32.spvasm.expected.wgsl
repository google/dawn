var<private> I : i32 = 0i;

fn main_1() {
  I = 123i;
  I = ((100i + 20i) + 3i);
  return;
}

@stage(compute) @workgroup_size(1, 1, 1)
fn main() {
  main_1();
}
