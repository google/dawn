[[block]]
struct buf0 {
  zero : f32;
};

var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

[[group(0), binding(0)]] var<uniform> x_11 : buf0;

fn func_i1_(b : ptr<function, i32>) -> f32 {
  var ndx : i32;
  var i : i32;
  ndx = 0;
  loop {
    let x_100 : i32 = ndx;
    if ((x_100 < 2)) {
    } else {
      break;
    }
    let x_104 : f32 = gl_FragCoord.x;
    if ((x_104 < 0.0)) {
      i = 0;
      loop {
        let x_112 : i32 = i;
        if ((x_112 < 2)) {
        } else {
          break;
        }
        if ((i32(cosh(vec2<f32>(1.0, 800.0)).x) <= 1)) {
          discard;
        }

        continuing {
          let x_121 : i32 = i;
          i = (x_121 + 1);
        }
      }
    }

    continuing {
      let x_123 : i32 = ndx;
      ndx = (x_123 + 1);
    }
  }
  let x_125 : i32 = *(b);
  if ((x_125 > 1)) {
    return 3.0;
  }
  let x_130 : f32 = gl_FragCoord.x;
  if ((x_130 < 0.0)) {
    x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
  }
  return 5.0;
}

fn main_1() {
  var f : f32;
  var param : i32;
  var x : i32;
  var param_1 : i32;
  x_GLF_color = vec4<f32>(1.0, 1.0, 1.0, 1.0);
  f = 0.0;
  loop {
    let x_54 : f32 = x_GLF_color.y;
    if ((i32(x_54) < 0)) {
      discard;
    } else {
      let x_61 : f32 = x_11.zero;
      param = i32(x_61);
      let x_63 : f32 = func_i1_(&(param));
      f = x_63;
    }
    let x_65 : f32 = x_GLF_color.y;
    if ((i32(x_65) > 65)) {
      discard;
    }
    x = 0;
    loop {
      let x_74 : i32 = x;
      let x_76 : f32 = x_11.zero;
      if ((x_74 < (i32(x_76) + 1))) {
      } else {
        break;
      }
      let x_81 : i32 = x;
      param_1 = (x_81 + 10);
      let x_83 : f32 = func_i1_(&(param_1));
      f = x_83;

      continuing {
        let x_84 : i32 = x;
        x = (x_84 + 1);
      }
    }

    continuing {
      let x_87 : f32 = x_11.zero;
      if ((i32(x_87) > 1)) {
      } else {
        break;
      }
    }
  }
  let x_90 : f32 = f;
  if ((x_90 == 3.0)) {
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
