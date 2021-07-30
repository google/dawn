struct S {
  field0 : vec2<f32>;
  field1 : u32;
  field2 : i32;
};

fn main_1() {
  var x_35 : S;
  let x_1 : S = x_35;
  var x_2_1 : S = x_1;
  x_2_1.field2 = 30;
  let x_2 : S = x_2_1;
  return;
}

[[stage(compute), workgroup_size(1, 1, 1)]]
fn main() {
  main_1();
}
