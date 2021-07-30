struct S {
  field0 : u32;
  field1 : f32;
  field2 : array<u32, 2>;
};

fn main_1() {
  var a : u32;
  var b : i32;
  var c : f32;
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
