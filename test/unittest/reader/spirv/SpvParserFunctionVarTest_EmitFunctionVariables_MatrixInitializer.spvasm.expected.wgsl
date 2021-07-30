struct S {
  field0 : u32;
  field1 : f32;
  field2 : array<u32, 2>;
};

fn main_1() {
  var x_200 : mat3x2<f32> = mat3x2<f32>(vec2<f32>(1.5, 2.0), vec2<f32>(2.0, 3.0), vec2<f32>(3.0, 4.0));
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
