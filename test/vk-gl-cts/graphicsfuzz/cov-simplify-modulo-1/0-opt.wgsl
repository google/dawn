type Arr = [[stride(16)]] array<f32, 2>;

[[block]]
struct buf0 {
  x_GLF_uniform_float_values : Arr;
};

type Arr_1 = [[stride(16)]] array<i32, 1>;

[[block]]
struct buf1 {
  x_GLF_uniform_int_values : Arr_1;
};

[[group(0), binding(0)]] var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

[[group(0), binding(1)]] var<uniform> x_8 : buf1;

fn main_1() {
  var a : f32;
  let x_30 : f32 = x_6.x_GLF_uniform_float_values[0];
  a = (x_30 % 1.0);
  let x_32 : f32 = a;
  let x_34 : f32 = x_6.x_GLF_uniform_float_values[0];
  if ((x_32 == x_34)) {
    let x_40 : i32 = x_8.x_GLF_uniform_int_values[0];
    let x_42 : f32 = a;
    let x_43 : f32 = a;
    let x_45 : i32 = x_8.x_GLF_uniform_int_values[0];
    x_GLF_color = vec4<f32>(f32(x_40), x_42, x_43, f32(x_45));
  } else {
    let x_48 : f32 = a;
    x_GLF_color = vec4<f32>(x_48, x_48, x_48, x_48);
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
