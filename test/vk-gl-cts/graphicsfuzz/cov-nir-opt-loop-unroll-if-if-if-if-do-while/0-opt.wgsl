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

var<private> x_GLF_color : vec4<f32>;

[[group(0), binding(0)]] var<uniform> x_5 : buf0;

[[group(0), binding(1)]] var<uniform> x_8 : buf1;

fn main_1() {
  var a : f32;
  let x_31 : f32 = x_5.x_GLF_uniform_float_values[1];
  x_GLF_color = vec4<f32>(x_31, x_31, x_31, x_31);
  let x_34 : f32 = x_5.x_GLF_uniform_float_values[0];
  a = x_34;
  loop {
    let x_40 : f32 = x_5.x_GLF_uniform_float_values[0];
    let x_43 : f32 = x_5.x_GLF_uniform_float_values[0];
    if (((x_40 / 0.200000003) < x_43)) {
      return;
    }
    let x_48 : f32 = x_5.x_GLF_uniform_float_values[0];
    let x_51 : f32 = x_5.x_GLF_uniform_float_values[0];
    if (((x_48 / 0.200000003) < x_51)) {
      return;
    }
    let x_56 : f32 = x_5.x_GLF_uniform_float_values[0];
    let x_59 : f32 = x_5.x_GLF_uniform_float_values[0];
    if (((x_56 / 0.200000003) < x_59)) {
      return;
    }
    let x_64 : f32 = x_5.x_GLF_uniform_float_values[0];
    let x_67 : f32 = x_5.x_GLF_uniform_float_values[0];
    if (((x_64 / 0.200000003) < x_67)) {
      return;
    } else {
      a = 0.0;
    }

    continuing {
      let x_72 : f32 = a;
      if (!((x_72 == 0.0))) {
      } else {
        break;
      }
    }
  }
  let x_75 : i32 = x_8.x_GLF_uniform_int_values[1];
  let x_78 : i32 = x_8.x_GLF_uniform_int_values[0];
  let x_81 : i32 = x_8.x_GLF_uniform_int_values[0];
  let x_84 : i32 = x_8.x_GLF_uniform_int_values[1];
  x_GLF_color = vec4<f32>(f32(x_75), f32(x_78), f32(x_81), f32(x_84));
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
