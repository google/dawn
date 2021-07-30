struct S {
  field0 : f32;
  [[size(4)]]
  padding : u32;
  field1 : vec2<f32>;
  field2 : mat2x2<f32>;
};

fn x_100_1() {
  return;
}

[[stage(fragment)]]
fn x_100() {
  x_100_1();
}
