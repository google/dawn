var<private> x_1 : f32;

var<private> x_2 : f32;

var<private> x_3 : f32;

var<private> x_4 : f32;

var<private> x_5 : f32;

var<private> x_6 : f32;

fn main_1() {
  return;
}

struct main_out {
  [[location(1)]]
  x_1_1 : f32;
  [[location(2), interpolate(perspective, centroid)]]
  x_2_1 : f32;
  [[location(3), interpolate(perspective, sample)]]
  x_3_1 : f32;
  [[location(4), interpolate(linear)]]
  x_4_1 : f32;
  [[location(5), interpolate(linear, centroid)]]
  x_5_1 : f32;
  [[location(6), interpolate(linear, sample)]]
  x_6_1 : f32;
};

[[stage(fragment)]]
fn main() -> main_out {
  main_1();
  return main_out(x_1, x_2, x_3, x_4, x_5, x_6);
}
