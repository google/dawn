[[block]]
struct buf0 {
  five : i32;
};

var<private> gl_FragCoord : vec4<f32>;

[[group(0), binding(0)]] var<uniform> x_10 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn func_f1_(x : ptr<function, f32>) -> f32 {
  let x_56 : f32 = *(x);
  if ((x_56 > 5.0)) {
    let x_61 : f32 = gl_FragCoord.x;
    if ((x_61 < 0.5)) {
      discard;
    } else {
      let x_67 : f32 = gl_FragCoord.y;
      if ((x_67 < 0.5)) {
        discard;
      }
    }
  }
  let x_71 : f32 = *(x);
  return (x_71 + 1.0);
}

fn main_1() {
  var f : f32;
  var i : i32;
  var param : f32;
  f = 0.0;
  i = 0;
  loop {
    let x_39 : i32 = i;
    let x_41 : i32 = x_10.five;
    if ((x_39 < x_41)) {
    } else {
      break;
    }

    continuing {
      let x_45 : i32 = i;
      param = f32(x_45);
      let x_47 : f32 = func_f1_(&(param));
      f = x_47;
      let x_48 : i32 = i;
      i = (x_48 + 1);
    }
  }
  let x_50 : f32 = f;
  if ((x_50 == 5.0)) {
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
