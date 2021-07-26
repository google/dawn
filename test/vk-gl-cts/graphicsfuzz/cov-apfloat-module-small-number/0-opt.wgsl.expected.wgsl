type Arr = [[stride(16)]] array<i32, 2>;

[[block]]
struct buf1 {
  x_GLF_uniform_int_values : Arr;
};

type Arr_1 = [[stride(16)]] array<f32, 1>;

[[block]]
struct buf0 {
  x_GLF_uniform_float_values : Arr_1;
};

var<private> x_GLF_color : vec4<f32>;

[[group(0), binding(1)]] var<uniform> x_8 : buf1;

[[group(0), binding(0)]] var<uniform> x_10 : buf0;

fn main_1() {
  var f0 : f32;
  var s1 : f32;
  var f1 : f32;
  var x_72 : bool;
  var x_73_phi : bool;
  f0 = (10.0 % 0.000001);
  s1 = 0x1.b38fb8p-127;
  let x_38 : f32 = s1;
  if ((x_38 == 0.0)) {
    s1 = 1.0;
  }
  var x_62 : bool;
  var x_71 : bool;
  var x_63_phi : bool;
  var x_72_phi : bool;
  let x_42 : f32 = s1;
  f1 = (10.0 % x_42);
  let x_44 : f32 = f1;
  let x_46 : f32 = s1;
  let x_48 : bool = (isInf(x_44) || (x_46 == 1.0));
  x_73_phi = x_48;
  if (!(x_48)) {
    let x_52 : f32 = f0;
    let x_53 : f32 = f1;
    let x_54 : bool = (x_52 == x_53);
    x_63_phi = x_54;
    if (!(x_54)) {
      let x_58 : f32 = f0;
      let x_60 : f32 = f0;
      x_62 = ((x_58 > 0.99000001) && (x_60 < 0.01));
      x_63_phi = x_62;
    }
    let x_63 : bool = x_63_phi;
    x_72_phi = x_63;
    if (!(x_63)) {
      let x_67 : f32 = f1;
      let x_69 : f32 = f1;
      x_71 = ((x_67 > 0.99000001) && (x_69 < 0.01));
      x_72_phi = x_71;
    }
    x_72 = x_72_phi;
    x_73_phi = x_72;
  }
  let x_73 : bool = x_73_phi;
  let x_74 : f32 = f1;
  if ((x_73 || (x_74 == 10.0))) {
    let x_81 : i32 = x_8.x_GLF_uniform_int_values[1];
    let x_84 : i32 = x_8.x_GLF_uniform_int_values[0];
    let x_87 : i32 = x_8.x_GLF_uniform_int_values[0];
    let x_90 : i32 = x_8.x_GLF_uniform_int_values[1];
    x_GLF_color = vec4<f32>(f32(x_81), f32(x_84), f32(x_87), f32(x_90));
  } else {
    let x_94 : i32 = x_8.x_GLF_uniform_int_values[0];
    let x_95 : f32 = f32(x_94);
    x_GLF_color = vec4<f32>(x_95, x_95, x_95, x_95);
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
