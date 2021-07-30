var<private> x_1 : f32;

var<private> x_2 : f32;

var<private> x_3 : f32;

var<private> x_4 : f32;

var<private> x_5 : f32;

var<private> x_6 : f32;

fn main_1() {
  return;
}

[[stage(fragment)]]
fn main([[location(1)]] x_1_param : f32, [[location(2), interpolate(perspective, centroid)]] x_2_param : f32, [[location(3), interpolate(perspective, sample)]] x_3_param : f32, [[location(4), interpolate(linear)]] x_4_param : f32, [[location(5), interpolate(linear, centroid)]] x_5_param : f32, [[location(6), interpolate(linear, sample)]] x_6_param : f32) {
  x_1 = x_1_param;
  x_2 = x_2_param;
  x_3 = x_3_param;
  x_4 = x_4_param;
  x_5 = x_5_param;
  x_6 = x_6_param;
  main_1();
}
