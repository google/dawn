[[block]]
struct buf0 {
  injectionSwitch : vec2<f32>;
};

[[group(0), binding(0)]] var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var f : f32;
  var x_38 : bool;
  var x_39_phi : bool;
  f = sinh(724.322021484);
  let x_29 : f32 = f;
  let x_30 : bool = isInf(x_29);
  x_39_phi = x_30;
  if (!(x_30)) {
    let x_35 : f32 = x_6.injectionSwitch.x;
    let x_37 : f32 = x_6.injectionSwitch.y;
    x_38 = (x_35 < x_37);
    x_39_phi = x_38;
  }
  let x_39 : bool = x_39_phi;
  if (x_39) {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  } else {
    x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
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
