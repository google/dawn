type Arr = [[stride(16)]] array<f32, 3>;

[[block]]
struct buf1 {
  x_GLF_uniform_float_values : Arr;
};

type Arr_1 = [[stride(16)]] array<i32, 4>;

[[block]]
struct buf0 {
  x_GLF_uniform_int_values : Arr_1;
};

[[group(0), binding(1)]] var<uniform> x_6 : buf1;

[[group(0), binding(0)]] var<uniform> x_8 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var m43 : mat4x3<f32>;
  var sums : Arr;
  var i : i32;
  var a : i32;
  var x_67_phi : i32;
  let x_44 : f32 = x_6.x_GLF_uniform_float_values[1];
  let x_48 : vec3<f32> = vec3<f32>(0.0, 0.0, 0.0);
  m43 = mat4x3<f32>(vec3<f32>(x_44, 0.0, 0.0), vec3<f32>(0.0, x_44, 0.0), vec3<f32>(0.0, 0.0, x_44), vec3<f32>(0.0, 0.0, 0.0));
  let x_51 : i32 = x_8.x_GLF_uniform_int_values[0];
  let x_53 : i32 = x_8.x_GLF_uniform_int_values[0];
  let x_55 : f32 = x_6.x_GLF_uniform_float_values[0];
  m43[x_51][x_53] = x_55;
  let x_58 : f32 = x_6.x_GLF_uniform_float_values[0];
  let x_60 : f32 = x_6.x_GLF_uniform_float_values[0];
  let x_62 : f32 = x_6.x_GLF_uniform_float_values[0];
  sums = Arr(x_58, x_60, x_62);
  let x_65 : i32 = x_8.x_GLF_uniform_int_values[0];
  i = x_65;
  x_67_phi = x_65;
  loop {
    let x_67 : i32 = x_67_phi;
    let x_73 : i32 = x_8.x_GLF_uniform_int_values[3];
    if ((x_67 < x_73)) {
    } else {
      break;
    }
    let x_77 : i32 = x_8.x_GLF_uniform_int_values[0];
    let x_79 : i32 = x_8.x_GLF_uniform_int_values[0];
    let x_81 : f32 = m43[x_67][x_79];
    let x_83 : f32 = sums[x_77];
    sums[x_77] = (x_83 + x_81);

    continuing {
      let x_68 : i32 = (x_67 + 1);
      i = x_68;
      x_67_phi = x_68;
    }
  }
  let x_87 : i32 = x_8.x_GLF_uniform_int_values[1];
  if ((x_87 == 1)) {
    a = 4;
    let x_92 : i32 = x_8.x_GLF_uniform_int_values[2];
    let x_94 : i32 = x_8.x_GLF_uniform_int_values[0];
    let x_96 : f32 = m43[4][x_94];
    let x_98 : f32 = sums[x_92];
    sums[x_92] = (x_98 + x_96);
  }
  let x_102 : i32 = x_8.x_GLF_uniform_int_values[1];
  let x_104 : f32 = sums[x_102];
  let x_106 : i32 = x_8.x_GLF_uniform_int_values[0];
  let x_108 : f32 = sums[x_106];
  let x_111 : f32 = x_6.x_GLF_uniform_float_values[2];
  if (((x_104 + x_108) == x_111)) {
    let x_117 : i32 = x_8.x_GLF_uniform_int_values[0];
    let x_120 : i32 = x_8.x_GLF_uniform_int_values[1];
    let x_123 : i32 = x_8.x_GLF_uniform_int_values[1];
    let x_126 : i32 = x_8.x_GLF_uniform_int_values[0];
    x_GLF_color = vec4<f32>(f32(x_117), f32(x_120), f32(x_123), f32(x_126));
  } else {
    let x_130 : i32 = x_8.x_GLF_uniform_int_values[1];
    let x_131 : f32 = f32(x_130);
    x_GLF_color = vec4<f32>(x_131, x_131, x_131, x_131);
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
