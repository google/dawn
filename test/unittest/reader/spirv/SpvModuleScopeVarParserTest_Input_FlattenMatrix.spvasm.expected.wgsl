var<private> x_1 : mat2x4<f32>;

var<private> x_2 : vec4<f32>;

fn main_1() {
  return;
}

struct main_out {
  [[builtin(position)]]
  x_2_1 : vec4<f32>;
};

[[stage(vertex)]]
fn main([[location(9)]] x_1_param : vec4<f32>, [[location(10)]] x_1_param_1 : vec4<f32>) -> main_out {
  x_1[0] = x_1_param;
  x_1[1] = x_1_param_1;
  main_1();
  return main_out(x_2);
}
