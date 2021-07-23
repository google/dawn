type Arr = [[stride(16)]] array<f32, 1>;

[[block]]
struct buf0 {
  x_GLF_uniform_float_values : Arr;
};

type Arr_1 = [[stride(16)]] array<i32, 4>;

[[block]]
struct buf1 {
  x_GLF_uniform_int_values : Arr_1;
};

[[group(0), binding(0)]] var<uniform> x_7 : buf0;

[[group(0), binding(1)]] var<uniform> x_10 : buf1;

var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var data : array<vec4<f32>, 2>;
  var b : i32;
  var y : i32;
  var i : i32;
  let x_42 : f32 = x_7.x_GLF_uniform_float_values[0];
  let x_45 : f32 = x_7.x_GLF_uniform_float_values[0];
  data = array<vec4<f32>, 2>(vec4<f32>(x_42, x_42, x_42, x_42), vec4<f32>(x_45, x_45, x_45, x_45));
  let x_49 : i32 = x_10.x_GLF_uniform_int_values[1];
  b = x_49;
  let x_51 : f32 = gl_FragCoord.y;
  let x_54 : i32 = x_10.x_GLF_uniform_int_values[1];
  let x_56 : f32 = gl_FragCoord.y;
  let x_60 : i32 = x_10.x_GLF_uniform_int_values[1];
  y = clamp(i32(x_51), (x_54 | i32(x_56)), x_60);
  let x_63 : i32 = x_10.x_GLF_uniform_int_values[1];
  i = x_63;
  loop {
    var x_82 : bool;
    var x_83_phi : bool;
    let x_68 : i32 = i;
    let x_70 : i32 = x_10.x_GLF_uniform_int_values[0];
    if ((x_68 < x_70)) {
    } else {
      break;
    }
    let x_73 : i32 = b;
    let x_75 : i32 = x_10.x_GLF_uniform_int_values[0];
    let x_76 : bool = (x_73 > x_75);
    x_83_phi = x_76;
    if (x_76) {
      let x_79 : i32 = y;
      let x_81 : i32 = x_10.x_GLF_uniform_int_values[1];
      x_82 = (x_79 > x_81);
      x_83_phi = x_82;
    }
    let x_83 : bool = x_83_phi;
    if (x_83) {
      break;
    }
    let x_86 : i32 = b;
    b = (x_86 + 1);

    continuing {
      let x_88 : i32 = i;
      i = (x_88 + 1);
    }
  }
  let x_90 : i32 = b;
  let x_92 : i32 = x_10.x_GLF_uniform_int_values[0];
  if ((x_90 == x_92)) {
    let x_97 : i32 = x_10.x_GLF_uniform_int_values[2];
    let x_99 : i32 = x_10.x_GLF_uniform_int_values[1];
    let x_101 : i32 = x_10.x_GLF_uniform_int_values[3];
    let x_104 : i32 = x_10.x_GLF_uniform_int_values[1];
    let x_107 : i32 = x_10.x_GLF_uniform_int_values[2];
    let x_110 : i32 = x_10.x_GLF_uniform_int_values[2];
    let x_113 : i32 = x_10.x_GLF_uniform_int_values[1];
    data[clamp(x_97, x_99, x_101)] = vec4<f32>(f32(x_104), f32(x_107), f32(x_110), f32(x_113));
  }
  let x_118 : i32 = x_10.x_GLF_uniform_int_values[1];
  let x_120 : vec4<f32> = data[x_118];
  x_GLF_color = vec4<f32>(x_120.x, x_120.y, x_120.z, x_120.w);
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
