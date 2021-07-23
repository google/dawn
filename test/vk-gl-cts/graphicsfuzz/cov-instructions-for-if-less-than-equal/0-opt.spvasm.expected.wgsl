type Arr = [[stride(16)]] array<i32, 2>;

[[block]]
struct buf1 {
  x_GLF_uniform_int_values : Arr;
};

type Arr_1 = [[stride(16)]] array<f32, 2>;

[[block]]
struct buf0 {
  x_GLF_uniform_float_values : Arr_1;
};

var<private> x_GLF_color : vec4<f32>;

[[group(0), binding(1)]] var<uniform> x_5 : buf1;

[[group(0), binding(0)]] var<uniform> x_8 : buf0;

fn main_1() {
  var i : i32;
  let x_29 : i32 = x_5.x_GLF_uniform_int_values[0];
  let x_30 : f32 = f32(x_29);
  x_GLF_color = vec4<f32>(x_30, x_30, x_30, x_30);
  let x_33 : i32 = x_5.x_GLF_uniform_int_values[0];
  i = x_33;
  loop {
    let x_38 : i32 = i;
    let x_40 : i32 = x_5.x_GLF_uniform_int_values[1];
    if ((x_38 < x_40)) {
    } else {
      break;
    }
    let x_44 : f32 = x_8.x_GLF_uniform_float_values[1];
    let x_45 : i32 = i;
    if (!((x_44 <= f32(x_45)))) {
      let x_52 : f32 = x_8.x_GLF_uniform_float_values[0];
      let x_53 : i32 = i;
      let x_55 : i32 = i;
      let x_58 : f32 = x_8.x_GLF_uniform_float_values[0];
      let x_60 : vec4<f32> = x_GLF_color;
      x_GLF_color = (x_60 + vec4<f32>(x_52, f32(x_53), f32(x_55), x_58));
    }

    continuing {
      let x_62 : i32 = i;
      i = (x_62 + 1);
    }
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
