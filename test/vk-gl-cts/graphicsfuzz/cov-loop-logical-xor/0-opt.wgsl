type Arr = [[stride(16)]] array<i32, 3>;

[[block]]
struct buf0 {
  x_GLF_uniform_int_values : Arr;
};

[[group(0), binding(0)]] var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : i32;
  let x_26 : i32 = x_6.x_GLF_uniform_int_values[0];
  a = x_26;
  let x_28 : i32 = x_6.x_GLF_uniform_int_values[1];
  let x_29 : f32 = f32(x_28);
  x_GLF_color = vec4<f32>(x_29, x_29, x_29, x_29);
  loop {
    let x_36 : i32 = x_6.x_GLF_uniform_int_values[2];
    let x_37 : i32 = a;
    if (((x_36 == x_37) != true)) {
    } else {
      break;
    }
    let x_42 : i32 = x_6.x_GLF_uniform_int_values[0];
    let x_45 : i32 = x_6.x_GLF_uniform_int_values[1];
    let x_48 : i32 = x_6.x_GLF_uniform_int_values[1];
    let x_51 : i32 = x_6.x_GLF_uniform_int_values[0];
    x_GLF_color = vec4<f32>(f32(x_42), f32(x_45), f32(x_48), f32(x_51));
    break;
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
