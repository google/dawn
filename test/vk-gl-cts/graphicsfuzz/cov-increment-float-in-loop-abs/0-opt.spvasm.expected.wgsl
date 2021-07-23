type Arr = [[stride(16)]] array<f32, 3>;

[[block]]
struct buf0 {
  x_GLF_uniform_float_values : Arr;
};

type Arr_1 = [[stride(16)]] array<i32, 3>;

[[block]]
struct buf1 {
  x_GLF_uniform_int_values : Arr_1;
};

[[group(0), binding(0)]] var<uniform> x_6 : buf0;

[[group(0), binding(1)]] var<uniform> x_10 : buf1;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var f0 : f32;
  var f1 : f32;
  var i : i32;
  var x_63 : bool;
  var x_64_phi : bool;
  let x_34 : f32 = x_6.x_GLF_uniform_float_values[0];
  f0 = x_34;
  let x_36 : f32 = x_6.x_GLF_uniform_float_values[0];
  f1 = x_36;
  let x_38 : i32 = x_10.x_GLF_uniform_int_values[1];
  i = x_38;
  loop {
    let x_43 : i32 = i;
    let x_45 : i32 = x_10.x_GLF_uniform_int_values[0];
    if ((x_43 < x_45)) {
    } else {
      break;
    }
    let x_48 : f32 = f0;
    f0 = abs((1.100000024 * x_48));
    let x_51 : f32 = f0;
    f1 = x_51;

    continuing {
      let x_52 : i32 = i;
      i = (x_52 + 1);
    }
  }
  let x_54 : f32 = f1;
  let x_56 : f32 = x_6.x_GLF_uniform_float_values[1];
  let x_57 : bool = (x_54 > x_56);
  x_64_phi = x_57;
  if (x_57) {
    let x_60 : f32 = f1;
    let x_62 : f32 = x_6.x_GLF_uniform_float_values[2];
    x_63 = (x_60 < x_62);
    x_64_phi = x_63;
  }
  let x_64 : bool = x_64_phi;
  if (x_64) {
    let x_69 : i32 = x_10.x_GLF_uniform_int_values[2];
    let x_72 : i32 = x_10.x_GLF_uniform_int_values[1];
    let x_75 : i32 = x_10.x_GLF_uniform_int_values[1];
    let x_78 : i32 = x_10.x_GLF_uniform_int_values[2];
    x_GLF_color = vec4<f32>(f32(x_69), f32(x_72), f32(x_75), f32(x_78));
  } else {
    let x_82 : i32 = x_10.x_GLF_uniform_int_values[1];
    let x_83 : f32 = f32(x_82);
    x_GLF_color = vec4<f32>(x_83, x_83, x_83, x_83);
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
