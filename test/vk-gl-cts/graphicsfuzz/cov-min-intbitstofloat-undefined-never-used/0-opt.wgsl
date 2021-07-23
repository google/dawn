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

var<private> gl_FragCoord : vec4<f32>;

[[group(0), binding(0)]] var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4<f32>;

[[group(0), binding(1)]] var<uniform> x_9 : buf1;

fn main_1() {
  var f : f32;
  f = min(bitcast<f32>(-1), 1.0);
  let x_38 : f32 = gl_FragCoord.x;
  let x_40 : f32 = x_7.x_GLF_uniform_float_values[0];
  if ((x_38 > x_40)) {
    let x_46 : i32 = x_9.x_GLF_uniform_int_values[1];
    let x_49 : i32 = x_9.x_GLF_uniform_int_values[0];
    let x_52 : i32 = x_9.x_GLF_uniform_int_values[0];
    let x_55 : i32 = x_9.x_GLF_uniform_int_values[1];
    x_GLF_color = vec4<f32>(f32(x_46), f32(x_49), f32(x_52), f32(x_55));
  } else {
    let x_58 : f32 = f;
    x_GLF_color = vec4<f32>(x_58, x_58, x_58, x_58);
  }
  return;
}

struct main_out {
  [[location(0)]]
  x_GLF_color_1 : vec4<f32>;
};

[[stage(fragment)]]
fn main([[builtin(position)]] gl_FragCoord_param : vec4<f32>) -> main_out {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  return main_out(x_GLF_color);
}
