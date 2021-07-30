struct S {
  field0 : u32;
  field1 : f32;
  field2 : array<u32, 2>;
};

fn main_1() {
  var a : bool = false;
  var b : i32 = 0;
  var c : u32 = 0u;
  var d : f32 = 0.0;
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
