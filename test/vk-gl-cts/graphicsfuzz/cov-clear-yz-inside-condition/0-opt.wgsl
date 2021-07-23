type Arr = [[stride(16)]] array<f32, 2>;

[[block]]
struct buf0 {
  x_GLF_uniform_float_values : Arr;
};

[[block]]
struct buf1 {
  zero : f32;
};

[[group(0), binding(0)]] var<uniform> x_6 : buf0;

[[group(0), binding(1)]] var<uniform> x_9 : buf1;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var v0 : vec2<f32>;
  var v1 : vec4<f32>;
  var x_57 : vec4<f32>;
  let x_32 : f32 = x_6.x_GLF_uniform_float_values[0];
  v0 = vec2<f32>(x_32, x_32);
  let x_35 : f32 = v0.x;
  let x_36 : vec4<f32> = vec4<f32>(x_35, x_35, x_35, x_35);
  v1 = x_36;
  let x_38 : f32 = x_9.zero;
  let x_40 : f32 = x_6.x_GLF_uniform_float_values[0];
  if (!((x_38 == x_40))) {
    let x_46 : f32 = x_9.zero;
    let x_48 : f32 = x_6.x_GLF_uniform_float_values[1];
    if ((x_46 == x_48)) {
      return;
    }
    let x_53 : f32 = x_6.x_GLF_uniform_float_values[0];
    let x_56 : vec2<f32> = (vec2<f32>(x_36.y, x_36.z) - vec2<f32>(x_53, x_53));
    x_57 = vec4<f32>(x_36.x, x_56.x, x_56.y, x_36.w);
    v1 = x_57;
  } else {
    discard;
  }
  x_GLF_color = x_57;
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
