type Arr = [[stride(16)]] array<f32, 4>;

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

[[group(0), binding(1)]] var<uniform> x_9 : buf1;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var arr : array<f32, 3>;
  var i : i32;
  let x_36 : f32 = x_6.x_GLF_uniform_float_values[0];
  let x_38 : f32 = x_6.x_GLF_uniform_float_values[1];
  let x_40 : f32 = x_6.x_GLF_uniform_float_values[2];
  arr = array<f32, 3>(x_36, x_38, x_40);
  i = 1;
  loop {
    let x_46 : i32 = i;
    let x_48 : i32 = x_9.x_GLF_uniform_int_values[2];
    if ((x_46 < min(x_48, 3))) {
    } else {
      break;
    }
    let x_53 : i32 = x_9.x_GLF_uniform_int_values[2];
    let x_55 : f32 = x_6.x_GLF_uniform_float_values[0];
    let x_57 : f32 = arr[x_53];
    arr[x_53] = (x_57 + x_55);

    continuing {
      let x_60 : i32 = i;
      i = (x_60 + 1);
    }
  }
  let x_63 : i32 = x_9.x_GLF_uniform_int_values[2];
  let x_65 : f32 = arr[x_63];
  let x_67 : f32 = x_6.x_GLF_uniform_float_values[3];
  if ((x_65 == x_67)) {
    let x_73 : i32 = x_9.x_GLF_uniform_int_values[1];
    let x_76 : i32 = x_9.x_GLF_uniform_int_values[0];
    let x_79 : i32 = x_9.x_GLF_uniform_int_values[0];
    let x_82 : i32 = x_9.x_GLF_uniform_int_values[1];
    x_GLF_color = vec4<f32>(f32(x_73), f32(x_76), f32(x_79), f32(x_82));
  } else {
    let x_86 : i32 = x_9.x_GLF_uniform_int_values[0];
    let x_87 : f32 = f32(x_86);
    x_GLF_color = vec4<f32>(x_87, x_87, x_87, x_87);
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
