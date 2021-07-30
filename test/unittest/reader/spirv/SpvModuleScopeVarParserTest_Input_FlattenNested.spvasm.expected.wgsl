var<private> x_1 : array<mat2x4<f32>, 2>;

var<private> x_2 : vec4<f32>;

fn main_1() {
  return;
}

struct main_out {
  [[builtin(position)]]
  x_2_1 : vec4<f32>;
};

[[stage(vertex)]]
fn main([[location(7)]] x_1_param : vec4<f32>, [[location(8)]] x_1_param_1 : vec4<f32>, [[location(9)]] x_1_param_2 : vec4<f32>, [[location(10)]] x_1_param_3 : vec4<f32>) -> main_out {
  x_1[0][0] = x_1_param;
  x_1[0][1] = x_1_param_1;
  x_1[1][0] = x_1_param_2;
  x_1[1][1] = x_1_param_3;
  main_1();
  return main_out(x_2);
}
