struct S {
  numbers : array<f32, 3>;
};

type Arr = [[stride(16)]] array<f32, 5>;

[[block]]
struct buf1 {
  x_GLF_uniform_float_values : Arr;
};

[[block]]
struct buf2 {
  zeroVec : vec2<f32>;
};

[[block]]
struct buf3 {
  oneVec : vec2<f32>;
};

type Arr_1 = [[stride(16)]] array<i32, 2>;

[[block]]
struct buf0 {
  x_GLF_uniform_int_values : Arr_1;
};

[[group(0), binding(1)]] var<uniform> x_7 : buf1;

[[group(0), binding(2)]] var<uniform> x_9 : buf2;

[[group(0), binding(3)]] var<uniform> x_12 : buf3;

[[group(0), binding(0)]] var<uniform> x_15 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var obj : S;
  var a : f32;
  var x_49 : vec2<f32>;
  var b : f32;
  let x_51 : f32 = x_7.x_GLF_uniform_float_values[3];
  let x_53 : f32 = x_7.x_GLF_uniform_float_values[2];
  let x_55 : f32 = x_7.x_GLF_uniform_float_values[4];
  obj = S(array<f32, 3>(x_51, x_53, x_55));
  let x_59 : f32 = x_9.zeroVec.x;
  let x_62 : f32 = x_7.x_GLF_uniform_float_values[0];
  obj.numbers[i32(x_59)] = x_62;
  let x_65 : f32 = x_9.zeroVec.x;
  let x_67 : f32 = x_7.x_GLF_uniform_float_values[0];
  if ((x_65 > x_67)) {
    let x_73 : vec2<f32> = x_9.zeroVec;
    x_49 = x_73;
  } else {
    let x_75 : vec2<f32> = x_12.oneVec;
    x_49 = x_75;
  }
  let x_77 : f32 = x_49.y;
  a = x_77;
  let x_79 : f32 = x_7.x_GLF_uniform_float_values[0];
  let x_80 : f32 = a;
  let x_82 : i32 = x_15.x_GLF_uniform_int_values[0];
  let x_84 : f32 = obj.numbers[x_82];
  b = mix(x_79, x_80, x_84);
  let x_86 : f32 = b;
  let x_88 : f32 = x_7.x_GLF_uniform_float_values[2];
  let x_91 : f32 = x_7.x_GLF_uniform_float_values[1];
  if ((distance(x_86, x_88) < x_91)) {
    let x_97 : i32 = x_15.x_GLF_uniform_int_values[0];
    let x_100 : i32 = x_15.x_GLF_uniform_int_values[1];
    let x_103 : i32 = x_15.x_GLF_uniform_int_values[1];
    let x_106 : i32 = x_15.x_GLF_uniform_int_values[0];
    x_GLF_color = vec4<f32>(f32(x_97), f32(x_100), f32(x_103), f32(x_106));
  } else {
    let x_110 : i32 = x_15.x_GLF_uniform_int_values[1];
    let x_111 : f32 = f32(x_110);
    x_GLF_color = vec4<f32>(x_111, x_111, x_111, x_111);
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
