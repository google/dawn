struct S {
  field0 : vec2<f32>;
  field1 : u32;
  field2 : i32;
};

fn main_1() {
  let x_1 : vec2<u32> = vec2<u32>(4u, 3u);
  let x_10 : vec2<u32> = vec2<u32>(0u, x_1.y);
  return;
}

[[stage(compute), workgroup_size(1, 1, 1)]]
fn main() {
  main_1();
}
