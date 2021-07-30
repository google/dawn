var<private> x_1 : u32;

var<private> x_2 : vec2<u32>;

var<private> x_3 : i32;

var<private> x_4 : vec2<i32>;

var<private> x_5 : f32;

var<private> x_6 : vec2<f32>;

var<private> x_8 : vec4<f32>;

fn main_1() {
  return;
}

struct main_out {
  [[builtin(position)]]
  x_8_1 : vec4<f32>;
};

[[stage(vertex)]]
fn main([[location(1)]] x_1_param : u32, [[location(2)]] x_2_param : vec2<u32>, [[location(3)]] x_3_param : i32, [[location(4)]] x_4_param : vec2<i32>, [[location(5), interpolate(flat)]] x_5_param : f32, [[location(6), interpolate(flat)]] x_6_param : vec2<f32>) -> main_out {
  x_1 = x_1_param;
  x_2 = x_2_param;
  x_3 = x_3_param;
  x_4 = x_4_param;
  x_5 = x_5_param;
  x_6 = x_6_param;
  main_1();
  return main_out(x_8);
}
