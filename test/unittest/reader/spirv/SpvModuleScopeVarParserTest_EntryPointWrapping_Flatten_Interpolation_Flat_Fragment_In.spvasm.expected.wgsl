struct S {
  field0 : f32;
  field1 : f32;
};

var<private> x_1 : array<f32, 2>;

var<private> x_2 : S;

fn main_1() {
  return;
}

[[stage(fragment)]]
fn main([[location(1), interpolate(flat)]] x_1_param : f32, [[location(2), interpolate(flat)]] x_1_param_1 : f32, [[location(5), interpolate(flat)]] x_2_param : f32, [[location(6), interpolate(flat)]] x_2_param_1 : f32) {
  x_1[0] = x_1_param;
  x_1[1] = x_1_param_1;
  x_2.field0 = x_2_param;
  x_2.field1 = x_2_param_1;
  main_1();
}
