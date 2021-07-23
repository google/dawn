[[block]]
struct buf0 {
  zero : f32;
};

var<private> gl_FragCoord : vec4<f32>;

[[group(0), binding(0)]] var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var f : f32;
  var x_49 : bool;
  var x_50_phi : bool;
  let x_31 : f32 = gl_FragCoord.x;
  f = x_31;
  let x_32 : f32 = f;
  f = (x_32 + -0x1.8p+128);
  let x_34 : f32 = f;
  if (((5.0 / x_34) == 0.0)) {
    let x_39 : f32 = f;
    f = (x_39 + 1.0);
  }
  let x_41 : f32 = f;
  let x_42 : bool = isNan(x_41);
  x_50_phi = x_42;
  if (!(x_42)) {
    let x_46 : f32 = f;
    let x_48 : f32 = x_7.zero;
    x_49 = (x_46 != x_48);
    x_50_phi = x_49;
  }
  let x_50 : bool = x_50_phi;
  if (x_50) {
    f = 0.0;
  }
  let x_53 : f32 = f;
  if ((x_53 == 0.0)) {
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
fn main([[builtin(position)]] gl_FragCoord_param : vec4<f32>) -> main_out {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  return main_out(x_GLF_color);
}
