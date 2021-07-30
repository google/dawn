struct S {
  field0 : u32;
  field1 : f32;
  field2 : array<u32, 2>;
};

fn main_1() {
  var a : bool = true;
  var b : bool = false;
  var c : i32 = -1;
  var d : u32 = 1u;
  var e : f32 = 1.5;
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
