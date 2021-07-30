struct S {
  field0 : u32;
  field1 : f32;
  field2 : array<u32, 2>;
};

fn main_1() {
  let x_1 : u32 = 1u;
  let x_2 : u32 = x_1;
  if (true) {
  }
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
