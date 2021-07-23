type Arr = [[stride(16)]] array<f32, 2>;

[[block]]
struct buf0 {
  x_GLF_uniform_float_values : Arr;
};

[[group(0), binding(0)]] var<uniform> x_5 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  let x_23 : f32 = x_5.x_GLF_uniform_float_values[1];
  if ((inverseSqrt(x_23) < -1.0)) {
    let x_30 : f32 = x_5.x_GLF_uniform_float_values[0];
    x_GLF_color = vec4<f32>(x_30, x_30, x_30, x_30);
  } else {
    let x_33 : f32 = x_5.x_GLF_uniform_float_values[1];
    let x_35 : f32 = x_5.x_GLF_uniform_float_values[0];
    let x_37 : f32 = x_5.x_GLF_uniform_float_values[0];
    let x_39 : f32 = x_5.x_GLF_uniform_float_values[1];
    x_GLF_color = vec4<f32>(x_33, x_35, x_37, x_39);
  }
  return;
}

struct main_out {
  [[location(0)]]
  x_GLF_color_1 : vec4<f32>;
};

[[stage(fragment)]]
fn main() -> main_out {
  main_1();
  return main_out(x_GLF_color);
}
