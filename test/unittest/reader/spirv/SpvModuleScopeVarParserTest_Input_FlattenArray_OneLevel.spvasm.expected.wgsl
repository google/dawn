var<private> x_1 : array<f32, 3>;

var<private> x_2 : vec4<f32>;

fn main_1() {
  return;
}

struct main_out {
  [[builtin(position)]]
  x_2_1 : vec4<f32>;
};

[[stage(vertex)]]
fn main([[location(4)]] x_1_param : f32, [[location(5)]] x_1_param_1 : f32, [[location(6)]] x_1_param_2 : f32) -> main_out {
  x_1[0] = x_1_param;
  x_1[1] = x_1_param_1;
  x_1[2] = x_1_param_2;
  main_1();
  return main_out(x_2);
}
