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

[[stage(fragment)]]
fn main([[location(1)]] x_1_param : f32, [[location(2), interpolate(perspective, centroid)]] x_1_param_1 : f32, [[location(3), interpolate(perspective, sample)]] x_1_param_2 : f32, [[location(4), interpolate(linear)]] x_1_param_3 : f32, [[location(5), interpolate(linear, centroid)]] x_1_param_4 : f32, [[location(6), interpolate(linear, sample)]] x_1_param_5 : f32) {
  x_1.field0 = x_1_param;
  x_1.field1 = x_1_param_1;
  x_1.field2 = x_1_param_2;
  x_1.field3 = x_1_param_3;
  x_1.field4 = x_1_param_4;
  x_1.field5 = x_1_param_5;
  main_1();
}
