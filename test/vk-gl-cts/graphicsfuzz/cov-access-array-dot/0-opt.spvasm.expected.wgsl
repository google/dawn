type Arr = [[stride(16)]] array<i32, 3>;

[[block]]
struct buf1 {
  x_GLF_uniform_int_values : Arr;
};

type Arr_1 = [[stride(16)]] array<f32, 3>;

[[block]]
struct buf0 {
  x_GLF_uniform_float_values : Arr_1;
};

[[group(0), binding(1)]] var<uniform> x_6 : buf1;

[[group(0), binding(0)]] var<uniform> x_9 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var numbers : array<i32, 3>;
  var a : vec2<f32>;
  var b : f32;
  let x_38 : i32 = x_6.x_GLF_uniform_int_values[0];
  let x_40 : i32 = x_6.x_GLF_uniform_int_values[0];
  numbers[x_38] = x_40;
  let x_43 : i32 = x_6.x_GLF_uniform_int_values[1];
  let x_45 : i32 = x_6.x_GLF_uniform_int_values[1];
  numbers[x_43] = x_45;
  let x_48 : i32 = x_6.x_GLF_uniform_int_values[2];
  let x_50 : i32 = x_6.x_GLF_uniform_int_values[2];
  numbers[x_48] = x_50;
  let x_53 : i32 = x_6.x_GLF_uniform_int_values[0];
  let x_56 : f32 = x_9.x_GLF_uniform_float_values[2];
  let x_60 : i32 = numbers[select(2, 1, (0.0 < x_56))];
  a = vec2<f32>(f32(x_53), f32(x_60));
  let x_63 : vec2<f32> = a;
  let x_65 : f32 = x_9.x_GLF_uniform_float_values[1];
  let x_67 : f32 = x_9.x_GLF_uniform_float_values[1];
  b = dot(x_63, vec2<f32>(x_65, x_67));
  let x_70 : f32 = b;
  let x_72 : f32 = x_9.x_GLF_uniform_float_values[0];
  if ((x_70 == x_72)) {
    let x_78 : i32 = x_6.x_GLF_uniform_int_values[1];
    let x_81 : i32 = x_6.x_GLF_uniform_int_values[0];
    let x_84 : i32 = x_6.x_GLF_uniform_int_values[0];
    let x_87 : i32 = x_6.x_GLF_uniform_int_values[1];
    x_GLF_color = vec4<f32>(f32(x_78), f32(x_81), f32(x_84), f32(x_87));
  } else {
    let x_91 : i32 = x_6.x_GLF_uniform_int_values[0];
    let x_92 : f32 = f32(x_91);
    x_GLF_color = vec4<f32>(x_92, x_92, x_92, x_92);
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
