struct S {
  field0 : vec2<f32>;
  field1 : u32;
  field2 : i32;
};

fn main_1() {
  let x_1 : vec2<u32> = vec2<u32>(3u, 4u);
  let x_2 : u32 = 3u;
  let x_3 : i32 = 1;
  var x_10_1 : vec2<u32> = x_1;
  x_10_1[x_3] = x_2;
  let x_10 : vec2<u32> = x_10_1;
  return;
}

[[stage(compute), workgroup_size(1, 1, 1)]]
fn main() {
  main_1();
}
