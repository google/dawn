struct S {
  field0 : vec2<f32>;
  field1 : u32;
  field2 : i32;
};

fn main_1() {
  var x_35 : array<u32, 5>;
  let x_1 : array<u32, 5> = x_35;
  var x_2_1 : array<u32, 5> = x_1;
  x_2_1[3u] = 20u;
  let x_2 : array<u32, 5> = x_2_1;
  return;
}

[[stage(compute), workgroup_size(1, 1, 1)]]
fn main() {
  main_1();
}
