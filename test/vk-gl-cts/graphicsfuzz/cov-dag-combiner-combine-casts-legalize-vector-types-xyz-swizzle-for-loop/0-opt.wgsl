type Arr = [[stride(16)]] array<i32, 4>;

[[block]]
struct buf0 {
  x_GLF_uniform_int_values : Arr;
};

[[group(0), binding(0)]] var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var v : vec4<f32>;
  var i : i32;
  let x_36 : i32 = x_6.x_GLF_uniform_int_values[3];
  let x_37 : f32 = f32(x_36);
  v = vec4<f32>(x_37, x_37, x_37, x_37);
  let x_40 : i32 = x_6.x_GLF_uniform_int_values[0];
  i = x_40;
  loop {
    let x_45 : i32 = i;
    let x_47 : i32 = x_6.x_GLF_uniform_int_values[3];
    if ((x_45 < x_47)) {
    } else {
      break;
    }
    let x_50 : i32 = i;
    let x_51 : i32 = i;
    v[vec3<u32>(0u, 1u, 2u)[x_50]] = f32(x_51);

    continuing {
      let x_55 : i32 = i;
      i = (x_55 + 1);
    }
  }
  let x_57 : vec4<f32> = v;
  let x_59 : i32 = x_6.x_GLF_uniform_int_values[0];
  let x_62 : i32 = x_6.x_GLF_uniform_int_values[1];
  let x_65 : i32 = x_6.x_GLF_uniform_int_values[2];
  let x_68 : i32 = x_6.x_GLF_uniform_int_values[3];
  if (all((x_57 == vec4<f32>(f32(x_59), f32(x_62), f32(x_65), f32(x_68))))) {
    let x_77 : i32 = x_6.x_GLF_uniform_int_values[1];
    let x_80 : i32 = x_6.x_GLF_uniform_int_values[0];
    let x_83 : i32 = x_6.x_GLF_uniform_int_values[0];
    let x_86 : i32 = x_6.x_GLF_uniform_int_values[1];
    x_GLF_color = vec4<f32>(f32(x_77), f32(x_80), f32(x_83), f32(x_86));
  } else {
    let x_90 : i32 = x_6.x_GLF_uniform_int_values[0];
    let x_91 : f32 = f32(x_90);
    x_GLF_color = vec4<f32>(x_91, x_91, x_91, x_91);
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
