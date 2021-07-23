type Arr = [[stride(16)]] array<f32, 2>;

[[block]]
struct buf0 {
  x_GLF_uniform_float_values : Arr;
};

[[group(0), binding(0)]] var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var m0 : mat2x2<f32>;
  var m1 : mat2x2<f32>;
  var v : vec2<f32>;
  let x_35 : f32 = x_6.x_GLF_uniform_float_values[0];
  let x_37 : f32 = x_6.x_GLF_uniform_float_values[0];
  m0 = mat2x2<f32>(vec2<f32>(x_35, -0.540302277), vec2<f32>(0.540302277, x_37));
  let x_41 : mat2x2<f32> = m0;
  let x_42 : mat2x2<f32> = m0;
  m1 = (x_41 * x_42);
  let x_45 : f32 = x_6.x_GLF_uniform_float_values[0];
  let x_47 : mat2x2<f32> = m1;
  v = (vec2<f32>(x_45, x_45) * x_47);
  let x_50 : f32 = v.x;
  let x_52 : f32 = x_6.x_GLF_uniform_float_values[0];
  if ((x_50 < x_52)) {
    let x_58 : f32 = x_6.x_GLF_uniform_float_values[0];
    let x_60 : f32 = x_6.x_GLF_uniform_float_values[1];
    let x_62 : f32 = x_6.x_GLF_uniform_float_values[1];
    let x_64 : f32 = x_6.x_GLF_uniform_float_values[0];
    x_GLF_color = vec4<f32>(x_58, x_60, x_62, x_64);
  } else {
    let x_67 : f32 = x_6.x_GLF_uniform_float_values[1];
    x_GLF_color = vec4<f32>(x_67, x_67, x_67, x_67);
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
