struct S {
  field0 : vec2<f32>;
  field1 : u32;
  field2 : i32;
};

struct S_1 {
  field0 : u32;
  field1 : array<mat3x2<f32>, 3>;
};

fn main_1() {
  var x_37 : S_1;
  let x_1 : S_1 = x_37;
  var x_2_1 : S_1 = x_1;
  x_2_1.field1[2u][0u].y = 70.0;
  let x_2 : S_1 = x_2_1;
  return;
}

[[stage(compute), workgroup_size(1, 1, 1)]]
fn main() {
  main_1();
}
