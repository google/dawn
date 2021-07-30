struct Communicators {
  alice : f32;
  bob : vec4<f32>;
};

var<private> x_1 : Communicators;

var<private> x_3 : Communicators;

var<private> x_2 : vec4<f32>;

fn main_1() {
  return;
}

struct main_out {
  [[builtin(position)]]
  x_2_1 : vec4<f32>;
  [[location(9)]]
  x_3_1 : f32;
  [[location(11)]]
  x_3_2 : vec4<f32>;
};

[[stage(vertex)]]
fn main([[location(9)]] x_1_param : f32, [[location(11)]] x_1_param_1 : vec4<f32>) -> main_out {
  x_1.alice = x_1_param;
  x_1.bob = x_1_param_1;
  main_1();
  return main_out(x_2, x_3.alice, x_3.bob);
}
