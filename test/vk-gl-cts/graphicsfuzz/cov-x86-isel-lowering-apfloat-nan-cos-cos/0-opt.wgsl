type Arr = [[stride(16)]] array<i32, 2>;

[[block]]
struct buf0 {
  x_GLF_uniform_int_values : Arr;
};

type Arr_1 = [[stride(16)]] array<f32, 2>;

[[block]]
struct buf1 {
  x_GLF_uniform_float_values : Arr_1;
};

[[group(0), binding(0)]] var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

[[group(0), binding(1)]] var<uniform> x_8 : buf1;

fn main_1() {
  var v1 : vec2<f32>;
  var x_54 : bool;
  var x_55_phi : bool;
  let x_35 : i32 = x_6.x_GLF_uniform_int_values[0];
  v1 = cos(cos(bitcast<vec2<f32>>(vec2<i32>(-1, x_35))));
  let x_41 : f32 = v1.x;
  x_GLF_color = vec4<f32>(x_41, x_41, x_41, x_41);
  let x_44 : f32 = v1.y;
  let x_46 : f32 = x_8.x_GLF_uniform_float_values[0];
  let x_47 : bool = (x_44 > x_46);
  x_55_phi = x_47;
  if (x_47) {
    let x_51 : f32 = v1.y;
    let x_53 : f32 = x_8.x_GLF_uniform_float_values[1];
    x_54 = (x_51 < x_53);
    x_55_phi = x_54;
  }
  let x_55 : bool = x_55_phi;
  if (x_55) {
    let x_60 : i32 = x_6.x_GLF_uniform_int_values[0];
    let x_63 : i32 = x_6.x_GLF_uniform_int_values[1];
    let x_66 : i32 = x_6.x_GLF_uniform_int_values[1];
    let x_69 : i32 = x_6.x_GLF_uniform_int_values[0];
    x_GLF_color = vec4<f32>(f32(x_60), f32(x_63), f32(x_66), f32(x_69));
  } else {
    let x_73 : i32 = x_6.x_GLF_uniform_int_values[1];
    let x_74 : f32 = f32(x_73);
    x_GLF_color = vec4<f32>(x_74, x_74, x_74, x_74);
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
