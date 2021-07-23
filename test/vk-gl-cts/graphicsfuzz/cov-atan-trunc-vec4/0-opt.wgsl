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

[[group(0), binding(1)]] var<uniform> x_6 : buf1;

[[group(0), binding(0)]] var<uniform> x_9 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var v : vec4<f32>;
  var f : f32;
  var x_56 : bool;
  var x_57_phi : bool;
  let x_32 : i32 = x_6.x_GLF_uniform_int_values[0];
  let x_35 : i32 = x_6.x_GLF_uniform_int_values[0];
  let x_38 : i32 = x_6.x_GLF_uniform_int_values[0];
  v = vec4<f32>(f32(x_32), f32(x_35), -621.596008301, f32(x_38));
  let x_41 : vec4<f32> = v;
  f = atan(trunc(x_41)).z;
  let x_45 : f32 = f;
  let x_47 : f32 = x_9.x_GLF_uniform_float_values[0];
  let x_49 : bool = (x_45 > -(x_47));
  x_57_phi = x_49;
  if (x_49) {
    let x_52 : f32 = f;
    let x_54 : f32 = x_9.x_GLF_uniform_float_values[1];
    x_56 = (x_52 < -(x_54));
    x_57_phi = x_56;
  }
  let x_57 : bool = x_57_phi;
  if (x_57) {
    let x_62 : i32 = x_6.x_GLF_uniform_int_values[1];
    let x_65 : i32 = x_6.x_GLF_uniform_int_values[0];
    let x_68 : i32 = x_6.x_GLF_uniform_int_values[0];
    let x_71 : i32 = x_6.x_GLF_uniform_int_values[1];
    x_GLF_color = vec4<f32>(f32(x_62), f32(x_65), f32(x_68), f32(x_71));
  } else {
    let x_75 : i32 = x_6.x_GLF_uniform_int_values[0];
    let x_76 : f32 = f32(x_75);
    x_GLF_color = vec4<f32>(x_76, x_76, x_76, x_76);
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
