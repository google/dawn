struct S {
  field0 : u32;
  field1 : f32;
  field2 : array<u32, 2>;
};

fn main_1() {
  var x_101_phi : bool;
  let x_11 : bool = (true & true);
  let x_12 : bool = !(x_11);
  x_101_phi = x_11;
  if (true) {
    x_101_phi = x_12;
  }
  let x_101 : bool = x_101_phi;
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
