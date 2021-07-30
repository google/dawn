struct S {
  field0 : vec2<f32>;
  field1 : u32;
  field2 : i32;
};

fn main_1() {
  var x_1_1 : vec2<f32> = vec2<f32>(50.0, 60.0);
  x_1_1.y = 70.0;
  let x_1 : vec2<f32> = x_1_1;
  return;
}

[[stage(compute), workgroup_size(1, 1, 1)]]
fn main() {
  main_1();
}
