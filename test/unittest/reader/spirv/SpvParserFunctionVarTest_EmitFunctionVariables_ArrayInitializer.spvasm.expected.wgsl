struct S {
  field0 : u32;
  field1 : f32;
  field2 : array<u32, 2>;
};

fn main_1() {
  var x_200 : array<u32, 2> = array<u32, 2>(1u, 2u);
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
