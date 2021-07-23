type Arr = [[stride(16)]] array<i32, 4>;

[[block]]
struct buf0 {
  x_GLF_uniform_int_values : Arr;
};

[[group(0), binding(0)]] var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var i : i32;
  var a : array<i32, 2>;
  let x_32 : i32 = x_6.x_GLF_uniform_int_values[2];
  i = x_32;
  loop {
    let x_37 : i32 = i;
    let x_39 : i32 = x_6.x_GLF_uniform_int_values[0];
    if ((x_37 < x_39)) {
    } else {
      break;
    }
    let x_43 : i32 = x_6.x_GLF_uniform_int_values[1];
    let x_44 : i32 = i;
    let x_46 : i32 = x_6.x_GLF_uniform_int_values[3];
    a = array<i32, 2>(x_43, ((vec2<i32>(x_44, x_44) % vec2<i32>(3, x_46))).y);

    continuing {
      let x_52 : i32 = i;
      i = (x_52 + 1);
    }
  }
  let x_55 : i32 = x_6.x_GLF_uniform_int_values[2];
  let x_57 : i32 = a[x_55];
  let x_60 : i32 = x_6.x_GLF_uniform_int_values[1];
  let x_63 : i32 = x_6.x_GLF_uniform_int_values[1];
  let x_66 : i32 = x_6.x_GLF_uniform_int_values[2];
  let x_68 : i32 = a[x_66];
  x_GLF_color = vec4<f32>(f32(x_57), f32(x_60), f32(x_63), f32(x_68));
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
