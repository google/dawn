struct S {
  field0 : f32;
  field1 : f32;
  field2 : f32;
  field3 : f32;
  field4 : f32;
  field5 : f32;
};

var<private> x_1 : S;

fn main_1() {
  return;
}

struct main_out {
  [[location(1)]]
  x_1_1 : f32;
  [[location(2), interpolate(perspective, centroid)]]
  x_1_2 : f32;
  [[location(3), interpolate(perspective, sample)]]
  x_1_3 : f32;
  [[location(4), interpolate(linear)]]
  x_1_4 : f32;
  [[location(5), interpolate(linear, centroid)]]
  x_1_5 : f32;
  [[location(6), interpolate(linear, sample)]]
  x_1_6 : f32;
};

[[stage(fragment)]]
fn main() -> main_out {
  main_1();
  return main_out(x_1.field0, x_1.field1, x_1.field2, x_1.field3, x_1.field4, x_1.field5);
}
