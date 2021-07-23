type Arr = [[stride(16)]] array<i32, 5>;

[[block]]
struct buf0 {
  x_GLF_uniform_int_values : Arr;
};

type Arr_1 = [[stride(16)]] array<f32, 3>;

[[block]]
struct buf1 {
  x_GLF_uniform_float_values : Arr_1;
};

[[group(0), binding(0)]] var<uniform> x_6 : buf0;

[[group(0), binding(1)]] var<uniform> x_9 : buf1;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var I : vec4<f32>;
  var N : vec4<f32>;
  var R : vec4<f32>;
  var ref : vec4<f32>;
  let x_40 : i32 = x_6.x_GLF_uniform_int_values[2];
  let x_43 : i32 = x_6.x_GLF_uniform_int_values[3];
  let x_46 : i32 = x_6.x_GLF_uniform_int_values[4];
  I = bitcast<vec4<f32>>(vec4<u32>(bitcast<u32>(x_40), bitcast<u32>(x_43), bitcast<u32>(x_46), 92985u));
  let x_51 : f32 = x_9.x_GLF_uniform_float_values[1];
  N = vec4<f32>(x_51, x_51, x_51, x_51);
  let x_53 : vec4<f32> = I;
  R = reflect(x_53, vec4<f32>(0.5, 0.5, 0.5, 0.5));
  let x_55 : vec4<f32> = I;
  let x_57 : f32 = x_9.x_GLF_uniform_float_values[2];
  let x_58 : vec4<f32> = N;
  let x_59 : vec4<f32> = I;
  let x_62 : vec4<f32> = N;
  ref = (x_55 - (x_62 * (x_57 * dot(x_58, x_59))));
  let x_65 : vec4<f32> = R;
  let x_66 : vec4<f32> = ref;
  let x_69 : f32 = x_9.x_GLF_uniform_float_values[0];
  if ((distance(x_65, x_66) < x_69)) {
    let x_75 : i32 = x_6.x_GLF_uniform_int_values[0];
    let x_78 : i32 = x_6.x_GLF_uniform_int_values[1];
    let x_81 : i32 = x_6.x_GLF_uniform_int_values[1];
    let x_84 : i32 = x_6.x_GLF_uniform_int_values[0];
    x_GLF_color = vec4<f32>(f32(x_75), f32(x_78), f32(x_81), f32(x_84));
  } else {
    let x_88 : i32 = x_6.x_GLF_uniform_int_values[1];
    let x_89 : f32 = f32(x_88);
    x_GLF_color = vec4<f32>(x_89, x_89, x_89, x_89);
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
