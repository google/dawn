type Arr = [[stride(16)]] array<i32, 2>;

[[block]]
struct buf0 {
  x_GLF_uniform_int_values : Arr;
};

type Arr_1 = [[stride(16)]] array<f32, 2>;

[[block]]
struct buf1 {
  x_GLF_uniform_float_values : Arr_1;
};

var<private> x_GLF_color : vec4<f32>;

[[group(0), binding(0)]] var<uniform> x_8 : buf0;

var<private> gl_FragCoord : vec4<f32>;

[[group(0), binding(1)]] var<uniform> x_10 : buf1;

fn f1_f1_(a : ptr<function, f32>) -> f32 {
  let x_100 : f32 = *(a);
  return dpdx(x_100);
}

fn main_1() {
  var v2 : vec4<f32>;
  var a_1 : f32;
  var x_40 : f32;
  var param : f32;
  let x_42 : i32 = x_8.x_GLF_uniform_int_values[0];
  let x_45 : i32 = x_8.x_GLF_uniform_int_values[1];
  let x_48 : i32 = x_8.x_GLF_uniform_int_values[1];
  let x_51 : i32 = x_8.x_GLF_uniform_int_values[0];
  x_GLF_color = vec4<f32>(f32(x_42), f32(x_45), f32(x_48), f32(x_51));
  let x_55 : f32 = gl_FragCoord.x;
  let x_57 : f32 = x_10.x_GLF_uniform_float_values[1];
  if ((x_55 < x_57)) {
    let x_62 : f32 = v2.x;
    if (!((x_62 < 1.0))) {
      let x_68 : f32 = x_10.x_GLF_uniform_float_values[1];
      let x_70 : f32 = x_10.x_GLF_uniform_float_values[1];
      let x_72 : f32 = x_10.x_GLF_uniform_float_values[0];
      if ((x_70 > x_72)) {
        let x_78 : f32 = x_10.x_GLF_uniform_float_values[0];
        param = x_78;
        let x_79 : f32 = f1_f1_(&(param));
        x_40 = x_79;
      } else {
        let x_81 : f32 = x_10.x_GLF_uniform_float_values[0];
        x_40 = x_81;
      }
      let x_82 : f32 = x_40;
      a_1 = (x_68 / x_82);
      let x_85 : f32 = x_10.x_GLF_uniform_float_values[0];
      let x_88 : f32 = x_10.x_GLF_uniform_float_values[0];
      let x_90 : f32 = a_1;
      let x_92 : vec3<f32> = mix(vec3<f32>(x_85, x_85, x_85), vec3<f32>(x_88, x_88, x_88), vec3<f32>(x_90, x_90, x_90));
      let x_94 : f32 = x_10.x_GLF_uniform_float_values[1];
      x_GLF_color = vec4<f32>(x_92.x, x_92.y, x_92.z, x_94);
    }
  }
  return;
}

struct main_out {
  [[location(0)]]
  x_GLF_color_1 : vec4<f32>;
};

[[stage(fragment)]]
fn main([[builtin(position)]] gl_FragCoord_param : vec4<f32>) -> main_out {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  return main_out(x_GLF_color);
}
