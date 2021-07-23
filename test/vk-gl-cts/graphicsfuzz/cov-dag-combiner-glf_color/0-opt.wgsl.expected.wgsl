type Arr = [[stride(16)]] array<f32, 4>;

[[block]]
struct buf1 {
  x_GLF_uniform_float_values : Arr;
};

type Arr_1 = [[stride(16)]] array<i32, 2>;

[[block]]
struct buf0 {
  x_GLF_uniform_int_values : Arr_1;
};

var<private> x_GLF_color : vec4<f32>;

[[group(0), binding(1)]] var<uniform> x_7 : buf1;

[[group(0), binding(0)]] var<uniform> x_12 : buf0;

fn func_f1_(b : ptr<function, f32>) -> f32 {
  let x_90 : f32 = x_7.x_GLF_uniform_float_values[0];
  let x_92 : f32 = x_7.x_GLF_uniform_float_values[0];
  let x_94 : f32 = x_7.x_GLF_uniform_float_values[1];
  x_GLF_color = vec4<f32>(x_90, x_92, x_94, 1.0);
  let x_96 : vec4<f32> = x_GLF_color;
  x_GLF_color = x_96;
  let x_98 : f32 = x_7.x_GLF_uniform_float_values[0];
  let x_99 : f32 = *(b);
  if ((x_98 >= x_99)) {
    let x_104 : f32 = x_7.x_GLF_uniform_float_values[0];
    return x_104;
  }
  let x_106 : f32 = x_7.x_GLF_uniform_float_values[2];
  return x_106;
}

fn main_1() {
  var a : f32;
  var param : f32;
  var param_1 : f32;
  var x_71 : bool;
  var x_72_phi : bool;
  let x_44 : f32 = x_7.x_GLF_uniform_float_values[0];
  param = x_44;
  let x_45 : f32 = func_f1_(&(param));
  a = x_45;
  let x_47 : f32 = x_7.x_GLF_uniform_float_values[0];
  let x_49 : f32 = x_7.x_GLF_uniform_float_values[0];
  param_1 = (x_47 + x_49);
  let x_51 : f32 = func_f1_(&(param_1));
  let x_52 : f32 = a;
  a = (x_52 + x_51);
  let x_54 : f32 = a;
  let x_56 : f32 = x_7.x_GLF_uniform_float_values[3];
  let x_57 : bool = (x_54 == x_56);
  x_72_phi = x_57;
  if (x_57) {
    let x_60 : vec4<f32> = x_GLF_color;
    let x_62 : f32 = x_7.x_GLF_uniform_float_values[0];
    let x_64 : f32 = x_7.x_GLF_uniform_float_values[0];
    let x_66 : f32 = x_7.x_GLF_uniform_float_values[1];
    let x_68 : f32 = x_7.x_GLF_uniform_float_values[0];
    x_71 = all((x_60 == vec4<f32>(x_62, x_64, x_66, x_68)));
    x_72_phi = x_71;
  }
  let x_72 : bool = x_72_phi;
  if (x_72) {
    let x_15 : i32 = x_12.x_GLF_uniform_int_values[0];
    let x_16 : i32 = x_12.x_GLF_uniform_int_values[1];
    let x_17 : i32 = x_12.x_GLF_uniform_int_values[1];
    let x_18 : i32 = x_12.x_GLF_uniform_int_values[0];
    x_GLF_color = vec4<f32>(f32(x_15), f32(x_16), f32(x_17), f32(x_18));
  } else {
    let x_19 : i32 = x_12.x_GLF_uniform_int_values[1];
    let x_86 : f32 = f32(x_19);
    x_GLF_color = vec4<f32>(x_86, x_86, x_86, x_86);
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
