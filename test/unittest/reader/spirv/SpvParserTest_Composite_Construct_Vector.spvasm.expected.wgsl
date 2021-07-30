struct S {
  field0 : vec2<f32>;
  field1 : u32;
  field2 : i32;
};

fn main_1() {
  let x_1 : vec2<u32> = vec2<u32>(10u, 20u);
  let x_2 : vec2<i32> = vec2<i32>(30, 40);
  let x_3 : vec2<f32> = vec2<f32>(50.0, 60.0);
  return;
}

[[stage(compute), workgroup_size(1, 1, 1)]]
fn main() {
  main_1();
}
