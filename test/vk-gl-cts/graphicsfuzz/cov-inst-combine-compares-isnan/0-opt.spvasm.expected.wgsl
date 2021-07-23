type Arr = [[stride(16)]] array<i32, 2>;

[[block]]
struct buf0 {
  x_GLF_uniform_int_values : Arr;
};

var<private> x_GLF_color : vec4<f32>;

[[group(0), binding(0)]] var<uniform> x_5 : buf0;

fn main_1() {
  let x_7 : i32 = x_5.x_GLF_uniform_int_values[0];
  let x_8 : i32 = x_5.x_GLF_uniform_int_values[1];
  let x_9 : i32 = x_5.x_GLF_uniform_int_values[1];
  let x_10 : i32 = x_5.x_GLF_uniform_int_values[0];
  x_GLF_color = vec4<f32>(f32(x_7), f32(x_8), f32(x_9), f32(x_10));
  let x_36 : vec4<f32> = x_GLF_color;
  if (isNan((-(x_36)).x)) {
    let x_11 : i32 = x_5.x_GLF_uniform_int_values[0];
    let x_43 : f32 = f32(x_11);
    x_GLF_color = vec4<f32>(x_43, x_43, x_43, x_43);
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
