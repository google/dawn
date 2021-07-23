type Arr = [[stride(16)]] array<f32, 1>;

[[block]]
struct buf0 {
  x_GLF_uniform_float_values : Arr;
};

type Arr_1 = [[stride(16)]] array<i32, 2>;

[[block]]
struct buf1 {
  x_GLF_uniform_int_values : Arr_1;
};

[[group(0), binding(0)]] var<uniform> x_5 : buf0;

var<private> x_GLF_color : vec4<f32>;

[[group(0), binding(1)]] var<uniform> x_7 : buf1;

fn main_1() {
  let x_29 : f32 = x_5.x_GLF_uniform_float_values[0];
  let x_32 : f32 = x_5.x_GLF_uniform_float_values[0];
  if ((ldexp(x_29, 10000) == x_32)) {
    let x_38 : i32 = x_7.x_GLF_uniform_int_values[1];
    let x_41 : i32 = x_7.x_GLF_uniform_int_values[0];
    let x_44 : i32 = x_7.x_GLF_uniform_int_values[0];
    let x_47 : i32 = x_7.x_GLF_uniform_int_values[1];
    x_GLF_color = vec4<f32>(f32(x_38), f32(x_41), f32(x_44), f32(x_47));
  } else {
    let x_51 : i32 = x_7.x_GLF_uniform_int_values[1];
    let x_54 : i32 = x_7.x_GLF_uniform_int_values[0];
    let x_57 : i32 = x_7.x_GLF_uniform_int_values[0];
    let x_60 : i32 = x_7.x_GLF_uniform_int_values[1];
    x_GLF_color = vec4<f32>(f32(x_51), f32(x_54), f32(x_57), f32(x_60));
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
