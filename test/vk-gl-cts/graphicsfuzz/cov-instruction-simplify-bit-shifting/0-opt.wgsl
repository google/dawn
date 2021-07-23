type Arr = [[stride(16)]] array<i32, 2>;

[[block]]
struct buf0 {
  x_GLF_uniform_int_values : Arr;
};

[[group(0), binding(0)]] var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : i32;
  let x_25 : i32 = x_6.x_GLF_uniform_int_values[0];
  let x_29 : i32 = x_6.x_GLF_uniform_int_values[0];
  a = ((1 >> bitcast<u32>((x_25 << bitcast<u32>(5)))) >> bitcast<u32>(x_29));
  let x_31 : i32 = a;
  if ((x_31 == 1)) {
    let x_37 : i32 = x_6.x_GLF_uniform_int_values[1];
    let x_40 : i32 = x_6.x_GLF_uniform_int_values[0];
    let x_43 : i32 = x_6.x_GLF_uniform_int_values[0];
    let x_46 : i32 = x_6.x_GLF_uniform_int_values[1];
    x_GLF_color = vec4<f32>(f32(x_37), f32(x_40), f32(x_43), f32(x_46));
  } else {
    let x_49 : i32 = a;
    let x_50 : f32 = f32(x_49);
    x_GLF_color = vec4<f32>(x_50, x_50, x_50, x_50);
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
