type Arr = [[stride(16)]] array<i32, 2>;

[[block]]
struct buf0 {
  x_GLF_uniform_int_values : Arr;
};

var<private> x_GLF_color : vec4<f32>;

[[group(0), binding(0)]] var<uniform> x_5 : buf0;

fn main_1() {
  if (((1.0 % 1.0) <= 0.01)) {
    let x_29 : i32 = x_5.x_GLF_uniform_int_values[0];
    let x_32 : i32 = x_5.x_GLF_uniform_int_values[0];
    let x_35 : i32 = x_5.x_GLF_uniform_int_values[1];
    x_GLF_color = vec4<f32>(1.0, f32(x_29), f32(x_32), f32(x_35));
  } else {
    let x_39 : i32 = x_5.x_GLF_uniform_int_values[0];
    let x_40 : f32 = f32(x_39);
    x_GLF_color = vec4<f32>(x_40, x_40, x_40, x_40);
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
