type Arr = [[stride(16)]] array<i32, 3>;

[[block]]
struct buf1 {
  x_GLF_uniform_int_values : Arr;
};

type Arr_1 = [[stride(16)]] array<f32, 1>;

[[block]]
struct buf0 {
  x_GLF_uniform_float_values : Arr_1;
};

[[group(0), binding(1)]] var<uniform> x_6 : buf1;

[[group(0), binding(0)]] var<uniform> x_10 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : i32;
  var i : i32;
  var b : f32;
  let x_34 : i32 = x_6.x_GLF_uniform_int_values[1];
  a = x_34;
  let x_35 : i32 = a;
  a = (x_35 + 1);
  let x_38 : i32 = x_6.x_GLF_uniform_int_values[1];
  i = x_38;
  loop {
    let x_43 : i32 = i;
    let x_45 : i32 = x_6.x_GLF_uniform_int_values[0];
    if ((x_43 < x_45)) {
    } else {
      break;
    }
    let x_48 : i32 = i;
    let x_50 : i32 = a;
    b = ldexp(f32(x_48), -(x_50));

    continuing {
      let x_53 : i32 = i;
      i = (x_53 + 1);
    }
  }
  let x_55 : f32 = b;
  let x_57 : f32 = x_10.x_GLF_uniform_float_values[0];
  if ((x_55 == x_57)) {
    let x_63 : i32 = x_6.x_GLF_uniform_int_values[2];
    let x_66 : i32 = x_6.x_GLF_uniform_int_values[1];
    let x_69 : i32 = x_6.x_GLF_uniform_int_values[1];
    let x_72 : i32 = x_6.x_GLF_uniform_int_values[2];
    x_GLF_color = vec4<f32>(f32(x_63), f32(x_66), f32(x_69), f32(x_72));
  } else {
    let x_75 : f32 = b;
    x_GLF_color = vec4<f32>(x_75, x_75, x_75, x_75);
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
