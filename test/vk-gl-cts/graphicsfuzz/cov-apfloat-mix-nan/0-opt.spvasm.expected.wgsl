type Arr = [[stride(16)]] array<i32, 10>;

[[block]]
struct buf1 {
  x_GLF_uniform_int_values : Arr;
};

type Arr_1 = [[stride(16)]] array<f32, 1>;

[[block]]
struct buf0 {
  x_GLF_uniform_float_values : Arr_1;
};

[[group(0), binding(1)]] var<uniform> x_7 : buf1;

[[group(0), binding(0)]] var<uniform> x_9 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var nan : f32;
  var undefined : vec4<f32>;
  var x_83 : bool;
  var x_84_phi : bool;
  nan = bitcast<f32>(-1);
  let x_43 : i32 = x_7.x_GLF_uniform_int_values[0];
  let x_46 : i32 = x_7.x_GLF_uniform_int_values[1];
  let x_49 : i32 = x_7.x_GLF_uniform_int_values[2];
  let x_52 : i32 = x_7.x_GLF_uniform_int_values[3];
  let x_56 : i32 = x_7.x_GLF_uniform_int_values[4];
  let x_59 : i32 = x_7.x_GLF_uniform_int_values[5];
  let x_62 : i32 = x_7.x_GLF_uniform_int_values[6];
  let x_65 : i32 = x_7.x_GLF_uniform_int_values[7];
  let x_68 : f32 = nan;
  undefined = mix(vec4<f32>(f32(x_43), f32(x_46), f32(x_49), f32(x_52)), vec4<f32>(f32(x_56), f32(x_59), f32(x_62), f32(x_65)), vec4<f32>(x_68, x_68, x_68, x_68));
  let x_72 : i32 = x_7.x_GLF_uniform_int_values[0];
  let x_74 : i32 = x_7.x_GLF_uniform_int_values[9];
  let x_75 : bool = (x_72 == x_74);
  x_84_phi = x_75;
  if (!(x_75)) {
    let x_80 : f32 = undefined.x;
    let x_82 : f32 = x_9.x_GLF_uniform_float_values[0];
    x_83 = (x_80 > x_82);
    x_84_phi = x_83;
  }
  let x_84 : bool = x_84_phi;
  if (x_84) {
    let x_89 : i32 = x_7.x_GLF_uniform_int_values[0];
    let x_92 : i32 = x_7.x_GLF_uniform_int_values[8];
    let x_95 : i32 = x_7.x_GLF_uniform_int_values[8];
    let x_98 : i32 = x_7.x_GLF_uniform_int_values[0];
    x_GLF_color = vec4<f32>(f32(x_89), f32(x_92), f32(x_95), f32(x_98));
  } else {
    let x_102 : i32 = x_7.x_GLF_uniform_int_values[8];
    let x_103 : f32 = f32(x_102);
    x_GLF_color = vec4<f32>(x_103, x_103, x_103, x_103);
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
