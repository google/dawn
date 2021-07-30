struct S {
  field0 : vec2<f32>;
  field1 : u32;
  field2 : i32;
};

fn main_1() {
  let x_1 : mat3x2<f32> = mat3x2<f32>(vec2<f32>(50.0, 60.0), vec2<f32>(60.0, 50.0), vec2<f32>(70.0, 70.0));
  return;
}

[[stage(compute), workgroup_size(1, 1, 1)]]
fn main() {
  main_1();
}
