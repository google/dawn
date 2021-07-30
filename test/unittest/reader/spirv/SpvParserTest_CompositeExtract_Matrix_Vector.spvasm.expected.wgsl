struct S {
  field0 : vec2<f32>;
  field1 : u32;
  field2 : i32;
};

fn main_1() {
  var x_35 : mat3x2<f32>;
  let x_1 : mat3x2<f32> = x_35;
  let x_2 : f32 = x_1[2u].y;
  return;
}

[[stage(compute), workgroup_size(1, 1, 1)]]
fn main() {
  main_1();
}
