struct Communicators {
  alice : f32;
  bob : vec4<f32>;
};

var<private> x_1 : Communicators;

var<private> x_2 : vec4<f32>;

fn main_1() {
  return;
}

struct main_out {
  [[location(9)]]
  x_1_1 : f32;
  [[location(10)]]
  x_1_2 : vec4<f32>;
  [[builtin(position)]]
  x_2_1 : vec4<f32>;
};

[[stage(vertex)]]
fn main() -> main_out {
  main_1();
  return main_out(x_1.alice, x_1.bob, x_2);
}
