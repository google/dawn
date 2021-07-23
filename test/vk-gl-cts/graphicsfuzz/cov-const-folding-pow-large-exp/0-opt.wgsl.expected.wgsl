type Arr = [[stride(16)]] array<i32, 2>;

[[block]]
struct buf0 {
  x_GLF_uniform_int_values : Arr;
};

type Arr_1 = [[stride(16)]] array<f32, 1>;

[[block]]
struct buf1 {
  x_GLF_uniform_float_values : Arr_1;
};

[[group(0), binding(0)]] var<uniform> x_6 : buf0;

[[group(0), binding(1)]] var<uniform> x_8 : buf1;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var f : f32;
  var x_48 : bool;
  var x_49_phi : bool;
  f = pow(40.330001831, ldexp(1.0, 98980));
  let x_35 : i32 = x_6.x_GLF_uniform_int_values[0];
  let x_37 : i32 = x_6.x_GLF_uniform_int_values[0];
  let x_39 : i32 = x_6.x_GLF_uniform_int_values[1];
  let x_41 : bool = (x_35 == (x_37 + x_39));
  x_49_phi = x_41;
  if (!(x_41)) {
    let x_45 : f32 = f;
    let x_47 : f32 = x_8.x_GLF_uniform_float_values[0];
    x_48 = (x_45 > x_47);
    x_49_phi = x_48;
  }
  let x_49 : bool = x_49_phi;
  if (x_49) {
    let x_54 : i32 = x_6.x_GLF_uniform_int_values[0];
    let x_57 : i32 = x_6.x_GLF_uniform_int_values[1];
    let x_60 : i32 = x_6.x_GLF_uniform_int_values[1];
    let x_63 : i32 = x_6.x_GLF_uniform_int_values[0];
    x_GLF_color = vec4<f32>(f32(x_54), f32(x_57), f32(x_60), f32(x_63));
  } else {
    let x_67 : i32 = x_6.x_GLF_uniform_int_values[1];
    let x_68 : f32 = f32(x_67);
    x_GLF_color = vec4<f32>(x_68, x_68, x_68, x_68);
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
