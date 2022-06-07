struct strided_arr {
  @size(16)
  el : f32,
}

type Arr = array<strided_arr, 2u>;

struct buf1 {
  x_GLF_uniform_float_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : i32,
}

type Arr_1 = array<strided_arr_1, 3u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr_1,
}

@group(0) @binding(1) var<uniform> x_6 : buf1;

@group(0) @binding(0) var<uniform> x_8 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var m32 : mat3x2<f32>;
  var sums : array<f32, 3u>;
  var x_52_phi : i32;
  let x_40 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  m32 = mat3x2<f32>(vec2<f32>(x_40, 0.0), vec2<f32>(0.0, x_40), vec2<f32>(0.0, 0.0));
  let x_45 : i32 = x_8.x_GLF_uniform_int_values[0].el;
  if ((x_45 == 1)) {
    m32[3][x_45] = x_40;
  }
  sums = array<f32, 3u>(x_40, x_40, x_40);
  x_52_phi = x_45;
  loop {
    var x_53 : i32;
    let x_52 : i32 = x_52_phi;
    let x_56 : i32 = x_8.x_GLF_uniform_int_values[2].el;
    if ((x_52 < x_56)) {
    } else {
      break;
    }

    continuing {
      let x_60 : f32 = m32[x_52][x_45];
      let x_61_save = x_56;
      let x_62 : f32 = sums[x_61_save];
      sums[x_61_save] = (x_62 + x_60);
      x_53 = (x_52 + 1);
      x_52_phi = x_53;
    }
  }
  let x_65 : f32 = sums[x_45];
  let x_67 : f32 = x_6.x_GLF_uniform_float_values[1].el;
  let x_69 : i32 = x_8.x_GLF_uniform_int_values[1].el;
  let x_71 : f32 = sums[x_69];
  x_GLF_color = vec4<f32>(x_65, x_67, x_67, x_71);
  return;
}

struct main_out {
  @location(0)
  x_GLF_color_1 : vec4<f32>,
}

@fragment
fn main() -> main_out {
  main_1();
  return main_out(x_GLF_color);
}
