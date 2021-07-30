struct S {
  field0 : bool;
  field1 : u32;
  field2 : i32;
  field3 : f32;
};

fn main_1() {
  let x_11 : S = S(false, 0u, 0, 0.0);
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
