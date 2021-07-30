struct S {
  field0 : u32;
  field1 : f32;
  field2 : array<u32, 2>;
};

fn main_1() {
  var x_25 : u32;
  x_25 = 1u;
  x_25 = (1u + 1u);
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
