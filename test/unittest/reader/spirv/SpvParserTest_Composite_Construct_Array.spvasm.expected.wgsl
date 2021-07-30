struct S {
  field0 : vec2<f32>;
  field1 : u32;
  field2 : i32;
};

fn main_1() {
  let x_1 : array<u32, 5> = array<u32, 5>(10u, 20u, 3u, 4u, 5u);
  return;
}

[[stage(compute), workgroup_size(1, 1, 1)]]
fn main() {
  main_1();
}
