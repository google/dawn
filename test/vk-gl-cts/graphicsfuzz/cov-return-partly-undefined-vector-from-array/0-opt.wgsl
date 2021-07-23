type Arr = [[stride(16)]] array<f32, 2>;

[[block]]
struct buf0 {
  x_GLF_uniform_float_values : Arr;
};

type Arr_1 = [[stride(16)]] array<i32, 2>;

[[block]]
struct buf1 {
  x_GLF_uniform_int_values : Arr_1;
};

[[group(0), binding(0)]] var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4<f32>;

[[group(0), binding(1)]] var<uniform> x_11 : buf1;

fn func_() -> vec2<f32> {
  var v : vec2<f32>;
  var a : i32;
  var indexable : array<vec2<f32>, 3>;
  let x_67 : f32 = x_7.x_GLF_uniform_float_values[0];
  v.y = x_67;
  a = 2;
  let x_70 : f32 = x_7.x_GLF_uniform_float_values[1];
  let x_73 : f32 = x_7.x_GLF_uniform_float_values[1];
  let x_75 : vec2<f32> = v;
  let x_77 : i32 = a;
  indexable = array<vec2<f32>, 3>(vec2<f32>(x_70, x_70), vec2<f32>(x_73, x_73), x_75);
  let x_79 : vec2<f32> = indexable[x_77];
  return x_79;
}

fn main_1() {
  let x_40 : vec2<f32> = func_();
  let x_43 : f32 = x_7.x_GLF_uniform_float_values[0];
  if ((x_40.y == x_43)) {
    let x_49 : i32 = x_11.x_GLF_uniform_int_values[0];
    let x_52 : i32 = x_11.x_GLF_uniform_int_values[1];
    let x_55 : i32 = x_11.x_GLF_uniform_int_values[1];
    let x_58 : i32 = x_11.x_GLF_uniform_int_values[0];
    x_GLF_color = vec4<f32>(f32(x_49), f32(x_52), f32(x_55), f32(x_58));
  } else {
    let x_62 : i32 = x_11.x_GLF_uniform_int_values[1];
    let x_63 : f32 = f32(x_62);
    x_GLF_color = vec4<f32>(x_63, x_63, x_63, x_63);
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
